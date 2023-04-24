#include <QtGui/QGuiApplication>
#include <QQmlApplicationEngine>
//#include <QSurfaceFormat>

int main(int argc, char* argv[])
{
  QGuiApplication app(argc, argv);
  QQmlApplicationEngine engine;
//  QtQuick2ApplicationViewer view;

//  QSurfaceFormat format;
//  format.setSamples(16);
//  engine.view.setFormat(format);

  engine.addImportPath(":/");
  const QUrl url(u"qrc:/simple/main.qml"_qs);
  QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                   &app, [url](QObject *obj, const QUrl &objUrl) {
      if (!obj && url == objUrl)
          QCoreApplication::exit(-1);
  }, Qt::QueuedConnection);
  engine.load(url);

  return app.exec();
}

