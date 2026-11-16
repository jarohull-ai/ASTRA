#include <QtTest>
#include <QTemporaryDir>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickWindow>
#include <QQuickStyle>
#include "backend.h"
class Tests:public QObject {
 Q_OBJECT
private slots:
 void cpuDeltas(){QCOMPARE(Backend::cpuUsage(200,100,100,50),50.0);QCOMPARE(Backend::cpuUsage(100,100,0,0),-1.0);QCOMPARE(Backend::cpuUsage(90,80,100,90),-1.0);}
 void persistence(){QTemporaryDir dir;QSettings::setPath(QSettings::NativeFormat,QSettings::UserScope,dir.path());Backend b;b.reset();b.toggle("gpu");b.move("network",-1);b.setGlass(.6);b.setPalette("ice");Backend restored;QCOMPARE(restored.order(),b.order());QCOMPARE(restored.glass(),.6);QCOMPARE(restored.accent(),QString("#70ddff"));QVERIFY(!restored.order().contains("gpu"));b.reset();}
 void readings(){Backend b;QTest::qWait(1100);b.sample();QCOMPARE(b.cards().size(),6);QVERIFY(b.card("cpu")["available"].toBool());QVERIFY(b.card("memory")["available"].toBool());QVERIFY(b.card("cpu")["level"].toDouble()>=0);QVERIFY(b.card("cpu")["level"].toDouble()<=100);}
 void realWindow(){QQuickStyle::setStyle("Material");Backend b;b.reset();QQmlApplicationEngine e;e.rootContext()->setContextProperty("backend",&b);e.load(QUrl("qrc:/Main.qml"));QVERIFY(!e.rootObjects().isEmpty());auto w=qobject_cast<QQuickWindow*>(e.rootObjects().first());QVERIFY(w);QTest::qWait(200);QTest::keyClick(w,Qt::Key_E);QCOMPARE(w->property("editing").toBool(),true);QTest::keyClick(w,Qt::Key_Space);QVERIFY(b.paused());w->resize(720,600);QTest::qWait(150);QVERIFY(!w->grabWindow().isNull());}
};
QTEST_MAIN(Tests)
#include "tests.moc"
