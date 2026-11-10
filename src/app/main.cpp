#include "../accounts/profilemanager.h"
#include "../core/logbuffer.h"
#include "../core/logging.h"
#include "gameinfo.h"
#include <KAboutData>
#include <KIconTheme>
#include <KLocalizedQmlContext>
#include <KLocalizedString>
#include <QApplication>
#include <QCommandLineParser>
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

#ifdef Q_OS_WIN
#include "../core/updatechecker.h"
#include <shobjidl.h>
#endif

namespace
{
#ifdef Q_OS_WIN
// The installer puts this AppUserModelID on its Start Menu shortcut, which is
// what makes Windows show the app's name and icon for the process in toasts
// and the taskbar. Without such a shortcut, as with the portable zip, Windows
// would label toasts with the raw ID instead of the executable's description,
// so only an installed copy takes the ID.
void setAppUserModelId()
{
    if (!UpdateChecker::runsFromInstalledCopy()) {
        return;
    }
    qCInfo(LOG_APP) << "Running from the installed location, taking its AppUserModelID";
    SetCurrentProcessExplicitAppUserModelID(L"someblocks.D2RLoader");
}
#endif
}

int main(int argc, char *argv[])
{
    KIconTheme::initTheme();
    QApplication app(argc, argv);
    // Closing the settings window while the main one is hidden in the tray
    // must not quit; Main.qml decides what closing the main window does.
    QApplication::setQuitOnLastWindowClosed(false);
    KLocalizedString::setApplicationDomain("com.someblocks.d2rloader");
    QApplication::setOrganizationName(QStringLiteral("someblocks"));
    QApplication::setOrganizationDomain(QStringLiteral("someblocks.com"));
    QApplication::setApplicationName(QStringLiteral("D2RLoader"));
    QApplication::setDesktopFileName(QStringLiteral("com.someblocks.d2rloader"));
    LogBuffer::install();

    qCInfo(LOG_APP) << "D2RLoader started" << APP_VERSION;
#ifdef Q_OS_WIN
    setAppUserModelId();
#endif

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
    aboutData.addCredit(QStringLiteral("d2emu.com"), i18nc("@info:credit", "Terror Zone and Diablo Clone data"), QString(), QStringLiteral("https://d2emu.com"));

    aboutData.setHomepage(QStringLiteral("https://github.com/sh4nks/d2rloader2"));
    aboutData.setBugAddress("https://github.com/sh4nks/d2rloader2/issues");
    aboutData.setCopyrightStatement(QStringLiteral("Copyright (c) 2025 - 2026 Peter Justin"));
    aboutData.setOrganizationDomain("someblocks.com");
    aboutData.setDesktopFileName(QStringLiteral("com.someblocks.d2rloader"));
    KAboutData::setApplicationData(aboutData);

    QCommandLineParser parser;
    aboutData.setupCommandLine(&parser);
    const QCommandLineOption debugOption(QStringLiteral("debug"), i18n("Show a Debug menu with actions that simulate notifications."));
    parser.addOption(debugOption);
    parser.process(app);
    aboutData.processCommandLine(&parser);
    GameInfo::setDebugMode(parser.isSet(debugOption));

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
