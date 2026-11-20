#include "../accounts/profilemanager.h"
#include "../core/logbuffer.h"
#include "../core/logging.h"
#include "gameinfo.h"
#include "gamemanager.h"
#include <KAboutData>
#include <KIconTheme>
#include <KLocalizedQmlContext>
#include <KLocalizedString>
#include <QApplication>
#include <QCommandLineParser>
#include <QDataStream>
#include <QFileInfo>
#include <QGuiApplication>
#include <QIcon>
#include <QLocalServer>
#include <QLocalSocket>
#include <QQmlApplicationEngine>
#include <QQuickStyle>
#include <QStandardPaths>
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
/**
 * Where the D2RLoader that is running takes the accounts and launch sequences
 * a later one was started with.
 */
QString instanceSocket()
{
#ifdef Q_OS_WIN
    return QStringLiteral("com.someblocks.d2rloader");
#else
    QString directory = QStandardPaths::writableLocation(QStandardPaths::RuntimeLocation);
    // The rest of the runtime directory is private to each instance of a Flatpak.
    if (QFileInfo::exists(QStringLiteral("/.flatpak-info"))) {
        directory += QStringLiteral("/app/com.someblocks.d2rloader");
    }
    return directory + QStringLiteral("/d2rloader.sock");
#endif
}

/**
 * Hands \a accounts and \a sequences to the D2RLoader that is running.
 * Returns false when there is none.
 */
bool forwardLaunch(const QStringList &accounts, const QStringList &sequences)
{
    QLocalSocket socket;
    socket.connectToServer(instanceSocket());
    if (!socket.waitForConnected(1000)) {
        return false;
    }

    QDataStream stream(&socket);
    stream << accounts << sequences;
    // The running instance hangs up once it has taken the request.
    return socket.waitForBytesWritten(1000) && (socket.state() == QLocalSocket::UnconnectedState || socket.waitForDisconnected(5000));
}

void listenForLaunches(QLocalServer *server, GameManager *manager)
{
    // Listening replaces the socket of a D2RLoader that is running, so that one is asked first.
    QLocalSocket probe;
    probe.connectToServer(instanceSocket());
    if (probe.waitForConnected(1000)) {
        qCInfo(LOG_APP) << "Another D2RLoader is running and keeps taking the launches of later ones";
        return;
    }

    // left behind by a D2RLoader that did not exit cleanly
    QLocalServer::removeServer(instanceSocket());
    server->setSocketOptions(QLocalServer::UserAccessOption);
    if (!server->listen(instanceSocket())) {
        qCWarning(LOG_APP) << "Cannot take launches from later D2RLoaders:" << server->errorString();
        return;
    }

    QObject::connect(server, &QLocalServer::newConnection, manager, [server, manager] {
        while (QLocalSocket *socket = server->nextPendingConnection()) {
            QObject::connect(socket, &QLocalSocket::disconnected, socket, &QObject::deleteLater);
            QObject::connect(socket, &QLocalSocket::readyRead, manager, [socket, manager] {
                QDataStream stream(socket);
                QStringList accounts;
                QStringList sequences;
                stream.startTransaction();
                stream >> accounts >> sequences;
                if (stream.commitTransaction()) {
                    qCInfo(LOG_APP) << "Another D2RLoader asked to start the accounts" << accounts << "and the launch sequences" << sequences;
                    manager->startByName(accounts, sequences);
                    socket->disconnectFromServer();
                }
            });
        }
    });
}

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
    aboutData.addCredit(QStringLiteral("d2emu.com"),
                        i18nc("@info:credit", "Terror Zone and Diablo Clone data"),
                        QString(),
                        QStringLiteral("https://d2emu.com"));

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
    const QCommandLineOption launchOption(QStringLiteral("launch"),
                                          i18n("Start the game of this account. Can be given more than once."),
                                          i18nc("@info:shell value of the launch option", "account"));
    parser.addOption(launchOption);
    const QCommandLineOption sequenceOption(QStringLiteral("sequence"),
                                            i18n("Start the accounts of this launch sequence. Can be given more than once."),
                                            i18nc("@info:shell value of the sequence option", "name"));
    parser.addOption(sequenceOption);
    const QCommandLineOption minimizedOption(QStringLiteral("minimized"), i18n("Start hidden in the system tray, or minimized where there is none."));
    parser.addOption(minimizedOption);
    parser.process(app);
    aboutData.processCommandLine(&parser);
    GameInfo::setDebugMode(parser.isSet(debugOption));

    const bool launches = parser.isSet(launchOption) || parser.isSet(sequenceOption);
    if (launches && forwardLaunch(parser.values(launchOption), parser.values(sequenceOption))) {
        return 0;
    }

    QQmlApplicationEngine engine;
    KLocalization::setupLocalizedContext(&engine);
    ProfileManager::instance().loadProfiles();

    if (parser.isSet(minimizedOption)) {
        engine.setInitialProperties({{QStringLiteral("startMinimized"), true}});
    }

    // engine.load(QUrl(QStringLiteral("qrc:/qml/Main.qml")));
    engine.loadFromModule("com.someblocks.d2rloader", "Main");
    if (engine.rootObjects().isEmpty()) {
        return -1;
    }

    auto *manager = engine.singletonInstance<GameManager *>("com.someblocks.d2rloader", "GameManager");
    QLocalServer instanceServer;
    listenForLaunches(&instanceServer, manager);
    if (launches) {
        manager->startByName(parser.values(launchOption), parser.values(sequenceOption));
    }

    return app.exec();
}
