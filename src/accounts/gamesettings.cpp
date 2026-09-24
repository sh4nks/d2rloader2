#include "gamesettings.h"
#include "../core/logging.h"
#include "profile.h"

#include <KLocalizedString>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QRegularExpression>
#include <QStandardPaths>

namespace
{
const QString SteamAppId = QStringLiteral("2536520");

QString settingsFolder()
{
    return QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation) + QStringLiteral("/d2rloader/game_settings");
}

/**
 * The Proton prefix Steam runs the game in. Steam may keep the game in any
 * of its libraries, which are listed in libraryfolders.vdf.
 */
QString steamPrefix()
{
    const QString home = QDir::homePath();
    const QStringList roots{
        home + QStringLiteral("/.steam/steam"),
        home + QStringLiteral("/.local/share/Steam"),
        home + QStringLiteral("/.var/app/com.valvesoftware.Steam/.local/share/Steam"),
    };
    const QString compatdata = QStringLiteral("/steamapps/compatdata/%1/pfx").arg(SteamAppId);

    for (const QString &root : roots) {
        if (!QFileInfo::exists(root + QStringLiteral("/steamapps"))) {
            continue;
        }

        QStringList libraries{root};
        QFile folders(root + QStringLiteral("/steamapps/libraryfolders.vdf"));
        if (folders.open(QIODevice::ReadOnly)) {
            static const QRegularExpression pathEntry(QStringLiteral(R"re("path"\s+"([^"]+)")re"));
            auto matches = pathEntry.globalMatch(QString::fromUtf8(folders.readAll()));
            while (matches.hasNext()) {
                libraries << matches.next().captured(1);
            }
        }

        for (const QString &library : std::as_const(libraries)) {
            if (QFileInfo::exists(library + compatdata)) {
                return library + compatdata;
            }
        }
        return root + compatdata;
    }
    return {};
}
}

GameSettings::GameSettings(QObject *parent)
    : QObject(parent)
{
    refresh();
}

QString GameSettings::gamePrefix(const Profile *profile)
{
    if (profile->authMethod() == AuthMethodModel::Steam) {
        return steamPrefix();
    }
    return profile->wineprefix();
}

QString GameSettings::savedGamesPath(const Profile *profile)
{
    const QString prefix = gamePrefix(profile);
    if (prefix.isEmpty()) {
        return {};
    }
    return QDir(prefix).filePath(QStringLiteral("drive_c/users/steamuser/Saved Games/Diablo II Resurrected"));
}

QString GameSettings::currentSettingsPath(const Profile *profile)
{
    const QString folder = savedGamesPath(profile);
    if (folder.isEmpty()) {
        return {};
    }
    return QDir(folder).filePath(QStringLiteral("Settings.json"));
}

QString GameSettings::profileSettingsPath(const Profile *profile)
{
    return QDir(settingsFolder()).filePath(QStringLiteral("settings.%1.json").arg(profile->normalizedName()));
}

QUrl GameSettings::profileSettingsFolder() const
{
    QDir().mkpath(settingsFolder());
    return QUrl::fromLocalFile(settingsFolder());
}

QString GameSettings::resolvedPath(const Profile *profile)
{
    switch (profile->gameSettings()) {
    case GameSettingsType::None:
        return {};
    case GameSettingsType::Profile:
        return profileSettingsPath(profile);
    case GameSettingsType::Custom:
        return profile->gameSettingsPath();
    }
    return {};
}

std::expected<void, QString> GameSettings::apply(const Profile *profile)
{
    const QString source = resolvedPath(profile);
    if (source.isEmpty()) {
        return {};
    }
    if (!QFileInfo::exists(source)) {
        return std::unexpected(i18nc("@info", "The game settings \"%1\" do not exist.", source));
    }

    const QString target = currentSettingsPath(profile);
    if (target.isEmpty()) {
        return std::unexpected(i18nc("@info", "Could not find the Steam installation the game runs in."));
    }
    // A custom path may point at the game's own file, which is already in place.
    if (QFileInfo(source).canonicalFilePath() == QFileInfo(target).canonicalFilePath()) {
        return {};
    }
    if (!QDir().mkpath(QFileInfo(target).absolutePath())) {
        return std::unexpected(i18nc("@info", "Could not create \"%1\".", QFileInfo(target).absolutePath()));
    }

    if (QFileInfo::exists(target)) {
        const QString backup = target + QStringLiteral(".bak");
        QFile::remove(backup);
        if (!QFile::rename(target, backup)) {
            return std::unexpected(i18nc("@info", "Could not back up \"%1\".", target));
        }
    }
    if (!QFile::copy(source, target)) {
        return std::unexpected(i18nc("@info", "Could not copy \"%1\" to \"%2\".", source, target));
    }

    qCInfo(LOG_GAME) << "using game settings" << source << "for" << profile->profileName();
    return {};
}

QStringList GameSettings::files() const
{
    return m_files;
}

void GameSettings::refresh()
{
    QStringList files = QDir(settingsFolder()).entryList({QStringLiteral("*.json")}, QDir::Files);
    // The folder also holds lootfilter.<name>.json loot filter assignments.
    files.removeIf([](const QString &file) {
        return file.startsWith(QStringLiteral("lootfilter."));
    });
    files.sort(Qt::CaseInsensitive);
    if (files != m_files) {
        m_files = files;
        Q_EMIT filesChanged();
    }
}

bool GameSettings::uses(Profile *profile, const QString &name) const
{
    if (!profile) {
        return false;
    }
    const QString used = QFileInfo(resolvedPath(profile)).canonicalFilePath();
    return !used.isEmpty() && used == QFileInfo(QDir(settingsFolder()).filePath(name)).canonicalFilePath();
}

QString GameSettings::remove(const QString &name)
{
    const QString path = QDir(settingsFolder()).filePath(name);
    if (!QFile::remove(path)) {
        return i18nc("@info", "Could not remove \"%1\".", path);
    }
    qCInfo(LOG_GAME) << "removed game settings" << path;
    refresh();
    return {};
}

bool GameSettings::profileSettingsExist(Profile *profile) const
{
    return profile && QFileInfo::exists(profileSettingsPath(profile));
}

QString GameSettings::settingsPath(Profile *profile, GameSettingsType::Type type) const
{
    if (!profile) {
        return {};
    }

    switch (type) {
    case GameSettingsType::None:
        return currentSettingsPath(profile);
    case GameSettingsType::Profile:
        return profileSettingsPath(profile);
    case GameSettingsType::Custom:
        return profile->gameSettingsPath();
    }
    return {};
}

QString GameSettings::copyCurrent(Profile *profile, bool overwrite)
{
    if (!profile || profile->profileName().isEmpty()) {
        return i18nc("@info", "Choose an account name first.");
    }

    const QString source = currentSettingsPath(profile);
    if (source.isEmpty()) {
        return i18nc("@info", "Could not find the Steam installation the game runs in.");
    }
    if (!QFileInfo::exists(source)) {
        return i18nc("@info", "The game has no settings for this account yet. Start it once to create \"%1\".", source);
    }

    const QString target = profileSettingsPath(profile);
    if (!QDir().mkpath(QFileInfo(target).absolutePath())) {
        return i18nc("@info", "Could not create \"%1\".", QFileInfo(target).absolutePath());
    }
    if (QFileInfo::exists(target)) {
        if (!overwrite) {
            return i18nc("@info", "\"%1\" already exists.", target);
        }
        QFile::remove(target);
    }
    if (!QFile::copy(source, target)) {
        return i18nc("@info", "Could not copy \"%1\" to \"%2\".", source, target);
    }

    qCInfo(LOG_GAME) << "copied" << source << "to" << target;
    refresh();
    return {};
}
