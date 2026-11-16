#pragma once
#include <QObject>
#include <QVariantList>
#include <QTimer>
#include <QElapsedTimer>
#include <QSettings>
class Backend : public QObject {
 Q_OBJECT
 Q_PROPERTY(QVariantList cards READ cards NOTIFY changed)
 Q_PROPERTY(QString accent READ accent NOTIFY changed)
 Q_PROPERTY(QStringList order READ order NOTIFY layoutChanged)
 Q_PROPERTY(bool paused READ paused WRITE setPaused NOTIFY changed)
 Q_PROPERTY(double glass READ glass WRITE setGlass NOTIFY layoutChanged)
 Q_PROPERTY(QString palette READ palette WRITE setPalette NOTIFY layoutChanged)
public:
 explicit Backend(QObject *parent=nullptr);
 QVariantList cards() const {return data;}
 QString accent() const {return color;}
 QStringList order() const {return ids;}
 bool paused() const {return stopped;}
 double glass() const {return opacity;}
 QString palette() const {return scheme;}
 void setPaused(bool p){if(stopped&&!p){lastCpu=lastIdle=lastRx=lastTx=0;elapsed.restart();}stopped=p;emit changed();}
 void setGlass(double v);
 void setPalette(QString v);
 Q_INVOKABLE void toggle(QString id);
 Q_INVOKABLE void move(QString id,int delta);
 Q_INVOKABLE void reset();
 Q_INVOKABLE QVariantMap card(QString id) const;
 Q_INVOKABLE void sample();
 static double cpuUsage(quint64 total,quint64 idle,quint64 oldTotal,quint64 oldIdle);
signals:
 void changed();
 void layoutChanged();
private:
 void save();
 void put(QString id,QString label,QString value,QString unit,QString detail,double level,bool available=true);
 QSettings settings;
 QTimer timer;
 QElapsedTimer elapsed;
 QVariantList data;
 QStringList ids;
 QString color="#31f7a5", scheme="omarchy";
 bool stopped=false;
 double opacity=.83;
 quint64 lastCpu=0,lastIdle=0,lastRx=0,lastTx=0;
 QMap<QString,QVariantList> history;
};
