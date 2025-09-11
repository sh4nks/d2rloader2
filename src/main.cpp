#include "d2rloader.h"
#include "settingsmanager.h"
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickStyle>
#include <QQmlContext>
#include <print>

int main(int argc, char* argv[])
{
    QGuiApplication app(argc, argv);
    app.setApplicationName("D2RLoader");


    D2RLoader *d2rloader = D2RLoader::getInstance();

    std::println("D2RLoader started {}", d2rloader->appName().toStdString());

    // Init app components
    SettingsManager *sm = SettingsManager::getInstance();
    if (!sm)
    {
        qWarning() << "Cannot init app components!";
        return EXIT_FAILURE;
    }

    QQmlApplicationEngine engine;
    // QQuickStyle::setStyle("Fusion");

    qDebug() << engine.importPathList();

    engine.rootContext()->setContextProperty("settingsManager", sm);
    engine.rootContext()->setContextProperty("app", d2rloader);

    engine.addImportPath("qrc:/qml");
    // _qmlAppEngine->load(QUrl(QStringLiteral("qrc:/qml/main.qml")));
    engine.loadFromModule("D2RLoader", "Main");
    if (engine.rootObjects().isEmpty()) {
        return -1;
    }

    return app.exec();
}
