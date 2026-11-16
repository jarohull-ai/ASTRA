#include "backend.h"
#include <QFile>
#include <QDir>
#include <QRegularExpression>
#include <QStorageInfo>
#include <QLocale>
#include <algorithm>
#include <cmath>
static QByteArray readFile(QString path){QFile f(path);return f.open(QIODevice::ReadOnly)?f.readAll():QByteArray();}
static QString number(double n,int precision=1){return QLocale().toString(n,'f',precision);}
Backend::Backend(QObject *parent):QObject(parent),settings("astramon","astramon"){
 ids=settings.value("tiles",QStringList{"cpu","memory","gpu","thermal","disk","network"}).toStringList();
 const QStringList valid{"cpu","memory","gpu","thermal","disk","network"};
 for(auto it=ids.begin();it!=ids.end();)if(!valid.contains(*it))it=ids.erase(it);else ++it;
 ids.removeDuplicates();opacity=settings.value("glass",.83).toDouble();scheme=settings.value("palette","omarchy").toString();
 elapsed.start();sample();connect(&timer,&QTimer::timeout,this,&Backend::sample);timer.start(1000);
}
double Backend::cpuUsage(quint64 total,quint64 idle,quint64 oldTotal,quint64 oldIdle){
 if(!oldTotal||total<=oldTotal||idle<oldIdle)return -1;
 return std::clamp(100.0*(1.0-double(idle-oldIdle)/double(total-oldTotal)),0.0,100.0);
}
void Backend::save(){settings.setValue("tiles",ids);settings.setValue("glass",opacity);settings.setValue("palette",scheme);settings.sync();emit layoutChanged();}
void Backend::setGlass(double v){opacity=std::clamp(v,.45,.98);save();}
void Backend::setPalette(QString v){scheme=v;save();sample();}
void Backend::toggle(QString id){if(ids.contains(id))ids.removeAll(id);else if(QStringList{"cpu","memory","gpu","thermal","disk","network"}.contains(id))ids.append(id);save();}
void Backend::move(QString id,int delta){int i=ids.indexOf(id),j=i+delta;if(i>=0&&j>=0&&j<ids.size()){ids.move(i,j);save();}}
void Backend::reset(){ids={"cpu","memory","gpu","thermal","disk","network"};opacity=.83;scheme="omarchy";save();sample();}
QVariantMap Backend::card(QString id)const{for(auto v:data)if(v.toMap().value("id")==id)return v.toMap();return {};}
void Backend::put(QString id,QString label,QString value,QString unit,QString detail,double level,bool available){
 auto &h=history[id];if(available){h.append(level);while(h.size()>90)h.removeFirst();}
 data.append(QVariantMap{{"id",id},{"label",label},{"value",available?value:QString::fromUtf8("—")},{"unit",available?unit:""},{"detail",detail},{"level",level},{"available",available},{"history",h}});
}
void Backend::sample(){
 auto theme=QString::fromUtf8(readFile(QDir::homePath()+"/.local/state/omarchy/current/theme/colors.toml"));
 auto match=QRegularExpression("accent\\s*=\\s*\"(#[0-9a-fA-F]{6})\"").match(theme);
 color=scheme=="ice"?"#70ddff":scheme=="amber"?"#ffbb66":match.hasMatch()?match.captured(1):"#31f7a5";
 if(stopped){elapsed.restart();emit changed();return;}
 double seconds=std::max(.001,elapsed.restart()/1000.0);data.clear();
 auto lines=readFile("/proc/stat").split('\n');auto fields=lines.value(0).simplified().split(' ');quint64 total=0,idle=0;
 for(int i=1;i<std::min(9,int(fields.size()));i++)total+=fields[i].toULongLong();
 idle=fields.value(4).toULongLong()+fields.value(5).toULongLong();
 double cpu=cpuUsage(total,idle,lastCpu,lastIdle);lastCpu=total;lastIdle=idle;
 int cores=0;for(auto l:lines)if(l.startsWith("cpu")&&l.size()>3&&l[3]>='0'&&l[3]<='9')cores++;
 put("cpu",tr("Processor"),number(cpu),"%",tr("%1 logical cores · 1 s sampling").arg(cores),cpu,cpu>=0);
 QMap<QByteArray,quint64> mem;for(auto line:readFile("/proc/meminfo").split('\n')){auto p=line.simplified().split(' ');if(p.size()>1)mem[p[0]]=p[1].toULongLong();}
 double mt=mem["MemTotal:"],used=std::max(0.0,mt-double(mem["MemAvailable:"]));
 put("memory",tr("Memory"),number(used/1048576),"GiB",tr("%1 GiB available / %2 GiB total").arg(number((mt-used)/1048576),number(mt/1048576)),mt?100*used/mt:0,mt>0);
 bool gpu=false;for(auto c:QDir("/sys/class/drm").entryList({"card*"},QDir::Dirs|QDir::NoDotAndDotDot)){
 if(c.contains('-'))continue;
 auto v=readFile("/sys/class/drm/"+c+"/device/gpu_busy_percent").trimmed();bool ok=false;double n=v.toDouble(&ok);if(ok){put("gpu",tr("Graphics"),number(n,0),"%",c+" · "+tr("GPU engine utilization"),n);gpu=true;break;}}
 if(!gpu)put("gpu",tr("Graphics"),"","",tr("GPU utilization unavailable from this driver"),0,false);
 double temp=-1;QString sensor;for(auto d:QDir("/sys/class/hwmon").entryList(QDir::Dirs|QDir::NoDotAndDotDot)){
 QString base="/sys/class/hwmon/"+d,name=QString::fromUtf8(readFile(base+"/name")).trimmed();
 if(name!="coretemp"&&name!="k10temp"&&name!="zenpower")continue;
 for(auto f:QDir(base).entryList({"temp*_input"},QDir::Files)){bool ok=false;double t=readFile(base+"/"+f).trimmed().toDouble(&ok)/1000;if(ok&&t>temp&&t<160){temp=t;sensor=name;}}}
 put("thermal",tr("Thermals"),number(temp),QString::fromUtf8("°C"),temp>=0?tr("CPU · highest sensor reading (%1)").arg(sensor):tr("CPU temperature sensor unavailable"),temp,temp>=0);
 QStorageInfo disk=QStorageInfo::root();double ds=disk.bytesTotal(),du=ds-disk.bytesFree();
 put("disk",tr("Storage"),number(du/1073741824,0),"GiB",tr("Root volume · %1 GiB free").arg(number(disk.bytesAvailable()/1073741824.0,0)),ds?100*du/ds:0,ds>0);
 quint64 rx=0,tx=0;QStringList interfaces;
 for(auto line:readFile("/proc/net/dev").split('\n')){int pos=line.indexOf(':');if(pos<0)continue;QString name=line.left(pos).trimmed();if(name=="lo")continue;
 if(readFile("/sys/class/net/"+name+"/operstate").trimmed()!="up")continue;
 auto p=line.mid(pos+1).simplified().split(' ');if(p.size()<9)continue;rx+=p[0].toULongLong();tx+=p[8].toULongLong();interfaces<<name;}
 bool ready=lastRx>0&&rx>=lastRx&&tx>=lastTx;double down=ready?(rx-lastRx)/seconds/1048576:0,up=ready?(tx-lastTx)/seconds/1048576:0;lastRx=rx;lastTx=tx;
 put("network",tr("Network"),number(down,2),"MiB/s",interfaces.isEmpty()?tr("No active network interface"):tr("↑ %1 MiB/s · %2").arg(number(up,2),interfaces.join(", ")),down,ready);
 emit changed();
}
