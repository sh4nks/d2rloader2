#pragma once

#include "profile.h"
#include <QAbstractTableModel>

class ProfileTableModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    enum ProfileRoles {
        NameRole = 0,
        AuthMethodRole,
        RegionRole,
        LaunchParametersRole,
    };
    Q_ENUM(ProfileRoles)

    explicit ProfileTableModel(const QList<Profile *> &profiles, QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

private:
    QList<Profile *> m_profiles;
};
