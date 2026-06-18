#include "profilemanager.h"
#include "authmethod.h"
#include "profile.h"
#include "region.h"
#include <qabstractitemmodel.h>
#include <qhashfunctions.h>
#include <qlist.h>
#include <qvariant.h>

using namespace Qt::StringLiterals;

ProfileManager::ProfileManager(QObject *parent)
    : QAbstractTableModel(parent)
{
}

int ProfileManager::rowCount(const QModelIndex &) const
{
    return m_profiles.count();
}

int ProfileManager::columnCount(const QModelIndex &) const
{
    // status, profile name, authmethod, region, params, actions
    return 6;
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

void ProfileManager::addProfile(const QString &name)
{
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

void ProfileManager::loadProfiles()
{
    // TODO: implement loading from json
    QList<Profile *> profileList;
    for (int i = 0; i <= 5; i++) {
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

QHash<int, QByteArray> ProfileManager::roleNames() const
{
    static QHash<int, QByteArray> roles{
        {StatusRole, "status"},
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
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal) {
        if (section == StatusRole) {
            return QStringLiteral("Status");
        }

        if (section == ProfileNameRole) {
            return QStringLiteral("Profile");
        }

        if (section == AuthMethodRole) {
            return QStringLiteral("Auth Method");
        }

        if (section == RegionRole) {
            return QStringLiteral("Region");
        }

        if (section == GameParametersRole) {
            return QStringLiteral("Launch Parameters");
        }

        if (section == ActionRole) {
            return QStringLiteral("Actions");
        }
    }

    return QVariant();
}

QList<Profile *> ProfileManager::profiles() const
{
    return m_profiles;
}
