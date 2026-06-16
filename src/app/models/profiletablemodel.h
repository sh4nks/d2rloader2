#pragma once
#include "profile.h"
#include <QAbstractTableModel>
#include <qqmlintegration.h>

class ProfileTableModel : public QAbstractTableModel
{
    Q_OBJECT
    QML_ELEMENT

public:
    enum ProfileRoles {
        StatusRole = Qt::UserRole + 1,
        NameRole,
        AuthMethodRole,
        RegionRole,
        GameParametersRole,
        ActionRole,
    };
    Q_ENUM(ProfileRoles)

public:
    explicit ProfileTableModel(QObject *parent = nullptr);
    Q_INVOKABLE int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    Q_INVOKABLE int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    Q_INVOKABLE QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    Q_INVOKABLE QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    QHash<int, QByteArray> roleNames() const override;
    void addProfile(const QString &name);
    bool setProfiles(QList<Profile *> profiles);
    bool loadProfiles();

private:
    QList<Profile *> m_profiles;
};
