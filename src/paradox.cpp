#include "paradox.h"
#include <QFile>
#include <QDir>
#include <QSettings>
#include <QFileInfo>
#include <QDateTime>
#include <QJsonDocument>
#include <QJsonObject>
#include <QCryptographicHash>
#include <QRegularExpression>
#include <QStandardPaths>
#include <algorithm>
#include <cmath>
static QByteArray slurp(const QString &p){QFile f(p);return f.open(QIODevice::ReadOnly)?f.readAll():QByteArray();}
static double r3(double v){return std::round(v*1000.0)/1000.0;}
Paradox::Paradox(QObject *parent,QString path,bool autostart):QObject(parent){
 QSettings s("astramon","astramon");
 target=s.value("paradox/target","8.8.8.8").toString();tcpPort=s.value("paradox/tcpPort",443).toInt();
 proofPath=path.isEmpty()?QDir::homePath()+"/.local/state/astramon/paradox_proof.jsonl":path;
 loadChainTip();
 snapshot={{"ready",false},{"target",target},{"proof",proofPath},{"entries",counter},{"history",QVariantList{}}};
 connect(&ping,&QProcess::finished,this,[this]{
  pingOk=parsePing(QString::fromUtf8(ping.readAllStandardOutput()),pAvg,pMdev,pLoss);finishProbe();});
 connect(&ping,&QProcess::errorOccurred,this,[this](QProcess::ProcessError e){if(e==QProcess::FailedToStart){pingOk=false;pLoss=100;finishProbe();}});
 connect(&tcp,&QTcpSocket::connected,this,[this]{tcpOk=true;tcp.abort();finishProbe();});
 connect(&tcp,&QTcpSocket::errorOccurred,this,[this]{if(pending>0&&tcp.state()!=QAbstractSocket::ConnectedState){tcpOk=false;tcp.abort();finishProbe();}});
 connect(&timer,&QTimer::timeout,this,&Paradox::probe);
 bwClock.start();
 if(autostart){timer.start(5000);QTimer::singleShot(300,this,&Paradox::probe);}
}
bool Paradox::parsePing(const QString &out,double &avg,double &mdev,double &loss){
 auto lm=QRegularExpression("([\\d.]+)% packet loss").match(out);loss=lm.hasMatch()?lm.captured(1).toDouble():100;
 auto rm=QRegularExpression("= ([\\d.]+)/([\\d.]+)/([\\d.]+)/([\\d.]+) ms").match(out);
 if(rm.hasMatch()){avg=rm.captured(2).toDouble();mdev=rm.captured(4).toDouble();}else{avg=0;mdev=0;}
 return loss<100&&rm.hasMatch();
}
void Paradox::probe(){
 if(stopped||pending>0)return;
 pending=2;tcpOk=false;pingOk=false;
 ping.start("ping",{"-n","-q","-c","5","-i","0.2","-W","1",target});
 tcpClock.start();tcp.abort();tcp.connectToHost(target,quint16(tcpPort));
 QTimer::singleShot(1500,this,[this]{if(pending>0&&tcp.state()!=QAbstractSocket::UnconnectedState){tcpOk=false;tcp.abort();finishProbe();}});
}
void Paradox::finishProbe(){
 if(pending<=0||--pending>0)return;
 NetMetrics m;m.ping=pingOk;m.tcp=tcpOk;m.latency=pAvg;m.loss=pLoss;m.jitter=pMdev;
 // interfaces + default route (local, no network traffic)
 quint64 bytes=0;double linkMbps=0;bool anyUp=false;QStringList up;
 for(auto line:slurp("/proc/net/dev").split('\n')){int pos=line.indexOf(':');if(pos<0)continue;QString n=line.left(pos).trimmed();if(n=="lo")continue;
  if(slurp("/sys/class/net/"+n+"/operstate").trimmed()!="up")continue;
  anyUp=true;up<<n;
  auto p=line.mid(pos+1).simplified().split(' ');if(p.size()>8)bytes+=p[0].toULongLong()+p[8].toULongLong();
  bool ok=false;double sp=slurp("/sys/class/net/"+n+"/speed").trimmed().toDouble(&ok);if(ok&&sp>0)linkMbps+=sp;}
 m.interfaces=anyUp;
 bool route=false;for(auto l:slurp("/proc/net/route").split('\n')){auto f=l.simplified().split(' ');if(f.size()>2&&f[1]=="00000000"){route=true;break;}}
 m.routes=route;
 double secs=std::max(.001,bwClock.restart()/1000.0);
 if(lastBytes&&bytes>=lastBytes&&linkMbps>0)m.bandwidth=std::clamp((bytes-lastBytes)*8.0/secs/1e6/linkMbps*100.0,0.0,100.0);
 lastBytes=bytes;
 snapshot["linkKnown"]=linkMbps>0;snapshot["interfaces"]=up.join(", ");
 ingest(m);
}
Verdict Paradox::evaluate(const NetMetrics &m){
 Verdict v;
 if(!(m.ping&&m.tcp&&m.interfaces&&m.routes)){v.score=0;v.critical=true;v.reason="sensor_missing";v.breached={"sensor_health"};return v;}
 double h=100;
 if(m.latency>=500){v.breached<<"latency_critical";h-=55;}else if(m.latency>=200){v.breached<<"latency_high";h-=20;}
 if(m.loss>=5){v.breached<<"packet_loss_critical";h-=50;}else if(m.loss>=1){v.breached<<"packet_loss_high";h-=25;}
 if(m.jitter>=100){v.breached<<"jitter_critical";h-=30;}else if(m.jitter>=30){v.breached<<"jitter_high";h-=15;}
 if(m.bandwidth>=95){v.breached<<"bandwidth_critical";h-=20;}
 h=std::clamp(h,0.0,100.0);
 bool hasCrit=std::any_of(v.breached.begin(),v.breached.end(),[](const QString &b){return b.endsWith("_critical");});
 v.critical=h<50||v.breached.size()>=2||hasCrit;
 v.reason=v.breached.isEmpty()?"healthy":v.breached.first();
 if(v.critical&&v.reason.contains("critical"))v.reason="critical_state";
 v.score=std::round(h*100)/100;return v;
}
Recommendation Paradox::recommend(const NetMetrics &m){
 if(m.latency>300||m.loss>3||m.jitter>80||m.bandwidth>90)return {"L3","set_qdisc",QString("critical_state latency=%1ms loss=%2%").arg(m.latency).arg(m.loss),true};
 if(m.latency>200||m.loss>1.5||m.jitter>50)return {"L2","set_tcp_window",QString("high_degradation latency=%1ms jitter=%2ms").arg(m.latency).arg(m.jitter),true};
 return {};
}
double Paradox::gradient(const NetMetrics &c,const NetMetrics &n){return r3((n.latency-c.latency)*.5+(n.loss-c.loss)*2.0+(n.jitter-c.jitter)*.3);}
void Paradox::ingest(const NetMetrics &m){
 Verdict v=evaluate(m);Recommendation r=recommend(m);
 double g=hasPrev?gradient(prev,m):0;prev=m;hasPrev=true;
 streak=v.critical?streak+1:0;
 QString st=v.critical?"critical":v.breached.isEmpty()?"healthy":"degraded";
 health.append(v.score);while(health.size()>90)health.removeFirst();
 bool armed=streak>=3; // streak guard: action only after 3 consecutive critical states
 QString action=r.valid?r.name:QString();
 if(st!=lastState||(!action.isEmpty()&&streak==3)){
  appendProof({{"event",st!=lastState?"state_change":"action_recommended"},{"ts",QDateTime::currentDateTimeUtc().toString(Qt::ISODate)},
   {"state",st},{"reason",v.reason},{"health_score",QString::number(v.score,'f',1)},{"latency_ms",QString::number(m.latency,'f',2)},
   {"packet_loss_pct",QString::number(m.loss,'f',1)},{"jitter_ms",QString::number(m.jitter,'f',2)},{"action",action.isEmpty()?"none":action},{"dry_run",true}});
  lastState=st;}
 snapshot["ready"]=true;snapshot["score"]=v.score;snapshot["state"]=st;snapshot["reason"]=v.reason;snapshot["breached"]=v.breached;
 snapshot["latency"]=m.latency;snapshot["loss"]=m.loss;snapshot["jitter"]=m.jitter;snapshot["bandwidth"]=m.bandwidth;
 snapshot["sensors"]=QVariantMap{{"ping",m.ping},{"tcp",m.tcp},{"interfaces",m.interfaces},{"routes",m.routes}};
 snapshot["gradient"]=g;snapshot["streak"]=streak;snapshot["armed"]=armed&&r.valid;snapshot["level"]=r.valid?r.level:"L0";snapshot["action"]=action;snapshot["actionReason"]=r.reason;
 snapshot["entries"]=counter;snapshot["history"]=health;snapshot["target"]=target;
 emit changed();
}
// SHA-256 hash chain, byte-compatible with `jfp-paradox verify-chain` (sorted keys, compact JSON, zero seed).
void Paradox::loadChainTip(){
 prevHash=QString(64,'0');counter=0;
 for(auto line:slurp(proofPath).split('\n')){auto o=QJsonDocument::fromJson(line).object();if(o.contains("entry_hash")){prevHash=o["entry_hash"].toString();counter=o["counter"].toInteger()+1;}}
}
void Paradox::appendProof(QVariantMap record){
 QDir().mkpath(QFileInfo(proofPath).path());
 record["counter"]=counter;record["prev_hash"]=prevHash;
 QByteArray canon=QJsonDocument(QJsonObject::fromVariantMap(record)).toJson(QJsonDocument::Compact);
 QCryptographicHash h(QCryptographicHash::Sha256);h.addData(prevHash.toUtf8());h.addData(canon);
 QString entry=QString::fromLatin1(h.result().toHex());record["entry_hash"]=entry;
 QFile f(proofPath);if(!f.open(QIODevice::Append))return;
 f.write(QJsonDocument(QJsonObject::fromVariantMap(record)).toJson(QJsonDocument::Compact)+"\n");
 prevHash=entry;counter++;snapshot["entries"]=counter;
}
