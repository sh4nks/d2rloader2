#include "kwinwindowpositions.h"
#include "../accounts/profile.h"
#include "../accounts/profilemanager.h"
#include "../core/logging.h"

#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusReply>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QSaveFile>
#include <QStandardPaths>

namespace
{
const QString Service = QStringLiteral("com.someblocks.d2rloader");
const QString ObjectPath = QStringLiteral("/WindowPositions");
const QString Interface = QStringLiteral("com.someblocks.d2rloader.WindowPositions");

const QString KWinService = QStringLiteral("org.kde.KWin");
const QString ScriptingPath = QStringLiteral("/Scripting");
const QString ScriptingInterface = QStringLiteral("org.kde.kwin.Scripting");
const QString ScriptInterface = QStringLiteral("org.kde.kwin.Script");
const QString PluginName = QStringLiteral("d2rloader-window-positions");

/**
 * KWin reads the script from disk, so it goes where the host sees it at the
 * same path, from inside the Flatpak too.
 */
QString scriptPath()
{
    return QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation) + QStringLiteral("/d2rloader/kwin-window-positions.js");
}

QDBusMessage scriptingCall(const QString &method, const QVariantList &arguments)
{
    QDBusMessage message = QDBusMessage::createMethodCall(KWinService, ScriptingPath, ScriptingInterface, method);
    message.setArguments(arguments);
    return message;
}

Profile *profileWithWindowTitle(const QString &title)
{
    const QList<Profile *> profiles = ProfileManager::instance().profiles();
    for (Profile *profile : profiles) {
        // Steam's game window keeps the game's own title.
        if (profile->authMethod() != AuthMethodModel::Steam && profile->windowTitle() == title) {
            return profile;
        }
    }
    return nullptr;
}
}

bool KWinWindowPositions::isSupported()
{
    return qEnvironmentVariable("XDG_CURRENT_DESKTOP").split(u':').contains(QStringLiteral("KDE"));
}

KWinWindowPositions::KWinWindowPositions(QObject *parent)
    : QDBusVirtualObject(parent)
{
    // The script finds the loader through its name. A second loader cannot
    // have it and leaves the first one's script alone.
    QDBusConnection bus = QDBusConnection::sessionBus();
    if (!bus.registerService(Service) || !bus.registerVirtualObject(ObjectPath, this)) {
        qCWarning(LOG_GAME) << "window positions are not kept, could not register" << Service << "on the session bus:" << bus.lastError().message();
        return;
    }
    loadScript();
}

KWinWindowPositions::~KWinWindowPositions()
{
    if (m_scriptLoaded) {
        QDBusConnection::sessionBus().call(scriptingCall(QStringLiteral("unloadScript"), {PluginName}));
    }
}

void KWinWindowPositions::loadScript()
{
    QFile script(QStringLiteral(":/kwin/windowpositions.js"));
    QSaveFile file(scriptPath());
    if (!script.open(QIODevice::ReadOnly) || !QDir().mkpath(QFileInfo(scriptPath()).absolutePath()) || !file.open(QIODevice::WriteOnly)
        || file.write(script.readAll()) < 0 || !file.commit()) {
        qCWarning(LOG_GAME) << "window positions are not kept, could not write" << scriptPath() << ":" << file.errorString();
        return;
    }

    // A script left behind by a loader that did not quit cleanly holds the
    // plugin name, and may be from an older version.
    QDBusConnection bus = QDBusConnection::sessionBus();
    bus.call(scriptingCall(QStringLiteral("unloadScript"), {PluginName}));

    const QDBusReply<int> id = bus.call(scriptingCall(QStringLiteral("loadScript"), {scriptPath(), PluginName}));
    if (!id.isValid() || id.value() < 0) {
        qCWarning(LOG_GAME) << "window positions are not kept, KWin did not load" << scriptPath() << ":" << id.error().message();
        return;
    }

    const QString path = ScriptingPath + QStringLiteral("/Script%1").arg(id.value());
    const QDBusMessage reply = bus.call(QDBusMessage::createMethodCall(KWinService, path, ScriptInterface, QStringLiteral("run")));
    if (reply.type() == QDBusMessage::ErrorMessage) {
        qCWarning(LOG_GAME) << "window positions are not kept, KWin did not run" << scriptPath() << ":" << reply.errorMessage();
        return;
    }

    m_scriptLoaded = true;
    qCInfo(LOG_GAME) << "keeping window positions through the KWin script" << scriptPath();
}

QString KWinWindowPositions::introspect(const QString &path) const
{
    Q_UNUSED(path)
    return QStringLiteral(R"(<interface name="%1">
  <method name="windowPosition">
    <arg name="title" type="s" direction="in"/>
    <arg name="found" type="b" direction="out"/>
    <arg name="x" type="i" direction="out"/>
    <arg name="y" type="i" direction="out"/>
  </method>
  <method name="saveWindowPosition">
    <arg name="title" type="s" direction="in"/>
    <arg name="x" type="i" direction="in"/>
    <arg name="y" type="i" direction="in"/>
  </method>
</interface>
)")
        .arg(Interface);
}

bool KWinWindowPositions::handleMessage(const QDBusMessage &message, const QDBusConnection &connection)
{
    const QVariantList arguments = message.arguments();
    if ((!message.interface().isEmpty() && message.interface() != Interface) || arguments.isEmpty()) {
        return false;
    }

    Profile *profile = profileWithWindowTitle(arguments.constFirst().toString());
    const bool remembered = profile && profile->rememberWindowPosition();

    if (message.member() == QLatin1String("windowPosition") && arguments.size() == 1) {
        const QPoint position = remembered ? profile->windowPosition().value_or(QPoint()) : QPoint();
        const bool found = remembered && profile->windowPosition().has_value();
        connection.send(message.createReply(QVariantList{found, position.x(), position.y()}));
        return true;
    }

    if (message.member() == QLatin1String("saveWindowPosition") && arguments.size() == 3) {
        if (remembered) {
            profile->setWindowPosition(QPoint(qRound(arguments.at(1).toDouble()), qRound(arguments.at(2).toDouble())));
        }
        connection.send(message.createReply());
        return true;
    }

    return false;
}
