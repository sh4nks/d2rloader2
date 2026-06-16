#include "d2rloader.h"
#include "settingsmanager.h"
#include <KAboutData>
#include <KIconTheme>
#include <KLocalizedContext>
#include <KLocalizedString>
#include <QApplication>
#include <QGuiApplication>
#include <QIcon>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>
#include <QtQml>
#include <kaboutdata.h>
#include <print>
#include <qhashfunctions.h>
#include <qlogging.h>
#include <qobject.h>
#include <qstylefactory.h>

int main(int argc, char *argv[])
{
    KIconTheme::initTheme();
    QApplication app(argc, argv);
    KLocalizedString::setApplicationDomain("com.someblocks.d2rloader");
    QApplication::setOrganizationName(QStringLiteral("someblocks"));
    QApplication::setOrganizationDomain(QStringLiteral("someblocks.com"));
    QApplication::setApplicationName(QStringLiteral("D2RLoader"));
    QApplication::setDesktopFileName(QStringLiteral("d2rloader"));
    D2RLoader *d2rloader = D2RLoader::getInstance();

    std::println("D2RLoader started {}", d2rloader->appVersion().toStdString());

    // Init app components
    SettingsManager *sm = SettingsManager::getInstance();
    if (!sm) {
        qWarning() << "Cannot init app components!";
        return EXIT_FAILURE;
    }

    QApplication::setStyle(QStringLiteral("breeze"));
    if (qEnvironmentVariableIsEmpty("QT_QUICK_CONTROLS_STYLE")) {
        QQuickStyle::setStyle(QStringLiteral("org.kde.desktop"));
    }
    QApplication::setWindowIcon(QIcon::fromTheme(QStringLiteral("d2rloader")));

    KAboutData aboutData(QStringLiteral("d2rloader"),
                         i18nc("@title", "D2RLoader"),
                         QStringLiteral("1.0"),
                         i18n("A Diablo 2 Resurrected Loader"),
                         KAboutLicense::MIT);

    aboutData.addAuthor(i18nc("@info:credit", "Peter Justin"),
                        i18nc("@info:credit", "Lead Developer"),
                        QStringLiteral("peter.justin@outlook.com"),
                        QStringLiteral("https://peterjustin.com"));

    aboutData.setHomepage(QStringLiteral("https://github.com/sh4nks/d2rloader"));
    aboutData.setBugAddress("https://github.com/sh4nks/d2rloader/issues");
    aboutData.setCopyrightStatement(QStringLiteral("Copyright (c) 2025 - 2026 Peter Justin"));
    aboutData.setOrganizationDomain("someblocks.com"); //
    aboutData.setDesktopFileName(QStringLiteral("d2rloader"));
    KAboutData::setApplicationData(aboutData);

    // Register a singleton that will be accessible from QML.
    qmlRegisterSingletonType("com.someblocks.d2rloader", // How the import statement should look like
                             1,
                             0, // Major and minor versions of the import
                             "About", // The name of the QML object
                             [](QQmlEngine *engine, QJSEngine *) -> QJSValue {
                                 // Here we retrieve our aboutData and give it to the QML engine
                                 // to turn it into a QML type
                                 return engine->toScriptValue(KAboutData::applicationData());
                             });

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextObject(new KLocalizedContext(&engine));
    // engine.rootContext()->setContextProperty(QStringLiteral("profileTableModel"), tableProxy);
    // engine.rootContext()->setContextProperty(QStringLiteral("settingsManager"), sm);
    engine.rootContext()->setContextProperty(QStringLiteral("app"), d2rloader);
    qDebug() << engine.importPathList();

    // engine.load(QUrl(QStringLiteral("qrc:/qml/Main.qml")));
    engine.loadFromModule("com.someblocks.d2rloader", "Main");
    if (engine.rootObjects().isEmpty()) {
        return -1;
    }

    return app.exec();
}
