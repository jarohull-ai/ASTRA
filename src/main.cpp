#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>
#include <QQuickWindow>
#include <QTimer>
#include <QJsonDocument>
#include <cstdio>
#include "backend.h"
#include "translator.h"
int main(int argc,char **argv){
 bool capture=false,probe=false;for(int i=1;i<argc;i++){capture|=QString(argv[i])=="--screenshot";probe|=QString(argv[i])=="--probe";}
 if(capture||probe)qputenv("QT_QPA_PLATFORM","offscreen");
 QGuiApplication app(argc,argv);app.setApplicationName("astramon");app.setOrganizationName("astramon");app.setDesktopFileName("astramon");
 QQuickStyle::setStyle("Material");Translator translator;app.installTranslator(&translator);Backend backend;
 if(probe){QTimer::singleShot(1100,&app,[&]{backend.sample();puts(QJsonDocument::fromVariant(backend.cards()).toJson().constData());app.quit();});return app.exec();}
 QQmlApplicationEngine engine;QObject::connect(&translator,&Translator::changed,&engine,&QQmlEngine::retranslate);engine.rootContext()->setContextProperty("backend",&backend);engine.load(QUrl("qrc:/Main.qml"));if(engine.rootObjects().isEmpty())return 1;
 if(capture)QTimer::singleShot(3500,&app,[&]{auto w=qobject_cast<QQuickWindow*>(engine.rootObjects().first());bool ok=w->grabWindow().save(app.arguments().value(app.arguments().indexOf("--screenshot")+1,"/tmp/astramon.png"));app.exit(ok?0:2);});
 return app.exec();
}
