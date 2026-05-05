#include "profiletablemodel.h"
#include "profile.h"

ProfileTableModel::ProfileTableModel(const QList<Profile *> &profiles, QObject *parent)
    : QAbstractTableModel(parent)
    , m_profiles(profiles)
{
}

int ProfileTableModel::rowCount(const QModelIndex & /* parent */) const
{
    return m_profiles.count();
}

int ProfileTableModel::columnCount(const QModelIndex & /* parent */) const
{
    return 4; // for title, author, year, rating
}

QVariant ProfileTableModel::data(const QModelIndex &index, int role) const
{
    if (index.row() < 0 || index.row() >= m_profiles.count())
        return QVariant();

    Profile *profile = m_profiles[index.row()];

    if (role == Qt::DisplayRole) {
        switch (index.column()) {
        case NameRole:
            return profile->name();
        case AuthMethodRole:
            return profile->authMethod();
        case RegionRole:
            return profile->region();
        case LaunchParametersRole:
            return profile->launchParameters();
        }
    }

    return QVariant();
}

QVariant ProfileTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal) {
        if (section == NameRole) {
            return QStringLiteral("Profile");
        }

        if (section == AuthMethodRole) {
            return QStringLiteral("Auth Method");
        }

        if (section == RegionRole) {
            return QStringLiteral("Region");
        }

        if (section == LaunchParametersRole) {
            return QStringLiteral("Launch Parameters");
        }
    }

    return QVariant();
}
