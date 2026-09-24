#pragma once
#include "profile.h"
#include <QAbstractListModel>
#include <QAbstractTableModel>
#include <QJSEngine>
#include <QSet>
#include <QUrl>
#include <QWindow>
#include <qqmlengine.h>
#include <qqmlintegration.h>
#include <qvariant.h>

/**
 * The columns of the accounts table, in display order.
 */
class ProfileColumn : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("")

public:
    enum Column {
        Status = 0,
        Name,
        AuthMethod,
        Region,
        GameParameters,
        Actions,
        Count, /* number of columns, not a column itself */
    };
    Q_ENUM(Column)
};

class ProfileManager : public QAbstractTableModel
{
    Q_OBJECT
    QML_SINGLETON
    QML_ELEMENT

    Q_PROPERTY(bool isReady READ isReady NOTIFY profileReady)
    Q_PROPERTY(bool hasProfiles READ hasProfiles NOTIFY profileChanged)
    Q_PROPERTY(Profile *selectedProfile READ selectedProfile WRITE selectProfile NOTIFY profileSelected)
    Q_PROPERTY(int selectedIndex READ selectedIndex NOTIFY profileSelected)
    Q_PROPERTY(QString loadError READ loadError NOTIFY profileReady)

public:
    enum ProfileRoles {
        StatusRole = Qt::UserRole + 1,
        ProfileNameRole,
        AuthMethodRole,
        RegionRole,
        GameParametersRole,
        ProfileRole,
        ActionRole,
        GameSettingsRole,
        GameSettingsPathRole,
        ProfileIdRole,
        LootFilterRole,
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

    /**
     * Why the accounts file could not be loaded, empty if it was. While this
     * is set, no change to the accounts is written.
     */
    QString loadError() const;
    bool hasProfiles() const;
    void selectProfile(Profile *profile);
    Q_INVOKABLE Profile *selectedProfile() const;
    int selectedIndex() const;
    Q_INVOKABLE Profile *getProfile(int index);
    Q_INVOKABLE void removeProfile(Profile *profile);

    /**
     * Removes the account in \a row. Does nothing if the row is out of range.
     */
    Q_INVOKABLE void removeProfileAt(int row);

    /**
     * Appends a copy of the account in \a row under a free name.
     */
    Q_INVOKABLE void cloneProfile(int row);
    QList<Profile *> profiles() const;

    Q_INVOKABLE int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    Q_INVOKABLE int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    Q_INVOKABLE QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    Q_INVOKABLE bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    Q_INVOKABLE QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    /**
     * Returns a profile that is not part of the model yet, for the editor to
     * fill in. Pass it to commit() to keep it or discard() to throw it away.
     */
    Q_INVOKABLE Profile *createDraft();

    /**
     * Returns a detached copy of the profile in \a row, so the editor can
     * modify it without touching the stored account until commit().
     */
    Q_INVOKABLE Profile *editDraft(int row);

    /**
     * Applies \a draft: appends it when it is new, otherwise copies it over
     * the account it was taken from. Writes the accounts file either way.
     */
    Q_INVOKABLE void commit(Profile *draft);

    /**
     * Throws \a draft away without touching the stored accounts.
     */
    Q_INVOKABLE void discard(Profile *draft);
    Q_INVOKABLE void moveUp(int row);
    Q_INVOKABLE void moveDown(int row);

    /**
     * Writes every account, credentials included, to the local file \a url.
     * Returns an error message, or an empty string on success.
     */
    Q_INVOKABLE QString exportAccounts(const QUrl &url) const;

    /**
     * Loads the accounts of the local file \a url as new accounts, after the
     * existing ones or in their place when \a replace is set. Replacing is
     * refused while a game runs. Returns an error message, or an empty string
     * on success.
     */
    Q_INVOKABLE QString importAccounts(const QUrl &url, bool replace);
    QHash<int, QByteArray> roleNames() const override;

Q_SIGNALS:
    void profileAdded(Profile *profile);
    void profileRemoved(Profile *profile);
    void profileChanged(Profile *profile);
    void profilesChanged();
    void profileReady();
    void profileSelected(Profile *profile);

private:
    explicit ProfileManager(QObject *parent = nullptr);

    ~ProfileManager() override;

    int calculateNextId();
    QString accountsFilePath() const;
    bool readAccounts();
    bool writeAccounts();
    Profile *profileById(int id) const;
    bool isProfileNameTaken(const QString &name) const;
    QString uniqueProfileName(const QString &base) const;

    /**
     * Refreshes the status and action cells of \a profile whenever its status
     * changes, and writes the accounts file whenever its game window moved.
     * The launcher sets both on the profile rather than through setData().
     */
    void watch(Profile *profile);
    Profile *m_selected_profile = nullptr;
    QList<Profile *> m_profiles;
    /* Drafts handed out but neither committed nor discarded yet. Membership is
       what makes commit()/discard() safe to call more than once. */
    QSet<Profile *> m_drafts;
    bool m_ready = false;
    /* Whether the accounts file was read successfully; guards writeAccounts(). */
    bool m_loaded = false;
    QString m_loadError;
    int m_sessionMaxId = 0;
};
