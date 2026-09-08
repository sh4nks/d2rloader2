#include "profilemanager.h"
#include "authmethod.h"
#include "profile.h"
#include "region.h"
#include <KConfigGroup>
#include <KLocalizedString>
#include <KSharedConfig>
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

void ProfileManager::selectProfileByIndex(int index)
{
    m_selected_profile = m_profiles.at(index);
    Q_EMIT profileSelected(m_selected_profile);
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
    return m_profiles.at(index);
}

void ProfileManager::addProfile(const QString &name)
{
    /*
     *
     int nextRowIndex = m_data.count();

     beginInsertRows(QModelIndex(), nextRowIndex, nextRowIndex);

     TableItem newItem;
     newItem.uniqueId = calculateNextId();
     newItem.name = name;
     newItem.value = value;
     m_data.append(newItem);

     endInsertRows();
     saveOrder();
     *
     *
     */

    Profile *p = new Profile(nullptr);
    p->setProfileName(name);

    beginInsertRows(QModelIndex(), m_profiles.count(), m_profiles.count());
    m_profiles.append(p);
    endInsertRows();
}

bool ProfileManager::setProfiles(QList<Profile *> profiles)
{
    beginResetModel();
    m_profiles.clear();
    m_profiles = profiles;
    endResetModel();
    return true;
}

void ProfileManager::removeProfile(Profile *profile)
{
    // remove from settings
    // auto config = KSharedConfig::openStateConfig();
    // config->deleteGroup(account->settingsGroupName());
    // config->sync();

    /*
     *


     if (row < 0 || row >= m_data.count()) return;

     beginRemoveRows(QModelIndex(), row, row);
     m_data.removeAt(row);
     endRemoveRows();

     saveOrder();


     *
     */

    const auto index = m_profiles.indexOf(profile);
    beginRemoveRows(QModelIndex(), index, index);
    m_profiles.removeOne(profile);
    endRemoveRows();

    if (hasProfiles()) {
        m_selected_profile = m_profiles.first();
    } else {
        m_selected_profile = nullptr;
    }
    Q_EMIT profileSelected(m_selected_profile);

    Q_EMIT profileRemoved(profile);
    Q_EMIT profilesChanged();
}

void ProfileManager::moveUp(int row)
{
    if (row <= 0 || row >= m_profiles.count())
        return;

    if (beginMoveRows(QModelIndex(), row, row, QModelIndex(), row - 1)) {
        m_profiles.move(row, row - 1);
        endMoveRows();
        saveOrder();
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
        saveOrder();
    }
}

bool ProfileManager::isReady() const
{
    return m_ready;
}

void ProfileManager::loadProfiles()
{
    // TODO: implement loading from json
    QList<Profile *> profileList;
    for (int i = 0; i <= 3; i++) {
        profileList.append(Profile::create(ProfileState::Type::Running,
                                           u"Test Profile %1"_s.arg(i),
                                           AuthMethodModel::Token,
                                           RegionModel::Europe,
                                           u"test%1@example.org"_s.arg(i),
                                           "token"_L1,
                                           "tokenProtected"_L1,
                                           "secretsauce"_L1,
                                           "-w"_L1,
                                           GameSettingsType::Type::Custom,
                                           "/dev/null/d2rconfig.json"_L1,
                                           ""_L1,
                                           ""_L1));
    }

    setProfiles(profileList);
    addProfile(QStringLiteral("added test"));

    Profile *profile = profileList.at(0);
    profile->setStatus(ProfileState::Type::Stopped);
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
        qDebug() << "(ProfileManager::setData) " << m_profiles[row]->authMethod();
        changed = true;
        break;
    case ProfileRoles::RegionRole:
        m_profiles[row]->setRegion(static_cast<RegionModel::Region>(value.toInt()));
        qDebug() << "(ProfileManager::setData) " << m_profiles[row]->region();
        changed = true;
        break;
    case ProfileRoles::GameParametersRole:
        m_profiles[row]->setGameParameters(value.toString());
        changed = true;
        break;
    }

    if (changed) {
        // Crucial: Tell the view which specific roles changed
        Q_EMIT dataChanged(index, index, {Qt::DisplayRole, role});
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
        return i18nc("@title:column", "Launch Parameters");
    case ProfileColumn::Actions:
        return i18nc("@title:column", "Actions");
    }

    return QVariant();
}

QList<Profile *> ProfileManager::profiles() const
{
    return m_profiles;
}

int ProfileManager::getIndexOfProfile(Profile *profile)
{
    for (int i = 0; i < m_profiles.size(); i++) {
        auto p = m_profiles[i];
        if (QString::compare(p->profileName(), profile->profileName(), Qt::CaseInsensitive) == 0) {
            return i;
        }
    }
    return -1;
}

void ProfileManager::save(Profile *profile)
{
    int idx = getIndexOfProfile(profile);

    // The top-left boundary of the change (Column 0)
    QModelIndex topLeft = index(idx, 0);

    // The bottom-right boundary of the change (Last Column)
    QModelIndex bottomRight = index(idx, columnCount() - 1);

    qDebug() << "(ProfileManager::save) " << profile->authMethod();
    qDebug() << "(ProfileManager::save) " << profile->region();

    // Emit the signal to refresh the entire row range
    Q_EMIT dataChanged(topLeft, bottomRight);
    Q_EMIT profileChanged(profile);
}

int ProfileManager::calculateNextId()
{
    m_sessionMaxId++;
    return m_sessionMaxId;
}

void ProfileManager::saveOrder()
{
    QList<int> orderedIds;
    for (const auto &item : m_profiles) {
        orderedIds.append(item->id());
    }

    KConfigGroup config = KSharedConfig::openConfig()->group("AccountTableSettings"_L1);
    config.writeEntry("RowOrder", orderedIds);
    config.sync();
}

void ProfileManager::loadAndRestoreOrder()
{
    // 1. Clean up old profiles from memory before loading new ones
    qDeleteAll(m_profiles);
    m_profiles.clear();

    // Replace this block with your actual database/file loading logic
    QList<Profile *> rawItems;

    // Assign your loaded items to the manager container
    m_profiles = rawItems;

    // Example using your Profile::create factory:
    Profile *p1 = Profile::create(ProfileState::Stopped,
                                  "Alpha"_L1,
                                  AuthMethodModel::AuthMethod(),
                                  RegionModel::Region(),
                                  "alpha@email.com"_L1,
                                  ""_L1,
                                  ""_L1,
                                  ""_L1,
                                  ""_L1,
                                  GameSettingsType::None,
                                  ""_L1,
                                  ""_L1,
                                  ""_L1);
    p1->setId(101);
    p1->setParent(this); // Memory management: this manager owns the profile lifespan
    rawItems.append(p1);

    Profile *p2 = Profile::create(ProfileState::Stopped,
                                  "Beta"_L1,
                                  AuthMethodModel::AuthMethod(),
                                  RegionModel::Region(),
                                  "beta@email.com"_L1,
                                  ""_L1,
                                  ""_L1,
                                  ""_L1,
                                  ""_L1,
                                  GameSettingsType::None,
                                  ""_L1,
                                  ""_L1,
                                  ""_L1);
    p2->setId(102);
    p2->setParent(this);
    rawItems.append(p2);

    // 3. Read saved ordering configuration
    KConfigGroup config = KSharedConfig::openConfig()->group("AccountTableSettings"_L1);
    QList<int> savedOrder = config.readEntry("RowOrder", QList<int>());

    m_profiles = rawItems;

    // 4. Sort the pointers safely based on your saved configuration
    if (!savedOrder.isEmpty()) {
        std::sort(m_profiles.begin(), m_profiles.end(), [&savedOrder](const Profile *a, const Profile *b) {
            int indexA = savedOrder.indexOf(a->id());
            int indexB = savedOrder.indexOf(b->id());

            // If an ID isn't found in the saved order, push it to the end
            if (indexA == -1)
                return false;
            if (indexB == -1)
                return true;

            return indexA < indexB;
        });
    }

    // 5. Track session maximum ID limits safely
    for (const auto *item : m_profiles) {
        if (item->id() > m_sessionMaxId) {
            m_sessionMaxId = item->id();
        }
    }
    if (m_sessionMaxId == 0) {
        m_sessionMaxId = 100;
    }

    // 6. Notify QML that the underlying data layout has changed
    // (Call your specific change signal or model reset methods here)
    // emit profilesChanged();
}

/*
     void ProfileManager::save(QString file_path)
    {
        QString full_file_path = file_path.contains(QString::fromUtf8(".json")) ? file_path : file_path.append(QString::fromUtf8(".json"));
        qDebug() << "(ProfileManager) Requested save list as " << full_file_path;

        // Get the current list as JSON
        QJsonDocument doc = toJson();

        // Save JSON
        QFile jsonFile(full_file_path);
        jsonFile.open(QFile::WriteOnly);
        jsonFile.write(doc.toJson());
        jsonFile.close();
    }

    bool ProfileManager::load(QString file_path)
    {
        if (!file_path.contains(QString::fromUtf8(".json"))) {
            qCritical() << "(ProfileManager) Error, the file should be a JSON file";
            return false;
        }

        qDebug() << "(ProfileManager) Requested load the list " << file_path;

        // Load JSON file
        QFile jsonFile(file_path);
        jsonFile.open(QFile::ReadOnly);
        QJsonDocument doc = QJsonDocument().fromJson(jsonFile.readAll());

        // Parse it to the app list
        loadFromJson(doc);

        return true;
    }

    QJsonDocument ProfileManager::toJson()
    {
        QJsonDocument doc;
        // QJsonArray objs_array;
        // for(const auto item : _item_list)
        // {
        //     QJsonObject json_obj;
        //     json_obj.insert("id", item->id());
        //     json_obj.insert("name", item->name());
        //     json_obj.insert("score", item->score());
        //     json_obj.insert("checked", item->checked());
        //     json_obj.insert("filepath", item->filepath());

        //     objs_array.push_back(json_obj);
        // }

        // QJsonDocument doc(objs_array);
        // //qDebug() << doc.toJson();
        return doc;
    }

    void ProfileManager::loadFromJson(QJsonDocument doc)
    {
        // qDebug() << doc.toJson();
        QJsonArray objs_array = doc.array();
        qDebug() << "Loading " << objs_array.size() << " elements";

        for (const auto value : objs_array) {
            QJsonObject obj = value.toObject();
            auto profile = Profile::fromJson(obj);
            // add(obj["id"].toInt(), obj["name"].toString(), obj["checked"].toBool(), obj["score"].toDouble(), obj["filepath"].toString());
        }
    }
 */
