#include "../accounts/profilemanager.h"
#include "../core/logbuffer.h"
#include "../core/logging.h"
#include <KAboutData>
#include <KIconTheme>
#include <KLocalizedQmlContext>
#include <KLocalizedString>
#include <QApplication>
#include <QGuiApplication>
#include <QIcon>
#include <QQmlApplicationEngine>
#include <QQuickStyle>
#include <QtQml>
#include <kaboutdata.h>
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
    QApplication::setDesktopFileName(QStringLiteral("com.someblocks.d2rloader"));
    LogBuffer::install();

    qCInfo(LOG_APP) << "D2RLoader started" << APP_VERSION;

    QApplication::setStyle(QStringLiteral("breeze"));
    if (qEnvironmentVariableIsEmpty("QT_QUICK_CONTROLS_STYLE")) {
        QQuickStyle::setStyle(QStringLiteral("org.kde.desktop"));
    }
    QApplication::setWindowIcon(QIcon::fromTheme(QStringLiteral("com.someblocks.d2rloader")));

    KAboutData aboutData(QStringLiteral("d2rloader"),
                         i18nc("@title", "D2RLoader"),
                         QStringLiteral(APP_VERSION),
                         i18n("A Diablo 2 Resurrected Loader"),
                         KAboutLicense::MIT);

    aboutData.addAuthor(i18nc("@info:credit", "Peter Justin"),
                        i18nc("@info:credit", "Lead Developer"),
                        QStringLiteral("peter.justin@outlook.com"),
                        QStringLiteral("https://peterjustin.com"));
    aboutData.addCredit(QStringLiteral("Lorc"),
                        i18nc("@info:credit", "Application icon based on \"Diablo skull\" from game-icons.net, licensed under CC BY 3.0"),
                        QString(),
                        QStringLiteral("https://game-icons.net/1x1/lorc/diablo-skull.html"));

    aboutData.setHomepage(QStringLiteral("https://github.com/sh4nks/d2rloader"));
    aboutData.setBugAddress("https://github.com/sh4nks/d2rloader/issues");
    aboutData.setCopyrightStatement(QStringLiteral("Copyright (c) 2025 - 2026 Peter Justin"));
    aboutData.setOrganizationDomain("someblocks.com"); //
    aboutData.setDesktopFileName(QStringLiteral("com.someblocks.d2rloader"));
    KAboutData::setApplicationData(aboutData);

    QQmlApplicationEngine engine;
    KLocalization::setupLocalizedContext(&engine);
    ProfileManager::instance().loadProfiles();

    // engine.load(QUrl(QStringLiteral("qrc:/qml/Main.qml")));
    engine.loadFromModule("com.someblocks.d2rloader", "Main");
    if (engine.rootObjects().isEmpty()) {
        return -1;
    }

    return app.exec();
}
