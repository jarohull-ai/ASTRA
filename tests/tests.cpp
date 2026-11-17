#include <QtTest>
#include <QTemporaryDir>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickWindow>
#include <QQuickStyle>
#include "backend.h"
#include "paradox.h"
#include "translator.h"
#include <QCryptographicHash>
#include <QJsonDocument>
#include <QJsonObject>
class Tests:public QObject {
 Q_OBJECT
private slots:
 void cpuDeltas(){QCOMPARE(Backend::cpuUsage(200,100,100,50),50.0);QCOMPARE(Backend::cpuUsage(100,100,0,0),-1.0);QCOMPARE(Backend::cpuUsage(90,80,100,90),-1.0);}
 void persistence(){QTemporaryDir dir;QSettings::setPath(QSettings::NativeFormat,QSettings::UserScope,dir.path());Backend b;b.reset();b.toggle("gpu");b.move("network",-1);b.setGlass(.6);b.setPalette("ice");Backend restored;QCOMPARE(restored.order(),b.order());QCOMPARE(restored.glass(),.6);QCOMPARE(restored.accent(),QString("#70ddff"));QVERIFY(!restored.order().contains("gpu"));b.reset();}
 void readings(){Backend b;QTest::qWait(1100);b.sample();QCOMPARE(b.cards().size(),6);QVERIFY(b.card("cpu")["available"].toBool());QVERIFY(b.card("memory")["available"].toBool());QVERIFY(b.card("cpu")["level"].toDouble()>=0);QVERIFY(b.card("cpu")["level"].toDouble()<=100);}
 void paradoxGovernorParity(){
  // same cases as jfp-paradox rust-core test_governor_normal_and_critical
  NetMetrics n;n.latency=20;n.loss=0;n.jitter=2;n.bandwidth=30;auto v1=Paradox::evaluate(n);QVERIFY(!v1.critical);QCOMPARE(v1.score,100.0);QCOMPARE(v1.reason,QString("healthy"));
  NetMetrics d;d.latency=550;d.loss=2;d.jitter=40;d.bandwidth=85;auto v2=Paradox::evaluate(d);QVERIFY(v2.critical);QVERIFY(v2.score<50);QCOMPARE(v2.reason,QString("critical_state"));
  NetMetrics c;c.latency=600;c.loss=6;c.jitter=120;c.bandwidth=10;auto v3=Paradox::evaluate(c);QCOMPARE(v3.score,0.0);QCOMPARE(v3.breached,QStringList({"latency_critical","packet_loss_critical","jitter_critical"}));
  NetMetrics s;s.tcp=false;auto v4=Paradox::evaluate(s);QCOMPARE(v4.reason,QString("sensor_missing"));QCOMPARE(v4.score,0.0);
  NetMetrics h;h.latency=250;auto v5=Paradox::evaluate(h);QVERIFY(!v5.critical);QCOMPARE(v5.score,80.0);
  QCOMPARE(Paradox::recommend(c).name,QString("set_qdisc"));QCOMPARE(Paradox::recommend(h).name,QString("set_tcp_window"));QVERIFY(!Paradox::recommend(n).valid);
  QCOMPARE(Paradox::gradient(n,d),280.4);}
 void paradoxPingParse(){double a,m,l;QVERIFY(Paradox::parsePing("5 packets transmitted, 5 received, 0% packet loss, time 802ms\nrtt min/avg/max/mdev = 15.470/16.173/18.040/1.080 ms",a,m,l));QCOMPARE(a,16.173);QCOMPARE(m,1.08);QCOMPARE(l,0.0);
  QVERIFY(!Paradox::parsePing("5 packets transmitted, 0 received, 100% packet loss",a,m,l));QCOMPARE(l,100.0);QVERIFY(!Paradox::parsePing("",a,m,l));}
 void paradoxProofChain(){QTemporaryDir dir;QString f=dir.path()+"/proof.jsonl";
  {Paradox p(nullptr,f,false);NetMetrics ok;ok.latency=20;p.ingest(ok);NetMetrics bad;bad.latency=600;bad.loss=6;for(int i=0;i<4;i++)p.ingest(bad);p.ingest(ok);QCOMPARE(p.state()["entries"].toInt(),4);QCOMPARE(p.state()["state"].toString(),QString("healthy"));}
  {Paradox p(nullptr,f,false);NetMetrics bad;bad.latency=600;p.ingest(bad);} // resumes chain after restart
  QFile in(f);QVERIFY(in.open(QIODevice::ReadOnly));QString prev(64,'0');int count=0;
  for(auto line:in.readAll().split('\n')){if(line.trimmed().isEmpty())continue;auto o=QJsonDocument::fromJson(line).object();QCOMPARE(o["prev_hash"].toString(),prev);QString rec=o["entry_hash"].toString();o.remove("entry_hash");
   QCryptographicHash h(QCryptographicHash::Sha256);h.addData(prev.toUtf8());h.addData(QJsonDocument(o).toJson(QJsonDocument::Compact));QCOMPARE(QString::fromLatin1(h.result().toHex()),rec);QCOMPARE(o["counter"].toInt(),count);prev=rec;count++;}
  QCOMPARE(count,5);}
 void languageSwitch(){QTemporaryDir d;Translator t(d.path()+"/lang");QCoreApplication::installTranslator(&t);
  t.setChoice("pl");QCOMPARE(t.language(),QString("pl"));QCOMPARE(QCoreApplication::translate("x","Edit layout"),QString("Edytuj układ"));QCOMPARE(QCoreApplication::translate("x","Processor"),QString("Procesor"));
  t.setChoice("en");QCOMPARE(t.language(),QString("en"));QCOMPARE(QCoreApplication::translate("x","Edit layout"),QString("Edit layout"));
  t.setChoice("auto");QCOMPARE(t.choice(),QString("auto"));QCOMPARE(t.language(),QString("en"));t.setChoice("bogus!");QCOMPARE(t.choice(),QString("auto"));
  QCoreApplication::removeTranslator(&t);}
 void realWindow(){QQuickStyle::setStyle("Material");Backend b;b.reset();QTemporaryDir pd;Paradox px(nullptr,pd.path()+"/p.jsonl",false);Translator tr(pd.path()+"/none");QQmlApplicationEngine e;e.rootContext()->setContextProperty("backend",&b);e.rootContext()->setContextProperty("paradox",&px);e.rootContext()->setContextProperty("i18n",&tr);e.load(QUrl("qrc:/Main.qml"));QVERIFY(!e.rootObjects().isEmpty());auto w=qobject_cast<QQuickWindow*>(e.rootObjects().first());QVERIFY(w);QTest::qWait(200);QTest::keyClick(w,Qt::Key_E);QCOMPARE(w->property("editing").toBool(),true);QTest::keyClick(w,Qt::Key_Space);QVERIFY(b.paused());w->resize(720,600);QTest::qWait(150);QVERIFY(!w->grabWindow().isNull());}
};
QTEST_MAIN(Tests)
#include "tests.moc"
