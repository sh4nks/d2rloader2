#include "profile.h"
#include "authmethod.h"
#include "d2rloaderconfig.h"
#include "region.h"
#include <QDir>
#include <QJsonObject>
#include <QMetaEnum>
#include <QRegularExpression>
#include <qhashfunctions.h>
#include <qobject.h>

namespace
{
/**
 * Enums are persisted by name rather than by value, so that reordering an
 * enum cannot silently reinterpret already stored accounts.
 */
template<typename Enum>
QString enumToKey(Enum value)
{
    const char *key = QMetaEnum::fromType<Enum>().valueToKey(static_cast<int>(value));
    return key ? QString::fromLatin1(key) : QString();
}

template<typename Enum>
Enum enumFromKey(const QJsonValue &value, Enum fallback)
{
    bool ok = false;
    const int result = QMetaEnum::fromType<Enum>().keyToValue(value.toString().toLatin1().constData(), &ok);
    return ok ? static_cast<Enum>(result) : fallback;
}
}

Profile::Profile(QObject *parent)
    : QObject(parent)
{
}

QString Profile::normalizedName() const
{
    static const QRegularExpression separators(QStringLiteral(R"([\t !"#$%&'()*\-/<=>?@\[\\\]^_`{|},.+]+)"));

    const auto normalize = [](const QString &text) {
        // Decomposing first turns accented letters into their ASCII base
        // letter plus a combining mark, which is then dropped along with
        // anything else outside ASCII.
        QString ascii;
        for (const QChar c : text.normalized(QString::NormalizationForm_KD)) {
            if (c.unicode() < 128) {
                ascii.append(c);
            }
        }
        return ascii.toLower().split(separators, Qt::SkipEmptyParts).join(u'-');
    };

    for (const QString &candidate : {normalize(m_profileName), normalize(m_email)}) {
        if (!candidate.isEmpty()) {
            return candidate;
        }
    }
    return QStringLiteral("account-%1").arg(m_id);
}

QString Profile::wineprefix() const
{
    return QDir(D2RLoaderConfig::self()->wineprefixPath()).filePath(normalizedName());
}

QString Profile::windowTitle() const
{
    const QString name = m_profileName.isEmpty() ? m_email : m_profileName;
    return QStringLiteral("%1 (%2)").arg(name, RegionModel::server(m_region));
}

int Profile::id() const
{
    return m_id;
}

void Profile::setId(const int id)
{
    if (m_id != id) {
        m_id = id;
    }
}

ProfileState::Type Profile::status() const
{
    return m_status;
}

void Profile::setStatus(const ProfileState::Type status)
{
    if (status == m_status) {
        return;
    }
    const auto oldStatus = std::exchange(m_status, status);
    Q_EMIT statusChanged(oldStatus, m_status);
}

QString Profile::profileName() const
{
    return m_profileName;
}

void Profile::setProfileName(const QString &name)
{
    if (m_profileName != name) {
        m_profileName = name;
        Q_EMIT profileNameChanged();
    }
}

AuthMethodModel::AuthMethod Profile::authMethod() const
{
    return m_authMethod;
}

void Profile::setAuthMethod(const AuthMethodModel::AuthMethod &authMethod)
{
    if (m_authMethod != authMethod) {
        m_authMethod = authMethod;
        Q_EMIT authMethodChanged();
    }
}

RegionModel::Region Profile::region() const
{
    return m_region;
}

void Profile::setRegion(const RegionModel::Region &region)
{
    if (m_region != region) {
        m_region = region;
        Q_EMIT regionChanged();
    }
}

QString Profile::email() const
{
    return m_email;
}

void Profile::setEmail(const QString &email)
{
    if (m_email != email) {
        m_email = email;
        Q_EMIT emailChanged();
    }
}

QString Profile::password() const
{
    return m_password;
}

void Profile::setPassword(const QString &password)
{
    if (m_password != password) {
        m_password = password;
        Q_EMIT passwordChanged();
    }
}

QString Profile::token() const
{
    return m_token;
}

void Profile::setToken(const QString &token)
{
    if (m_token != token) {
        m_token = token;
        Q_EMIT tokenChanged();
    }
}

QString Profile::gameParameters() const
{
    return m_gameParameters;
}

void Profile::setGameParameters(const QString &gameParameters)
{
    if (m_gameParameters != gameParameters) {
        m_gameParameters = gameParameters;
        Q_EMIT gameParametersChanged();
    }
}

GameSettingsType::Type Profile::gameSettings() const
{
    return m_gameSettings;
}

void Profile::setGameSettings(const GameSettingsType::Type type)
{
    if (m_gameSettings != type) {
        m_gameSettings = type;
        Q_EMIT gameSettingsChanged();
    }
}

QString Profile::gameSettingsPath() const
{
    return m_gameSettingsPath;
}

void Profile::setGameSettingsPath(const QString &gameSettingsPath)
{
    if (m_gameSettingsPath != gameSettingsPath) {
        m_gameSettingsPath = gameSettingsPath;
        Q_EMIT gameSettingsPathChanged();
    }
}

QString Profile::gamePath() const
{
    return m_gamePath;
}

void Profile::setGamePath(const QString &gamePath)
{
    if (m_gamePath != gamePath) {
        m_gamePath = gamePath;
        Q_EMIT gamePathChanged();
    }
}

QString Profile::protonPath() const
{
    return m_protonPath;
}

void Profile::setProtonPath(const QString &protonPath)
{
    if (m_protonPath != protonPath) {
        m_protonPath = protonPath;
        Q_EMIT protonPathChanged();
    }
}

QString Profile::environmentVariables() const
{
    return m_environmentVariables;
}

void Profile::setEnvironmentVariables(const QString &environmentVariables)
{
    if (m_environmentVariables != environmentVariables) {
        m_environmentVariables = environmentVariables;
        Q_EMIT environmentVariablesChanged();
    }
}

QString Profile::lootFilter() const
{
    return m_lootFilter;
}

void Profile::setLootFilter(const QString &lootFilter)
{
    if (m_lootFilter != lootFilter) {
        m_lootFilter = lootFilter;
        Q_EMIT lootFilterChanged();
    }
}

bool Profile::rememberWindowPosition() const
{
    return m_rememberWindowPosition;
}

void Profile::setRememberWindowPosition(bool rememberWindowPosition)
{
    if (m_rememberWindowPosition != rememberWindowPosition) {
        m_rememberWindowPosition = rememberWindowPosition;
        Q_EMIT rememberWindowPositionChanged();
    }
}

std::optional<QPoint> Profile::windowPosition() const
{
    return m_windowPosition;
}

void Profile::setWindowPosition(const QPoint &windowPosition)
{
    if (m_windowPosition != windowPosition) {
        m_windowPosition = windowPosition;
        Q_EMIT windowPositionChanged();
    }
}

void Profile::copyFrom(const Profile *other)
{
    if (!other || other == this) {
        return;
    }

    setId(other->id());
    setProfileName(other->profileName());
    setAuthMethod(other->authMethod());
    setRegion(other->region());
    setEmail(other->email());
    setToken(other->token());
    setPassword(other->password());
    setGameParameters(other->gameParameters());
    setGameSettings(other->gameSettings());
    setGameSettingsPath(other->gameSettingsPath());
    setGamePath(other->gamePath());
    setProtonPath(other->protonPath());
    setEnvironmentVariables(other->environmentVariables());
    setLootFilter(other->lootFilter());
    setRememberWindowPosition(other->rememberWindowPosition());
}

QJsonObject Profile::toJson() const
{
    // The status is runtime state and deliberately not persisted.
    QJsonObject json{
        {QStringLiteral("id"), m_id},
        {QStringLiteral("profileName"), m_profileName},
        {QStringLiteral("authMethod"), enumToKey(m_authMethod)},
        {QStringLiteral("region"), enumToKey(m_region)},
        {QStringLiteral("email"), m_email},
        {QStringLiteral("token"), m_token},
        {QStringLiteral("password"), m_password},
        {QStringLiteral("gameParameters"), m_gameParameters},
        {QStringLiteral("gameSettings"), enumToKey(m_gameSettings)},
        {QStringLiteral("gameSettingsPath"), m_gameSettingsPath},
        {QStringLiteral("gamePath"), m_gamePath},
        {QStringLiteral("protonPath"), m_protonPath},
        {QStringLiteral("environmentVariables"), m_environmentVariables},
        {QStringLiteral("lootFilter"), m_lootFilter},
        {QStringLiteral("rememberWindowPosition"), m_rememberWindowPosition},
    };
    if (m_windowPosition) {
        json[QStringLiteral("windowPosition")] = QJsonObject{{QStringLiteral("x"), m_windowPosition->x()}, {QStringLiteral("y"), m_windowPosition->y()}};
    }
    return json;
}

Profile *Profile::fromJson(const QJsonObject &json, QObject *parent)
{
    auto *profile = new Profile(parent);
    profile->setId(json[QStringLiteral("id")].toInt());
    profile->setProfileName(json[QStringLiteral("profileName")].toString());
    profile->setAuthMethod(enumFromKey(json[QStringLiteral("authMethod")], AuthMethodModel::Password));
    profile->setRegion(enumFromKey(json[QStringLiteral("region")], RegionModel::Europe));
    profile->setEmail(json[QStringLiteral("email")].toString());
    profile->setToken(json[QStringLiteral("token")].toString());
    profile->setPassword(json[QStringLiteral("password")].toString());
    profile->setGameParameters(json[QStringLiteral("gameParameters")].toString());
    profile->setGameSettings(enumFromKey(json[QStringLiteral("gameSettings")], GameSettingsType::None));
    profile->setGameSettingsPath(json[QStringLiteral("gameSettingsPath")].toString());
    profile->setGamePath(json[QStringLiteral("gamePath")].toString());
    profile->setProtonPath(json[QStringLiteral("protonPath")].toString());
    profile->setEnvironmentVariables(json[QStringLiteral("environmentVariables")].toString());
    profile->setLootFilter(json[QStringLiteral("lootFilter")].toString());
    profile->setRememberWindowPosition(json[QStringLiteral("rememberWindowPosition")].toBool(true));
    if (const QJsonValue position = json[QStringLiteral("windowPosition")]; position.isObject()) {
        profile->setWindowPosition(QPoint(position[QStringLiteral("x")].toInt(), position[QStringLiteral("y")].toInt()));
    }
    return profile;
}
