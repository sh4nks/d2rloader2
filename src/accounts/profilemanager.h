#pragma once
#include "profile.h"
#include <QAbstractListModel>
#include <QAbstractTableModel>
#include <QJSEngine>
#include <QWindow>
#include <qqmlengine.h>
#include <qqmlintegration.h>

class ProfileManager : public QAbstractTableModel
{
    Q_OBJECT
    QML_SINGLETON
    QML_ELEMENT

    Q_PROPERTY(bool isReady READ isReady NOTIFY profilesReady)
    Q_PROPERTY(bool hasProfiles READ hasProfiles NOTIFY profilesChanged)
    Q_PROPERTY(Profile *selectedProfile READ selectedProfile WRITE selectProfile NOTIFY profileSelected)
    Q_PROPERTY(QString selectedProfileId READ selectedProfileId NOTIFY profileSelected)
    Q_PROPERTY(int selectedIndex READ selectedIndex NOTIFY profileSelected)

public:
    enum ProfileRoles {
        StatusRole = Qt::UserRole + 1,
        ProfileNameRole,
        AuthMethodRole,
        RegionRole,
        GameParametersRole,
        ActionRole,
    };
    Q_ENUM(ProfileRoles)

public:
    static ProfileManager *create(QQmlEngine *, QJSEngine *)
    {
        auto inst = &instance();
        QJSEngine::setObjectOwnership(inst, QJSEngine::ObjectOwnership::CppOwnership);
        return inst;
    }

    static ProfileManager &instance();

    void loadProfiles();
    bool isReady() const;
    bool hasProfiles() const;
    void selectProfile(Profile *profile);
    Profile *selectedProfile() const;
    QString selectedProfileId() const;
    int selectedIndex() const;
    Q_INVOKABLE void removeProfile(Profile *profile);
    void addProfile(const QString &name);
    bool setProfiles(QList<Profile *> profiles);
    QList<Profile *> profiles() const;

    Q_INVOKABLE int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    Q_INVOKABLE int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    Q_INVOKABLE QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    Q_INVOKABLE QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

Q_SIGNALS:
    void profileAdded(Profile *profile);
    void profileRemoved(Profile *profile);
    void profileChanged();
    void profileReady();
    void profileSelected(Profile *profile);

private:
    explicit ProfileManager(QObject *parent = nullptr);

    ~ProfileManager() override;
    Profile *m_selected_profile = nullptr;
    QList<Profile *> m_profiles;
    bool m_ready = false;
};
