#include "lootfilters.h"
#include "../core/logging.h"
#include "gamesettings.h"
#include "profile.h"

#include <KLocalizedString>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>
#include <QSaveFile>
#include <QStandardPaths>

namespace
{
const QString FilterSuffix = QStringLiteral(".fltr");

QString libraryFolderPath()
{
    return QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation) + QStringLiteral("/d2rloader/loot_filters");
}

QString libraryPath(const QString &name)
{
    return QDir(libraryFolderPath()).filePath(name + FilterSuffix);
}

QStringList filterNames(const QString &folder)
{
    QStringList names;
    const QFileInfoList files = QDir(folder).entryInfoList({QStringLiteral("*.fltr")}, QDir::Files);
    for (const QFileInfo &file : files) {
        names << file.completeBaseName();
    }
    names.sort(Qt::CaseInsensitive);
    return names;
}

QByteArray readFile(const QString &path)
{
    QFile file(path);
    return file.open(QIODevice::ReadOnly) ? file.readAll() : QByteArray();
}

std::expected<void, QString> writeFile(const QString &path, const QByteArray &data)
{
    QSaveFile file(path);
    if (!file.open(QIODevice::WriteOnly) || file.write(data) < 0 || !file.commit()) {
        return std::unexpected(i18nc("@info", "Could not write \"%1\": %2", path, file.errorString()));
    }
    return {};
}
}

LootFilters::LootFilters(QObject *parent)
    : QObject(parent)
{
    refresh();
}

QStringList LootFilters::characters(const Profile *profile)
{
    // The game keeps the controls of each character as <name><id>.ctlo and
    // .keyo, and character names cannot contain digits.
    static const QRegularExpression characterId(QStringLiteral("\\d+$"));

    const QString folder = GameSettings::savedGamesPath(profile);
    if (folder.isEmpty()) {
        return {};
    }

    QStringList names;
    const QFileInfoList files = QDir(folder).entryInfoList({QStringLiteral("*.ctlo"), QStringLiteral("*.keyo")}, QDir::Files);
    for (const QFileInfo &file : files) {
        QString name = file.completeBaseName();
        name.remove(characterId);
        if (!name.isEmpty() && !names.contains(name)) {
            names << name;
        }
    }
    names.sort(Qt::CaseInsensitive);
    return names;
}

std::expected<void, QString> LootFilters::apply(const Profile *profile)
{
    const QString name = profile->lootFilter();
    if (name.isEmpty()) {
        return {};
    }
    if (!QFileInfo::exists(libraryPath(name))) {
        return std::unexpected(i18nc("@info", "The loot filter \"%1\" is not in the library.", name));
    }

    const QString folder = GameSettings::savedGamesPath(profile);
    if (folder.isEmpty()) {
        return std::unexpected(i18nc("@info", "Could not find the Steam installation the game runs in."));
    }
    if (!QDir().mkpath(folder)) {
        return std::unexpected(i18nc("@info", "Could not create \"%1\".", folder));
    }

    // The copy in the game's folder may hold changes made in game, which are
    // kept as .bak rather than lost.
    const QByteArray filter = readFile(libraryPath(name));
    const QString target = QDir(folder).filePath(name + FilterSuffix);
    if (readFile(target) != filter) {
        if (QFileInfo::exists(target)) {
            const QString backup = target + QStringLiteral(".bak");
            QFile::remove(backup);
            if (!QFile::copy(target, backup)) {
                return std::unexpected(i18nc("@info", "Could not back up \"%1\".", target));
            }
        }
        if (const auto written = writeFile(target, filter); !written) {
            return written;
        }
    }

    const QStringList names = characters(profile);
    if (names.isEmpty()) {
        qCInfo(LOG_GAME) << "no characters of" << profile->profileName() << "found to give the loot filter" << name;
        return {};
    }

    const QString assignmentsPath = QDir(folder).filePath(QStringLiteral("lootfilter.json"));
    QJsonObject assignments{{QStringLiteral("Version"), 1}};
    if (QFileInfo::exists(assignmentsPath)) {
        const QJsonDocument document = QJsonDocument::fromJson(readFile(assignmentsPath));
        if (!document.isObject()) {
            return std::unexpected(i18nc("@info", "Could not read the loot filter assignments in \"%1\".", assignmentsPath));
        }
        assignments = document.object();
    }

    QJsonObject characterFilters = assignments[QStringLiteral("Profiles")].toObject();
    for (const QString &character : names) {
        characterFilters[character] = name;
    }
    assignments[QStringLiteral("Profiles")] = characterFilters;
    if (const auto written = writeFile(assignmentsPath, QJsonDocument(assignments).toJson(QJsonDocument::Indented)); !written) {
        return written;
    }

    qCInfo(LOG_GAME) << "using loot filter" << name << "for" << names << "of" << profile->profileName();
    return {};
}

QStringList LootFilters::names() const
{
    return m_names;
}

void LootFilters::refresh()
{
    const QStringList names = filterNames(libraryFolderPath());
    if (names != m_names) {
        m_names = names;
        Q_EMIT namesChanged();
    }
}

QUrl LootFilters::libraryFolder() const
{
    QDir().mkpath(libraryFolderPath());
    return QUrl::fromLocalFile(libraryFolderPath());
}

QStringList LootFilters::characterNames(Profile *profile) const
{
    return profile ? characters(profile) : QStringList();
}

QStringList LootFilters::gameFilters(Profile *profile) const
{
    const QString folder = profile ? GameSettings::savedGamesPath(profile) : QString();
    return folder.isEmpty() ? QStringList() : filterNames(folder);
}

QString LootFilters::importFromGame(Profile *profile, const QString &name, bool overwrite)
{
    const QString folder = profile ? GameSettings::savedGamesPath(profile) : QString();
    if (folder.isEmpty()) {
        return i18nc("@info", "Could not find the Steam installation the game runs in.");
    }
    return store(name, readFile(QDir(folder).filePath(name + FilterSuffix)), overwrite);
}

QString LootFilters::importFile(const QUrl &url, bool overwrite)
{
    const QString path = url.toLocalFile();
    return store(QFileInfo(path).completeBaseName(), readFile(path), overwrite);
}

QString LootFilters::remove(const QString &name)
{
    if (!QFile::remove(libraryPath(name))) {
        return i18nc("@info", "Could not remove \"%1\".", libraryPath(name));
    }
    qCInfo(LOG_GAME) << "removed loot filter" << name << "from the library";
    refresh();
    return {};
}

QString LootFilters::store(const QString &name, QByteArray data, bool overwrite)
{
    QJsonObject filter = QJsonDocument::fromJson(data).object();
    if (!filter[QStringLiteral("rules")].isArray()) {
        return i18nc("@info", "\"%1\" is not a loot filter.", name);
    }
    // Whether the game goes by the file name or the name inside is unknown,
    // so both are kept the same.
    if (filter[QStringLiteral("name")].toString() != name) {
        filter[QStringLiteral("name")] = name;
        data = QJsonDocument(filter).toJson(QJsonDocument::Indented);
    }

    const QString target = libraryPath(name);
    if (QFileInfo::exists(target) && !overwrite) {
        return i18nc("@info", "The library already has a loot filter named \"%1\".", name);
    }
    if (!QDir().mkpath(libraryFolderPath())) {
        return i18nc("@info", "Could not create \"%1\".", libraryFolderPath());
    }
    if (const auto written = writeFile(target, data); !written) {
        return written.error();
    }

    qCInfo(LOG_GAME) << "stored loot filter" << name << "in" << target;
    refresh();
    return {};
}
