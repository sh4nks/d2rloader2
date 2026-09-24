#include "profilemanager.h"
#include "../core/logging.h"
#include "authmethod.h"
#include "d2rloaderconfig.h"
#include "profile.h"
#include "region.h"
#include <KLocalizedString>
#include <QDir>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSaveFile>

namespace
{
/**
 * Schema version of the accounts file. Bump it when the on-disk shape changes
 * in a way older builds cannot read.
 */
constexpr int AccountsFileVersion = 1;

/**
 * Reads the accounts array of the accounts file at \a path into \a accounts.
 * Returns an error message, or an empty string on success.
 */
QString readAccountsFile(const QString &path, QJsonArray &accounts)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        return file.errorString();
    }

    QJsonParseError error;
    const QJsonDocument document = QJsonDocument::fromJson(file.readAll(), &error);
    if (error.error != QJsonParseError::NoError) {
        return i18nc("@info %1 is the parser error", "The file is not valid JSON: %1", error.errorString());
    }

    const QJsonObject root = document.object();
    if (root[QStringLiteral("version")].toInt() > AccountsFileVersion) {
        return i18nc("@info", "The file was written by a newer version of D2RLoader.");
    }

    accounts = root[QStringLiteral("accounts")].toArray();
    return QString();
}

/**
 * Writes \a profiles as an accounts file to \a path.
 * Returns an error message, or an empty string on success.
 */
QString writeAccountsFile(const QString &path, const QList<Profile *> &profiles)
{
    QJsonArray accounts;
    for (const Profile *profile : profiles) {
        accounts.append(profile->toJson());
    }

    QJsonObject root;
    root[QStringLiteral("version")] = AccountsFileVersion;
    root[QStringLiteral("accounts")] = accounts;

    // QSaveFile so a crash mid-write cannot leave a truncated accounts file.
    QSaveFile file(path);
    if (!file.open(QIODevice::WriteOnly)) {
        return file.errorString();
    }
    file.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
    if (!file.commit()) {
        return file.errorString();
    }

    // The file holds credentials in plain text, so keep it owner-only.
    QFile::setPermissions(path, QFileDevice::ReadOwner | QFileDevice::WriteOwner);
    return QString();
}
}
#include <qabstractitemmodel.h>
#include <qhashfunctions.h>
#include <qlist.h>
#include <qtmetamacros.h>
#include <qvariant.h>

using namespace Qt::StringLiterals;

ProfileManager::ProfileManager(QObject *parent)
    : QAbstractTableModel(parent)
{
}

ProfileManager &ProfileManager::instance()
{
    static ProfileManager profileManager;
    return profileManager;
}

ProfileManager::~ProfileManager() = default;

int ProfileManager::rowCount(const QModelIndex &) const
{
    return m_profiles.count();
}

int ProfileManager::columnCount(const QModelIndex &) const
{
    return ProfileColumn::Count;
}

bool ProfileManager::hasProfiles() const
{
    return !m_profiles.empty();
}

void ProfileManager::selectProfile(Profile *profile)
{
    m_selected_profile = profile;
    Q_EMIT profileSelected(profile);
}

Profile *ProfileManager::selectedProfile() const
{
    return m_selected_profile;
}

int ProfileManager::selectedIndex() const
{
    return m_profiles.indexOf(m_selected_profile);
}

Profile *ProfileManager::getProfile(int index)
{
    if (index < 0 || index >= m_profiles.size()) {
        return nullptr;
    }
    return m_profiles.at(index);
}

void ProfileManager::removeProfile(Profile *profile)
{
    const auto index = m_profiles.indexOf(profile);
    if (index < 0) {
        return;
    }

    beginRemoveRows(QModelIndex(), index, index);
    m_profiles.removeOne(profile);
    endRemoveRows();

    if (hasProfiles()) {
        m_selected_profile = m_profiles.first();
    } else {
        m_selected_profile = nullptr;
    }
    Q_EMIT profileSelected(m_selected_profile);

    writeAccounts();

    Q_EMIT profileRemoved(profile);
    Q_EMIT profilesChanged();
    profile->deleteLater();
}

void ProfileManager::removeProfileAt(int row)
{
    if (row < 0 || row >= m_profiles.size()) {
        return;
    }
    removeProfile(m_profiles.at(row));
}

void ProfileManager::cloneProfile(int row)
{
    if (row < 0 || row >= m_profiles.size()) {
        return;
    }

    auto *clone = createDraft();
    clone->copyFrom(m_profiles.at(row));
    // A clone is a new account: commit() assigns it a fresh id and appends it.
    clone->setId(0);
    clone->setProfileName(uniqueProfileName(m_profiles.at(row)->profileName()));
    commit(clone);
}

bool ProfileManager::isProfileNameTaken(const QString &name) const
{
    for (const Profile *profile : m_profiles) {
        if (QString::compare(profile->profileName(), name, Qt::CaseInsensitive) == 0) {
            return true;
        }
    }
    return false;
}

QString ProfileManager::uniqueProfileName(const QString &base) const
{
    QString candidate = i18nc("@item:intable copy of an account, %1 is the original name", "%1 (copy)", base);
    for (int suffix = 2; isProfileNameTaken(candidate); ++suffix) {
        candidate = i18nc("@item:intable numbered copy of an account, %1 is the original name", "%1 (copy %2)", base, suffix);
    }
    return candidate;
}

void ProfileManager::moveUp(int row)
{
    if (row <= 0 || row >= m_profiles.count())
        return;

    if (beginMoveRows(QModelIndex(), row, row, QModelIndex(), row - 1)) {
        m_profiles.move(row, row - 1);
        endMoveRows();
        writeAccounts();
    }
}

void ProfileManager::moveDown(int row)
{
    if (row < 0 || row >= m_profiles.count() - 1)
        return;

    // Qt row movement logic: destination must be row + 2 to move below target row
    if (beginMoveRows(QModelIndex(), row, row, QModelIndex(), row + 2)) {
        m_profiles.move(row, row + 1);
        endMoveRows();
        writeAccounts();
    }
}

bool ProfileManager::isReady() const
{
    return m_ready;
}

QString ProfileManager::loadError() const
{
    return m_loadError;
}

void ProfileManager::loadProfiles()
{
    readAccounts();
    m_ready = true;
    Q_EMIT profileReady();
    Q_EMIT profilesChanged();
}

QVariant ProfileManager::data(const QModelIndex &index, int role) const
{
    if (index.row() < 0 || index.row() >= m_profiles.count())
        return QVariant();

    Profile *profile = m_profiles[index.row()];

    switch (role) {
    case ProfileRoles::StatusRole:
        return profile->status();
    case ProfileRoles::ProfileRole:
        return QVariant::fromValue(m_profiles[index.row()]);
    case ProfileRoles::ProfileNameRole:
        return profile->profileName();
    case ProfileRoles::AuthMethodRole:
        return profile->authMethod();
    case ProfileRoles::RegionRole:
        return profile->region();
    case ProfileRoles::GameParametersRole:
        return profile->gameParameters();
    case ProfileRoles::ActionRole:
        return profile->status();
    case ProfileRoles::GameSettingsRole:
        return profile->gameSettings();
    case ProfileRoles::GameSettingsPathRole:
        return profile->gameSettingsPath();
    case ProfileRoles::ProfileIdRole:
        return profile->id();
    case ProfileRoles::LootFilterRole:
        return profile->lootFilter();
    }

    return QVariant();
}

bool ProfileManager::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_profiles.size()) {
        return false;
    }

    int row = index.row();
    bool changed = false;

    switch (role) {
    case ProfileRoles::StatusRole:
        m_profiles[row]->setStatus(static_cast<ProfileState::Type>(value.toInt()));
        changed = true;
        break;
    case ProfileRoles::ProfileNameRole:
        m_profiles[row]->setProfileName(value.toString());
        changed = true;
        break;
    case ProfileRoles::AuthMethodRole:
        m_profiles[row]->setAuthMethod(static_cast<AuthMethodModel::AuthMethod>(value.toInt()));
        qCDebug(LOG_PROFILES) << "setData:" << m_profiles[row]->authMethod();
        changed = true;
        break;
    case ProfileRoles::RegionRole:
        m_profiles[row]->setRegion(static_cast<RegionModel::Region>(value.toInt()));
        qCDebug(LOG_PROFILES) << "setData:" << m_profiles[row]->region();
        changed = true;
        break;
    case ProfileRoles::GameParametersRole:
        m_profiles[row]->setGameParameters(value.toString());
        changed = true;
        break;
    case ProfileRoles::GameSettingsRole:
        m_profiles[row]->setGameSettings(static_cast<GameSettingsType::Type>(value.toInt()));
        changed = true;
        break;
    case ProfileRoles::GameSettingsPathRole:
        m_profiles[row]->setGameSettingsPath(value.toString());
        changed = true;
        break;
    case ProfileRoles::LootFilterRole:
        m_profiles[row]->setLootFilter(value.toString());
        changed = true;
        break;
    }

    if (changed) {
        // Crucial: Tell the view which specific roles changed
        Q_EMIT dataChanged(index, index, {Qt::DisplayRole, role});
        // Edits made inline in the table persist like any other change.
        writeAccounts();
    }
    return changed;
}

QHash<int, QByteArray> ProfileManager::roleNames() const
{
    static QHash<int, QByteArray> roles{
        // HorizontalHeaderView looks up its labels through the "display" role.
        {Qt::DisplayRole, "display"},
        {StatusRole, "status"},
        {ProfileRole, "profile"},
        {ProfileNameRole, "profileName"},
        {AuthMethodRole, "authMethod"},
        {RegionRole, "region"},
        {GameParametersRole, "gameParameters"},
        {ActionRole, "actions"},
        {GameSettingsRole, "gameSettings"},
        {GameSettingsPathRole, "gameSettingsPath"},
        {ProfileIdRole, "profileId"},
        {LootFilterRole, "lootFilter"},
    };
    return roles;
}

QVariant ProfileManager::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole || orientation != Qt::Horizontal) {
        return QVariant();
    }

    switch (section) {
    case ProfileColumn::Status:
        // The status indicator needs no label.
        return QString();
    case ProfileColumn::Name:
        return i18nc("@title:column", "Account");
    case ProfileColumn::AuthMethod:
        return i18nc("@title:column", "Auth Method");
    case ProfileColumn::Region:
        return i18nc("@title:column", "Region");
    case ProfileColumn::GameParameters:
        return i18nc("@title:column", "Parameters");
    case ProfileColumn::Actions:
        return i18nc("@title:column", "Actions");
    }

    return QVariant();
}

QList<Profile *> ProfileManager::profiles() const
{
    return m_profiles;
}

Profile *ProfileManager::createDraft()
{
    // Drafts are parented to the manager so QML never owns them, but they stay
    // out of m_profiles until commit().
    auto *draft = new Profile(this);
    m_drafts.insert(draft);
    return draft;
}

Profile *ProfileManager::editDraft(int row)
{
    if (row < 0 || row >= m_profiles.size()) {
        return createDraft();
    }

    auto *draft = createDraft();
    draft->copyFrom(m_profiles.at(row));
    return draft;
}

void ProfileManager::commit(Profile *draft)
{
    // Only an outstanding draft can be committed, which also makes committing
    // the same draft twice a no-op.
    if (!draft || !m_drafts.remove(draft)) {
        return;
    }

    Profile *existing = profileById(draft->id());
    if (existing) {
        existing->copyFrom(draft);
        const int row = m_profiles.indexOf(existing);
        const QModelIndex changed = index(row, 0);
        Q_EMIT dataChanged(changed, index(row, columnCount() - 1));
        Q_EMIT profileChanged(existing);
    } else {
        draft->setId(calculateNextId());
        beginInsertRows(QModelIndex(), m_profiles.count(), m_profiles.count());
        m_profiles.append(draft);
        endInsertRows();
        watch(draft);
        Q_EMIT profileAdded(draft);
        Q_EMIT profilesChanged();
    }

    writeAccounts();

    if (existing) {
        draft->deleteLater();
    }
}

void ProfileManager::discard(Profile *draft)
{
    // Ignore anything that is not an outstanding draft: already committed,
    // already discarded, or a profile that belongs to the model.
    if (!draft || !m_drafts.remove(draft)) {
        return;
    }
    draft->deleteLater();
}

Profile *ProfileManager::profileById(int id) const
{
    if (id <= 0) {
        return nullptr;
    }
    for (Profile *profile : m_profiles) {
        if (profile->id() == id) {
            return profile;
        }
    }
    return nullptr;
}

void ProfileManager::watch(Profile *profile)
{
    connect(profile, &Profile::statusChanged, this, [this, profile] {
        const int row = m_profiles.indexOf(profile);
        if (row >= 0) {
            Q_EMIT dataChanged(index(row, ProfileColumn::Status), index(row, ProfileColumn::Actions), {StatusRole, ActionRole});
        }
    });
    connect(profile, &Profile::windowPositionChanged, this, &ProfileManager::writeAccounts);
}

int ProfileManager::calculateNextId()
{
    m_sessionMaxId++;
    return m_sessionMaxId;
}

QString ProfileManager::accountsFilePath() const
{
    return D2RLoaderConfig::self()->profilePath();
}

bool ProfileManager::readAccounts()
{
    const QString path = accountsFilePath();

    qDeleteAll(m_profiles);
    beginResetModel();
    m_profiles.clear();

    if (!QFile::exists(path)) {
        // First run: no accounts yet is not an error.
        endResetModel();
        m_sessionMaxId = 0;
        m_loaded = true;
        return true;
    }

    QJsonArray accounts;
    const QString error = readAccountsFile(path, accounts);
    if (!error.isEmpty()) {
        qCWarning(LOG_PROFILES) << "cannot load" << path << ":" << error;
        m_loadError = i18nc("@info %1 is the file, %2 the reason",
                            "Could not load the accounts from \"%1\": %2\nChanges to the accounts will not be saved.",
                            path,
                            error);
        endResetModel();
        return false;
    }

    for (const QJsonValue &value : accounts) {
        auto *profile = Profile::fromJson(value.toObject(), this);
        m_profiles.append(profile);
        watch(profile);
    }

    endResetModel();

    // Seed the id counter from what was actually loaded, so ids stay unique
    // across restarts.
    m_sessionMaxId = 0;
    for (const Profile *profile : m_profiles) {
        m_sessionMaxId = std::max(m_sessionMaxId, profile->id());
    }

    m_loaded = true;
    return true;
}

bool ProfileManager::writeAccounts()
{
    const QString path = accountsFilePath();

    // The file exists but could not be read - it is either corrupt or was
    // written by a newer version. Overwriting it would destroy accounts we
    // simply failed to understand.
    if (!m_loaded) {
        qCWarning(LOG_PROFILES) << "refusing to overwrite" << path << "because it could not be loaded";
        return false;
    }

    const QDir directory = QFileInfo(path).absoluteDir();
    if (!directory.exists() && !directory.mkpath(QStringLiteral("."))) {
        qCWarning(LOG_PROFILES) << "cannot create" << directory.absolutePath();
        return false;
    }

    const QString error = writeAccountsFile(path, m_profiles);
    if (!error.isEmpty()) {
        qCWarning(LOG_PROFILES) << "cannot write" << path << ":" << error;
        return false;
    }
    return true;
}

QString ProfileManager::exportAccounts(const QUrl &url) const
{
    return writeAccountsFile(url.toLocalFile(), m_profiles);
}

QString ProfileManager::importAccounts(const QUrl &url, bool replace)
{
    if (!m_loaded) {
        return i18nc("@info", "The current accounts file could not be loaded, so no accounts can be added to it.");
    }

    // GameManager tracks a running game through its account, which replacing
    // would delete.
    if (replace) {
        for (const Profile *profile : std::as_const(m_profiles)) {
            if (profile->status() == ProfileState::Running || profile->status() == ProfileState::Starting) {
                return i18nc("@info", "Stop all running games before replacing the accounts.");
            }
        }
    }

    QJsonArray accounts;
    const QString error = readAccountsFile(url.toLocalFile(), accounts);
    if (!error.isEmpty()) {
        return error;
    }
    if (accounts.isEmpty()) {
        return i18nc("@info", "The file contains no accounts.");
    }

    // Fresh ids, so no launch sequence picks up an imported account through
    // an id that belonged to another one.
    QList<Profile *> imported;
    for (const QJsonValue &value : accounts) {
        auto *profile = Profile::fromJson(value.toObject(), this);
        profile->setId(calculateNextId());
        imported.append(profile);
    }

    QList<Profile *> removed;
    if (replace) {
        beginResetModel();
        removed = std::exchange(m_profiles, imported);
        endResetModel();
        m_selected_profile = m_profiles.first();
        Q_EMIT profileSelected(m_selected_profile);
    } else {
        beginInsertRows(QModelIndex(), m_profiles.count(), m_profiles.count() + imported.count() - 1);
        for (Profile *profile : imported) {
            // The wineprefix and the settings copy follow the name.
            if (isProfileNameTaken(profile->profileName())) {
                profile->setProfileName(uniqueProfileName(profile->profileName()));
            }
            m_profiles.append(profile);
        }
        endInsertRows();
    }

    for (Profile *profile : imported) {
        watch(profile);
        Q_EMIT profileAdded(profile);
    }
    for (Profile *profile : removed) {
        Q_EMIT profileRemoved(profile);
        profile->deleteLater();
    }
    Q_EMIT profilesChanged();

    if (!writeAccounts()) {
        return i18nc("@info", "The accounts file could not be written, so they will be gone after a restart.");
    }
    return QString();
}
