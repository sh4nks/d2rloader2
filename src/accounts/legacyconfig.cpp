#include "legacyconfig.h"
#include "../core/logging.h"
#include "d2rloaderconfig.h"
#include "gamesettings.h"
#include "region.h"

#include <KConfigGroup>
#include <KLocalizedString>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QHash>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStandardPaths>

namespace
{
QString legacyDirectory()
{
#if defined(Q_OS_WIN)
    const QString base = qEnvironmentVariable("APPDATA");
#else
    // Inside the Flatpak the config location is the sandbox's own, which the
    // previous D2RLoader never wrote to.
    QString base = qEnvironmentVariable("HOST_XDG_CONFIG_HOME");
    if (base.isEmpty()) {
        base = QFileInfo::exists(QStringLiteral("/.flatpak-info")) ? QDir::homePath() + QStringLiteral("/.config")
                                                                   : QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation);
    }
#endif
    return base.isEmpty() ? QString() : QDir(base).filePath(QStringLiteral("d2rloader"));
}

bool samePath(const QString &a, const QString &b)
{
    const QString left = QDir::cleanPath(QFileInfo(a).absoluteFilePath());
    const QString right = QDir::cleanPath(QFileInfo(b).absoluteFilePath());
#if defined(Q_OS_WIN)
    return left.compare(right, Qt::CaseInsensitive) == 0;
#else
    return left == right;
#endif
}

QJsonDocument readJson(const QString &path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        qCWarning(LOG_PROFILES) << "cannot read" << path << ":" << file.errorString();
        return {};
    }
    QJsonParseError error;
    const QJsonDocument document = QJsonDocument::fromJson(file.readAll(), &error);
    if (error.error != QJsonParseError::NoError) {
        qCWarning(LOG_PROFILES) << "cannot parse" << path << ":" << error.errorString();
    }
    return document;
}

/**
 * Whether the configuration file has no value for the entry \a name, so a
 * value of the previous D2RLoader fills a gap instead of overriding a choice
 * made in this one.
 */
bool isUnset(const char *name)
{
    auto *config = D2RLoaderConfig::self();
    const KConfigSkeletonItem *item = config->findItem(QLatin1String(name));
    return item && !config->config()->group(item->group()).hasKey(item->key());
}

int importSettings(const QJsonObject &old, const QString &defaultAccountsPath)
{
    auto *config = D2RLoaderConfig::self();
    int imported = 0;

    const auto takeString = [&](const char *oldKey, const char *name, auto setter) {
        const QString value = old[QLatin1String(oldKey)].toString();
        if (!value.isEmpty() && isUnset(name)) {
            setter(value);
            ++imported;
        }
    };
    const auto takeBool = [&](const char *oldKey, const char *name, auto setter) {
        const QJsonValue value = old[QLatin1String(oldKey)];
        if (value.isBool() && isUnset(name)) {
            setter(value.toBool());
            ++imported;
        }
    };

    takeString("game_path", "gamePath", &D2RLoaderConfig::setGamePath);
    takeString("wineprefix", "wineprefixPath", &D2RLoaderConfig::setWineprefixPath);
    takeString("protonpath", "protonPath", &D2RLoaderConfig::setProtonPath);
    takeString("token_username", "d2emuUser", &D2RLoaderConfig::setD2emuUser);
    takeString("token", "d2emuToken", &D2RLoaderConfig::setD2emuToken);
    takeBool("d2rinfo", "useD2RInfo", &D2RLoaderConfig::setUseD2RInfo);
    takeBool("rotw", "rotw", &D2RLoaderConfig::setRotw);
    takeBool("log_file", "logToFile", &D2RLoaderConfig::setLogToFile);
    takeBool("check_update", "checkForUpdates", &D2RLoaderConfig::setCheckForUpdates);

    // The accounts file stays where it is only when it was moved away from
    // the previous default on purpose.
    const QString accountsPath = old[QStringLiteral("accounts_path")].toString();
    if (!accountsPath.isEmpty() && !samePath(accountsPath, defaultAccountsPath) && isUnset("profilePath")) {
        config->setProfilePath(accountsPath);
        ++imported;
    }

    // loguru levels, of which the previous D2RLoader offered TRACE to ERROR.
    static const QHash<QString, int> levels{
        {QStringLiteral("TRACE"), D2RLoaderConfig::EnumLogLevel::Debug},
        {QStringLiteral("DEBUG"), D2RLoaderConfig::EnumLogLevel::Debug},
        {QStringLiteral("INFO"), D2RLoaderConfig::EnumLogLevel::Info},
        {QStringLiteral("SUCCESS"), D2RLoaderConfig::EnumLogLevel::Info},
        {QStringLiteral("WARNING"), D2RLoaderConfig::EnumLogLevel::Warn},
        {QStringLiteral("ERROR"), D2RLoaderConfig::EnumLogLevel::Error},
        {QStringLiteral("CRITICAL"), D2RLoaderConfig::EnumLogLevel::Error},
    };
    const QString level = old[QStringLiteral("log_level")].toString().toUpper();
    if (levels.contains(level) && isUnset("logLevel")) {
        config->setLogLevel(levels.value(level));
        ++imported;
    }

    return imported;
}

AuthMethodModel::AuthMethod authMethod(const QString &shortcode)
{
    if (shortcode == QLatin1String("token")) {
        return AuthMethodModel::Token;
    }
    if (shortcode == QLatin1String("steam")) {
        return AuthMethodModel::Steam;
    }
    return AuthMethodModel::Password;
}

RegionModel::Region region(const QString &server)
{
    for (const RegionModel::Region candidate : {RegionModel::Europe, RegionModel::Americas, RegionModel::Asia}) {
        if (RegionModel::server(candidate) == server) {
            return candidate;
        }
    }
    qCWarning(LOG_PROFILES) << "unknown region" << server << "- using Europe";
    return RegionModel::Europe;
}

/**
 * Copies the game settings files of the previous D2RLoader's \a oldFolder to
 * this D2RLoader's folder, leaving files that already exist there alone.
 */
void copyGameSettings(const QString &oldFolder)
{
    const QString newFolder = GameSettings::settingsFolder();
    const QDir source(oldFolder);
    if (!source.exists() || samePath(oldFolder, newFolder)) {
        return;
    }
    if (!QDir().mkpath(newFolder)) {
        qCWarning(LOG_PROFILES) << "cannot create" << newFolder;
        return;
    }
    const QStringList names = source.entryList({QStringLiteral("*.json")}, QDir::Files);
    for (const QString &name : names) {
        const QString target = QDir(newFolder).filePath(name);
        if (QFileInfo::exists(target)) {
            continue;
        }
        if (QFile::copy(source.filePath(name), target)) {
            qCInfo(LOG_PROFILES) << "copied game settings" << name << "to" << newFolder;
        } else {
            qCWarning(LOG_PROFILES) << "cannot copy" << source.filePath(name) << "to" << target;
        }
    }
}

/**
 * The previous D2RLoader stored a plain path to the Settings.json of an
 * account, typically settings.<name>.json in its own game_settings folder,
 * which copyGameSettings() has brought over to this D2RLoader's folder.
 */
void convertGameSettings(Profile *profile, const QString &oldPath, const QString &oldFolder)
{
    if (oldPath.isEmpty()) {
        return;
    }
    QString path = oldPath;
    if (!oldFolder.isEmpty() && samePath(QFileInfo(oldPath).absolutePath(), oldFolder)) {
        path = QDir(GameSettings::settingsFolder()).filePath(QFileInfo(oldPath).fileName());
    }
    if (samePath(path, GameSettings::profileSettingsPath(profile))) {
        profile->setGameSettings(GameSettingsType::Profile);
    } else {
        profile->setGameSettings(GameSettingsType::Custom);
        profile->setGameSettingsPath(path);
    }
}

QString backupName(const QString &path)
{
    QString candidate = path + QStringLiteral(".bak");
    for (int n = 1; QFileInfo::exists(candidate); ++n) {
        candidate = path + QStringLiteral(".bak%1").arg(n);
    }
    return candidate;
}
}

QList<Profile *> LegacyConfig::convertAccounts(const QJsonArray &accounts, const QString &gameSettingsFolder, QObject *parent)
{
    QList<Profile *> profiles;
    for (const QJsonValue &value : accounts) {
        const QJsonObject old = value.toObject();
        auto *profile = new Profile(parent);
        profile->setId(profiles.count() + 1);
        // The previous D2RLoader showed the email of an account without a name.
        const QString email = old[QStringLiteral("email")].toString();
        const QString name = old[QStringLiteral("profile_name")].toString();
        profile->setProfileName(name.isEmpty() ? email : name);
        profile->setEmail(email);
        profile->setAuthMethod(authMethod(old[QStringLiteral("auth_method")].toString()));
        profile->setRegion(region(old[QStringLiteral("region")].toString()));
        profile->setToken(old[QStringLiteral("token")].toString());
        profile->setPassword(old[QStringLiteral("password")].toString());
        profile->setGameParameters(old[QStringLiteral("params")].toString());
        profile->setGamePath(old[QStringLiteral("game_path")].toString());
        profile->setProtonPath(old[QStringLiteral("protonpath")].toString());
        convertGameSettings(profile, old[QStringLiteral("game_settings")].toString(), gameSettingsFolder);
        profiles.append(profile);
    }
    return profiles;
}

LegacyConfig::Result LegacyConfig::migrate(QObject *parent)
{
    Result result;
    auto *config = D2RLoaderConfig::self();
    if (config->legacyConfigImported()) {
        return result;
    }

    // Only the previous D2RLoader ever writes a settings.json there.
    const QString directory = legacyDirectory();
    const QString settingsPath = QDir(directory).filePath(QStringLiteral("settings.json"));
    if (directory.isEmpty() || !QFileInfo::exists(settingsPath)) {
        return result;
    }
    const QJsonDocument settings = readJson(settingsPath);
    if (!settings.isObject()) {
        return result;
    }
    qCInfo(LOG_PROFILES) << "importing the configuration of the previous D2RLoader from" << directory;

    const QString defaultAccountsPath = QDir(directory).filePath(QStringLiteral("accounts.json"));
    const int importedSettings = importSettings(settings.object(), defaultAccountsPath);

    QString oldAccountsPath = settings.object()[QStringLiteral("accounts_path")].toString();
    if (oldAccountsPath.isEmpty()) {
        oldAccountsPath = defaultAccountsPath;
    }
    const QString newAccountsPath = config->profilePath();
    const QString gameSettingsFolder = QDir(directory).filePath(QStringLiteral("game_settings"));

    QString skipped;
    QString backup;
    const QJsonDocument accounts = QFileInfo::exists(oldAccountsPath) ? readJson(oldAccountsPath) : QJsonDocument();
    // An object is this D2RLoader's own format, so there is nothing to convert.
    if (accounts.isArray()) {
        if (QFileInfo::exists(newAccountsPath) && !samePath(oldAccountsPath, newAccountsPath)) {
            skipped = i18nc("@info %1 is a file", "Its accounts were left alone because \"%1\" already exists.", newAccountsPath);
        } else {
            copyGameSettings(gameSettingsFolder);
            result.profiles = convertAccounts(accounts.array(), gameSettingsFolder, parent);
            result.accountsConverted = true;
        }
    }

    // On Linux both D2RLoaders use the same accounts file, which the previous
    // one cannot read once it is rewritten in this one's format.
    if (result.accountsConverted && samePath(oldAccountsPath, newAccountsPath)) {
        backup = backupName(oldAccountsPath);
        if (!QFile::rename(oldAccountsPath, backup)) {
            qCWarning(LOG_PROFILES) << "cannot move" << oldAccountsPath << "to" << backup;
            qDeleteAll(result.profiles);
            result.profiles.clear();
            result.accountsConverted = false;
            // legacyConfigImported stays unset, so the next start tries again.
            config->save();
            result.notice = i18nc("@info %1 is a file", "Could not back up \"%1\", so the accounts of the previous D2RLoader were not imported.", oldAccountsPath);
            return result;
        }
    }

    config->setLegacyConfigImported(true);
    config->save();

    if (result.accountsConverted) {
        qCInfo(LOG_PROFILES) << "imported" << result.profiles.count() << "accounts and" << importedSettings << "settings";
        result.notice = i18ncp("@info %2 is a folder",
                               "Imported one account and the settings of the previous D2RLoader from \"%2\".",
                               "Imported %1 accounts and the settings of the previous D2RLoader from \"%2\".",
                               result.profiles.count(),
                               directory);
        if (!backup.isEmpty()) {
            result.notice += QLatin1Char(' ') + i18nc("@info %1 is a file", "The previous accounts file was kept as \"%1\".", backup);
        }
    } else if (importedSettings > 0 || !skipped.isEmpty()) {
        qCInfo(LOG_PROFILES) << "imported" << importedSettings << "settings";
        result.notice = i18nc("@info %1 is a folder", "Imported the settings of the previous D2RLoader from \"%1\".", directory);
        if (!skipped.isEmpty()) {
            result.notice += QLatin1Char(' ') + skipped;
        }
    }
    return result;
}
