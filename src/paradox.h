#pragma once
// JFP PARADOX network governance inside ASTRA.
// Governor / Supervisor rules mirror jfp-paradox v0.2.0 rust-core (lib.rs) 1:1.
// Observe-only (dry run): actions are recommended and proof-logged, never executed.
#include <QObject>
#include <QVariantList>
#include <QVariantMap>
#include <QStringList>
#include <QTimer>
#include <QElapsedTimer>
#include <QProcess>
#include <QTcpSocket>
struct NetMetrics {double latency=0,loss=0,jitter=0,bandwidth=0;bool ping=true,tcp=true,interfaces=true,routes=true;};
struct Verdict {double score=0;bool critical=false;QString reason;QStringList breached;};
struct Recommendation {QString level,name,reason;bool valid=false;};
class Paradox : public QObject {
 Q_OBJECT
 Q_PROPERTY(QVariantMap state READ state NOTIFY changed)
 Q_PROPERTY(bool paused READ paused WRITE setPaused NOTIFY changed)
public:
 explicit Paradox(QObject *parent=nullptr,QString proofPath={},bool autostart=true);
 QVariantMap state() const {return snapshot;}
 bool paused() const {return stopped;}
 void setPaused(bool p){stopped=p;emit changed();}
 Q_INVOKABLE void probe();
 static Verdict evaluate(const NetMetrics &m);
 static Recommendation recommend(const NetMetrics &m);
 static double gradient(const NetMetrics &current,const NetMetrics &next);
 static bool parsePing(const QString &out,double &avg,double &mdev,double &loss);
 void ingest(const NetMetrics &m);
 QString proofFile() const {return proofPath;}
signals:
 void changed();
private:
 void finishProbe();
 void appendProof(QVariantMap record);
 void loadChainTip();
 QTimer timer;
 QProcess ping;
 QTcpSocket tcp;
 QElapsedTimer tcpClock;
 QString target="8.8.8.8",proofPath,prevHash;
 int tcpPort=443,pending=0,streak=0;
 qint64 counter=0;
 bool stopped=false,tcpOk=false,pingOk=false;
 double pAvg=0,pMdev=0,pLoss=100;
 quint64 lastBytes=0;QElapsedTimer bwClock;
 bool hasPrev=false;NetMetrics prev;QString lastState;
 QVariantList health;
 QVariantMap snapshot;
};
