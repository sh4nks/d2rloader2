#pragma once

#include <QAbstractListModel>
#include <QDateTime>
#include <QJsonObject>
#include <QStringList>
#include <qqmlintegration.h>

class QNetworkAccessManager;
class QNetworkReply;
class QTimer;

/**
 * The Diablo Clone progress of one realm, split into its ladder and
 * non-ladder variant the way d2emu.com reports it.
 */
class DCloneModel : public QAbstractListModel
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Owned by GameInfo.")

public:
    enum Roles {
        RegionRole = Qt::UserRole + 1,
        ProgressRole,
    };
    Q_ENUM(Roles)

    /**
     * Highest progress d2emu.com reports, used to render the values as "n/6".
     */
    static constexpr int MaxProgress = 6;

    explicit DCloneModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    /**
     * A realm whose progress changed in the last update(). Only the realms
     * the model currently lists are reported, in the mode it lists them in.
     */
    struct ProgressChange {
        QString region;
        int previous = 0;
        int current = 0;
    };

    /**
     * Replaces the progress values with the ones in \a dclone, which is the
     * object d2emu.com returns, keyed by e.g. "euLadderHardcore", and returns
     * the realms whose progress changed.
     */
    QList<ProgressChange> update(const QJsonObject &dclone);

    /**
     * Whether to list the Return of the Warlock realms rather than the
     * regular ones.
     */
    void setRotw(bool rotw);

    /**
     * Restricts the listing to one game mode: only realms matching
     * \a hardcore are listed, and each reports its \a ladder progress.
     */
    void setMode(bool hardcore, bool ladder);

private:
    struct Realm {
        QString regionCode; /* eu, us, kr, cn */
        bool hardcore = false;
        bool rotw = false;
        int ladder = 0;
        int nonLadder = 0;
    };

    void rebuildRows();

    QList<Realm> m_realms;
    QList<int> m_rows; /* indices into m_realms, filtered by m_rotw and m_hardcore */
    bool m_rotw = false;
    bool m_hardcore = false;
    bool m_ladder = true;
};

/**
 * Fetches the current terror zones and Diablo Clone progress.
 *
 * Mirrors the original D2RLoader: either one request to the combined d2rinfo
 * endpoint, or one request each to the d2emu.com terror zone and Diablo Clone
 * endpoints using the configured account credentials.
 */
class GameInfo : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

    Q_PROPERTY(QStringList currentZones READ currentZones NOTIFY terrorZonesChanged)
    Q_PROPERTY(QStringList nextZones READ nextZones NOTIFY terrorZonesChanged)
    Q_PROPERTY(QDateTime nextTerrorTime READ nextTerrorTime NOTIFY terrorZonesChanged)
    Q_PROPERTY(QDateTime predictionAvailableTime READ predictionAvailableTime NOTIFY terrorZonesChanged)
    Q_PROPERTY(QStringList currentImmunities READ currentImmunities NOTIFY terrorZonesChanged)
    Q_PROPERTY(QStringList nextImmunities READ nextImmunities NOTIFY terrorZonesChanged)
    Q_PROPERTY(QList<int> currentBossPacks READ currentBossPacks NOTIFY terrorZonesChanged)
    Q_PROPERTY(QList<int> nextBossPacks READ nextBossPacks NOTIFY terrorZonesChanged)
    Q_PROPERTY(QStringList currentSuperUniques READ currentSuperUniques NOTIFY terrorZonesChanged)
    Q_PROPERTY(QStringList nextSuperUniques READ nextSuperUniques NOTIFY terrorZonesChanged)
    Q_PROPERTY(QDateTime lastUpdated READ lastUpdated NOTIFY lastUpdatedChanged)
    Q_PROPERTY(bool loading READ loading NOTIFY loadingChanged)
    Q_PROPERTY(QString errorString READ errorString NOTIFY errorStringChanged)
    Q_PROPERTY(DCloneModel *dcloneModel READ dcloneModel CONSTANT)
    Q_PROPERTY(QString dcloneModeName READ dcloneModeName NOTIFY dcloneModeNameChanged)

public:
    explicit GameInfo(QObject *parent = nullptr);

    QStringList currentZones() const;
    QStringList nextZones() const;
    QDateTime nextTerrorTime() const;

    /**
     * When the prediction of the next zone can first be fetched from the
     * endpoint in use, or an invalid time while unknown.
     */
    QDateTime predictionAvailableTime() const;

    /**
     * Immunity codes as the API reports them: f, c, l, p, m and ph.
     */
    QStringList currentImmunities() const;
    QStringList nextImmunities() const;

    /**
     * The minimum and maximum number of boss packs, or empty while unknown.
     */
    QList<int> currentBossPacks() const;
    QList<int> nextBossPacks() const;

    QStringList currentSuperUniques() const;
    QStringList nextSuperUniques() const;
    QDateTime lastUpdated() const;
    bool loading() const;
    QString errorString() const;
    DCloneModel *dcloneModel() const;

    /**
     * Human readable name of the currently selected Diablo Clone mode.
     */
    QString dcloneModeName() const;

    /**
     * Maps a terror zone level id to its name, as the API reports ids rather
     * than names. Returns the id itself if it is unknown.
     */
    static QString levelName(const QString &id);

    Q_INVOKABLE void refresh();

Q_SIGNALS:
    void terrorZonesChanged();
    void lastUpdatedChanged();
    void loadingChanged();
    void errorStringChanged();
    void dcloneModeNameChanged();

private:
    enum class Request {
        TerrorZones,
        DiabloClone,
        Combined,
    };

    /**
     * Starts, stops or re-times the automatic refresh to match the current
     * configuration.
     */
    void applyRefreshSchedule();

    /**
     * Arranges one extra refresh for the next change of the Terror Zone data:
     * the prediction of the next zone while it is missing, otherwise the
     * rotation to it. The regular interval is aligned with neither.
     */
    void scheduleTerrorZoneRefresh();

    /**
     * Pushes the configured mode onto the Diablo Clone model.
     */
    void applyDCloneMode();

    void send(const QUrl &url, Request request);
    void handle(QNetworkReply *reply, Request request);
    void applyTerrorZones(const QJsonObject &tz);

    /**
     * Posts a desktop notification when the Terror Zone rotated away from \a
     * previousZones, or when a new prediction replaced \a previousNextZones.
     */
    void notifyTerrorZones(const QStringList &previousZones, const QStringList &previousNextZones);

    /**
     * Posts a desktop notification for every realm in \a changes that climbed
     * to or past the configured threshold.
     */
    void notifyDCloneProgress(const QList<DCloneModel::ProgressChange> &changes);
    void setError(const QString &error);
    void finishRequest();

    QNetworkAccessManager *m_network = nullptr;
    QTimer *m_refreshTimer = nullptr;
    QTimer *m_terrorZoneTimer = nullptr;
    DCloneModel *m_dclone = nullptr;
    QStringList m_currentZones;
    QStringList m_nextZones;
    QDateTime m_nextTerrorTime;
    QDateTime m_predictionAvailableTime;
    QStringList m_currentImmunities;
    QStringList m_nextImmunities;
    QList<int> m_currentBossPacks;
    QList<int> m_nextBossPacks;
    QStringList m_currentSuperUniques;
    QStringList m_nextSuperUniques;
    QDateTime m_lastUpdated;
    QString m_errorString;
    int m_pending = 0;
};
