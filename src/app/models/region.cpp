#include "region.h"
#include <QAbstractListModel>
#include <QMetaEnum>
#include <qhashfunctions.h>

RegionModel::RegionModel(QObject *parent)
    : QAbstractListModel(parent)
{
    m_items = {{Europe, tr("Europe"), QStringLiteral("eu.actual.battle.net")},
               {Americas, tr("Americas"), QStringLiteral("us.actual.battle.net")},
               {Asia, tr("Asia"), QStringLiteral("kr.actual.battle.net")}};
};

int RegionModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return m_items.count();
}

QVariant RegionModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_items.count())
        return QVariant();

    const auto &item = m_items.at(index.row());

    if (role == NameRole)
        return item.displayName;
    else if (role == ValueRole)
        return static_cast<int>(item.value);
    else if (role == ServerRole)
        return item.server;

    return QVariant();
}

QHash<int, QByteArray> RegionModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[NameRole] = "name";
    roles[ValueRole] = "value";
    roles[ServerRole] = "server";
    return roles;
}

// New Invokable lookup engine function to safely find the Enum via its server string
Q_INVOKABLE int RegionModel::serverToEnum(const QString &server) const
{
    for (const auto &item : m_items) {
        if (item.server == server) {
            return static_cast<int>(item.value);
        }
    }
    return static_cast<int>(Europe);
}

// Optional utility function to find index in model via server (useful for ComboBox initialization)
Q_INVOKABLE int RegionModel::indexOfServer(const QString &server) const
{
    for (int i = 0; i < m_items.count(); ++i) {
        if (m_items.at(i).server == server) {
            return i;
        }
    }
    return 0;
}

#include "moc_region.cpp"
