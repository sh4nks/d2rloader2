#include "profiletablemodel.h"
#include "models/authmethod.h"
#include "models/region.h"
#include "profile.h"
#include <qabstractitemmodel.h>
#include <qhashfunctions.h>
#include <qlist.h>
#include <qvariant.h>

ProfileTableModel::ProfileTableModel(QObject *parent)
    : QAbstractTableModel(parent)
{
    loadProfiles();
}

int ProfileTableModel::rowCount(const QModelIndex &) const
{
    return m_profiles.count();
}

int ProfileTableModel::columnCount(const QModelIndex &) const
{
    return 7; // for title, author, year, rating
}

void ProfileTableModel::addProfile(const QString &name)
{
    Profile *p = new Profile(name, AuthMethodModel::AuthMethod::Token, RegionModel::Region::Europe, QStringLiteral("-w"));

    beginInsertRows(QModelIndex(), m_profiles.count(), m_profiles.count());
    m_profiles.append(p);
    endInsertRows();
}

bool ProfileTableModel::setProfiles(QList<Profile *> profiles)
{
    beginResetModel();
    m_profiles.clear();
    m_profiles = profiles;
    endResetModel();
    return true;
}

bool ProfileTableModel::loadProfiles()
{
    QList<Profile *> profileList;
    profileList.append(new Profile(QStringLiteral("Steirer"), AuthMethodModel::AuthMethod::Token, RegionModel::Region::Europe, QStringLiteral("-w")));
    profileList.append(new Profile(QStringLiteral("GoKarliGo"), AuthMethodModel::AuthMethod::Password, RegionModel::Region::Europe, QStringLiteral("-w")));
    profileList.append(
        new Profile(QStringLiteral("BurliBurliBurli"), AuthMethodModel::AuthMethod::Token, RegionModel::Region::Europe, QStringLiteral("-w -ns")));
    profileList.append(new Profile(QStringLiteral("Neanderthaler"), AuthMethodModel::AuthMethod::Token, RegionModel::Region::Europe, QStringLiteral("-w -ns")));
    profileList.append(new Profile(QStringLiteral("Gaudiwipferl"), AuthMethodModel::AuthMethod::Token, RegionModel::Region::Europe, QStringLiteral("-w -ns")));
    setProfiles(profileList);
    addProfile(QStringLiteral("added test"));

    Profile *profile = profileList.at(0);
    profile->setStatus(ProfileState::Type::Running);
    return true;
}

QVariant ProfileTableModel::data(const QModelIndex &index, int role) const
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

QHash<int, QByteArray> ProfileTableModel::roleNames() const
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

QVariant ProfileTableModel::headerData(int section, Qt::Orientation orientation, int role) const
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
