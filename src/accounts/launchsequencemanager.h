#pragma once

#include "profile.h"

#include <QAbstractListModel>
#include <QJSEngine>
#include <qqmlengine.h>
#include <qqmlintegration.h>

/**
 * The launch sequences: named sets of accounts that are started one after
 * another. Accounts are referenced by id and start in the order of the
 * accounts table.
 */
class LaunchSequenceManager : public QAbstractListModel
{
    Q_OBJECT
    QML_SINGLETON
    QML_ELEMENT

    /**
     * The sequence selected in the accounts header, or -1 when there is none.
     */
    Q_PROPERTY(int currentIndex READ currentIndex WRITE setCurrentIndex NOTIFY currentIndexChanged)

    /**
     * Why the sequences file could not be loaded, empty if it was. While this
     * is set, no change to the sequences is written.
     */
    Q_PROPERTY(QString loadError READ loadError CONSTANT)

public:
    enum Roles {
        SequenceIdRole = Qt::UserRole + 1,
        NameRole,
        ProfileIdsRole,
        AccountCountRole,
    };
    Q_ENUM(Roles)

    static LaunchSequenceManager *create(QQmlEngine *, QJSEngine *)
    {
        auto inst = &instance();
        QJSEngine::setObjectOwnership(inst, QJSEngine::ObjectOwnership::CppOwnership);
        return inst;
    }

    static LaunchSequenceManager &instance();

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    int currentIndex() const;
    void setCurrentIndex(int row);
    QString loadError() const;

    /**
     * Stores \a name and \a profileIds in the sequence in \a row, or appends a
     * new sequence when \a row is out of range.
     */
    Q_INVOKABLE void save(int row, const QString &name, const QList<int> &profileIds);
    Q_INVOKABLE void remove(int row);
    Q_INVOKABLE void moveUp(int row);
    Q_INVOKABLE void moveDown(int row);

    QString name(int row) const;

    /**
     * The accounts of the sequence in \a row, in the order of the accounts
     * table.
     */
    QList<Profile *> profiles(int row) const;

Q_SIGNALS:
    void currentIndexChanged();

private:
    struct Sequence {
        int id = 0;
        QString name;
        QList<int> profileIds;
    };

    explicit LaunchSequenceManager(QObject *parent = nullptr);

    QString filePath() const;

    /**
     * Reads the sequences file. Returns an error message, or an empty string
     * on success.
     */
    QString read();
    bool write();

    /**
     * Drops the removed \a profile from every sequence, so a later account
     * that is given the same id does not join them.
     */
    void forgetProfile(Profile *profile);

    QList<Sequence> m_sequences;
    /* Whether the sequences file was read successfully; guards write(). */
    bool m_loaded = false;
    QString m_loadError;
};
