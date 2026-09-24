#include "gameinfo.h"
#include "../core/logging.h"
#include "d2rloaderconfig.h"

#include <KLocalizedString>
#include <KNotification>
#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>
#include <QLocale>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QRegularExpression>
#include <QTimer>
#include <QUrl>

namespace
{
/**
 * Endpoints of the original D2RLoader. The combined one needs no credentials,
 * the d2emu.com ones are authenticated with the configured account.
 */
const QUrl TerrorZoneUrl = QUrl(QStringLiteral("https://d2emu.com/api/v1/tz"));
const QUrl DiabloCloneUrl = QUrl(QStringLiteral("https://d2emu.com/api/v1/dclone"));
const QUrl CombinedUrl = QUrl(QStringLiteral("https://d2.someblocks.com/api/d2rinfo"));

/**
 * The d2rinfo endpoint only answers requests identifying as exactly
 * "D2RLoader" and returns 403 for anything else, including the longer
 * user agent that d2emu.com is sent.
 */
const QString CombinedUserAgent = QStringLiteral("D2RLoader");
const QString D2EmuUserAgent = QStringLiteral("D2RLoader (https://github.com/sh4nks/d2rloader)");

/**
 * Leaves room for the clocks of this machine and the server drifting apart.
 */
constexpr std::chrono::seconds ScheduledRefreshMargin(10);

/**
 * Terror zone level names, indexed by level id minus one. The API reports
 * numeric ids only.
 */
const QStringList &diabloLevels()
{
    static const QStringList levels = {
        QStringLiteral("Rogue Encampment"),
        QStringLiteral("Blood Moor"),
        QStringLiteral("Cold Plains"),
        QStringLiteral("Stony Field"),
        QStringLiteral("Dark Wood"),
        QStringLiteral("Black Marsh"),
        QStringLiteral("Tamoe Highland"),
        QStringLiteral("Den of Evil"),
        QStringLiteral("Cave 1"),
        QStringLiteral("Underground Passage 1"),
        QStringLiteral("Hole 1"),
        QStringLiteral("Pit 1"),
        QStringLiteral("Cave 2"),
        QStringLiteral("Underground Passage 2"),
        QStringLiteral("Hole 2"),
        QStringLiteral("Pit 2"),
        QStringLiteral("Burial Grounds"),
        QStringLiteral("Crypt"),
        QStringLiteral("Mausoleum"),
        QStringLiteral("Forgotten Tower"),
        QStringLiteral("Tower Cellar 1"),
        QStringLiteral("Tower Cellar 2"),
        QStringLiteral("Tower Cellar 3"),
        QStringLiteral("Tower Cellar 4"),
        QStringLiteral("Tower Cellar 5"),
        QStringLiteral("Monastery Gate"),
        QStringLiteral("Outer Cloister"),
        QStringLiteral("Barracks"),
        QStringLiteral("Jail 1"),
        QStringLiteral("Jail 2"),
        QStringLiteral("Jail 3"),
        QStringLiteral("Inner Cloister"),
        QStringLiteral("Cathedral"),
        QStringLiteral("Catacombs 1"),
        QStringLiteral("Catacombs 2"),
        QStringLiteral("Catacombs 3"),
        QStringLiteral("Catacombs 4"),
        QStringLiteral("Tristram"),
        QStringLiteral("The Secret Cow Level"),
        QStringLiteral("Lut Gholein"),
        QStringLiteral("Rocky Waste"),
        QStringLiteral("Dry Hills"),
        QStringLiteral("Far Oasis"),
        QStringLiteral("Lost City"),
        QStringLiteral("Valley of Snakes"),
        QStringLiteral("Canyon of the Magi"),
        QStringLiteral("Sewers 1"),
        QStringLiteral("Sewers 2"),
        QStringLiteral("Sewers 3"),
        QStringLiteral("Harem 1"),
        QStringLiteral("Harem 2"),
        QStringLiteral("Palace Cellar 1"),
        QStringLiteral("Palace Cellar 2"),
        QStringLiteral("Palace Cellar 3"),
        QStringLiteral("Stony Tomb 1"),
        QStringLiteral("Halls of the Dead 1"),
        QStringLiteral("Halls of the Dead 2"),
        QStringLiteral("Claw Viper Temple 1"),
        QStringLiteral("Stony Tomb 2"),
        QStringLiteral("Halls of the Dead 3"),
        QStringLiteral("Claw Viper Temple 2"),
        QStringLiteral("Maggot Lair 1"),
        QStringLiteral("Maggot Lair 2"),
        QStringLiteral("Maggot Lair 3"),
        QStringLiteral("Ancient Tunnels"),
        QStringLiteral("Tal Rasha's Tomb 1"),
        QStringLiteral("Tal Rasha's Tomb 2"),
        QStringLiteral("Tal Rasha's Tomb 3"),
        QStringLiteral("Tal Rasha's Tomb 4"),
        QStringLiteral("Tal Rasha's Tomb 5"),
        QStringLiteral("Tal Rasha's Tomb 6"),
        QStringLiteral("Tal Rasha's Tomb 7"),
        QStringLiteral("Tal Rasha's Chamber"),
        QStringLiteral("Arcane Sanctuary"),
        QStringLiteral("Kurast Docks"),
        QStringLiteral("Spider Forest"),
        QStringLiteral("Great Marsh"),
        QStringLiteral("Flayer Jungle"),
        QStringLiteral("Lower Kurast"),
        QStringLiteral("Kurast Bazaar"),
        QStringLiteral("Upper Kurast"),
        QStringLiteral("Kurast Causeway"),
        QStringLiteral("Travincal"),
        QStringLiteral("Archnid Lair"),
        QStringLiteral("Spider Cavern"),
        QStringLiteral("Swampy Pit 1"),
        QStringLiteral("Swampy Pit 2"),
        QStringLiteral("Flayer Dungeon 1"),
        QStringLiteral("Flayer Dungeon 2"),
        QStringLiteral("Swampy Pit 3"),
        QStringLiteral("Flayer Dungeon 3"),
        QStringLiteral("Sewers 1"),
        QStringLiteral("Sewers 2"),
        QStringLiteral("Ruined Temple"),
        QStringLiteral("Disused Fane"),
        QStringLiteral("Forgotten Reliquary"),
        QStringLiteral("Forgotten Temple"),
        QStringLiteral("Ruined Fane"),
        QStringLiteral("Disused Reliquary"),
        QStringLiteral("Durance of Hate 1"),
        QStringLiteral("Durance of Hate 2"),
        QStringLiteral("Durance of Hate 3"),
        QStringLiteral("Pandemonium Fortress"),
        QStringLiteral("Outer Steppes"),
        QStringLiteral("Plains of Despair"),
        QStringLiteral("City of the Damned"),
        QStringLiteral("River of Flame"),
        QStringLiteral("Chaos Sanctuary"),
        QStringLiteral("Harrogath"),
        QStringLiteral("Bloody Foothills"),
        QStringLiteral("Frigid Highlands"),
        QStringLiteral("Arreat Plateau"),
        QStringLiteral("Crystalline Passage"),
        QStringLiteral("Frozen River"),
        QStringLiteral("Glacial Trail"),
        QStringLiteral("Drifter Cavern"),
        QStringLiteral("Frozen Tundra"),
        QStringLiteral("The Ancients Way"),
        QStringLiteral("Icy Cellar"),
        QStringLiteral("Arreat Summit"),
        QStringLiteral("Nihlathaks Temple"),
        QStringLiteral("Halls of Anguish"),
        QStringLiteral("Halls of Pain"),
        QStringLiteral("Halls of Vaught"),
        QStringLiteral("Abaddon"),
        QStringLiteral("Pit of Acheron"),
        QStringLiteral("Infernal Pit"),
        QStringLiteral("Worldstone Keep 1"),
        QStringLiteral("Worldstone Keep 2"),
        QStringLiteral("Worldstone Keep 3"),
        QStringLiteral("Throne of Destruction"),
        QStringLiteral("Worldstone Chamber"),
        QStringLiteral("Matron's Den"),
        QStringLiteral("Forgotten Sands"),
        QStringLiteral("Furnace of Pain"),
        QStringLiteral("Uber Tristram"),
        QStringLiteral("Colossal Summit"),
    };
    return levels;
}

/**
 * Joins the numbered levels of one area, such as Jail 1 to 3, into a single
 * entry at the position of the first one. A lone numbered level is kept.
 */
QStringList compactLevels(const QStringList &levels)
{
    static const QRegularExpression numberedLevel(QStringLiteral("^(.+) \\d+$"));
    // Tal Rasha's Tombs are seven separate tombs rather than levels of one.
    static const QHash<QString, QString> areaNames = {
        {QStringLiteral("Tal Rasha's Tomb"), QStringLiteral("Tal Rasha's Tombs")},
        {QStringLiteral("Hole"), QStringLiteral("The Hole")},
        {QStringLiteral("Pit"), QStringLiteral("The Pit")},
    };

    const auto areaOf = [](const QString &level) {
        const QRegularExpressionMatch match = numberedLevel.match(level);
        return match.hasMatch() ? match.captured(1) : QString();
    };

    QHash<QString, int> levelsPerArea;
    for (const QString &level : levels) {
        const QString area = areaOf(level);
        if (!area.isEmpty()) {
            ++levelsPerArea[area];
        }
    }

    QStringList result;
    for (const QString &level : levels) {
        const QString area = areaOf(level);
        if (levelsPerArea.value(area) < 2) {
            result.append(level);
            continue;
        }
        const QString areaName = areaNames.value(area, area);
        if (!result.contains(areaName)) {
            result.append(areaName);
        }
    }
    return result;
}

/**
 * The realms the Diablo Clone table lists, in display order. Return of the
 * Warlock exists for every region except China.
 */
struct RealmSpec {
    const char *regionCode;
    bool hardcore;
    bool rotw;
};

constexpr RealmSpec RealmSpecs[] = {
    {"eu", false, false},
    {"eu", true, false},
    {"us", false, false},
    {"us", true, false},
    {"kr", false, false},
    {"kr", true, false},
    {"cn", false, false},
    {"cn", true, false},
    {"eu", false, true},
    {"eu", true, true},
    {"us", false, true},
    {"us", true, true},
    {"kr", false, true},
    {"kr", true, true},
};

QString regionName(const QString &code)
{
    if (code == QLatin1String("eu")) {
        return i18nc("@item:intable game server region", "Europe");
    }
    if (code == QLatin1String("us")) {
        return i18nc("@item:intable game server region", "America");
    }
    if (code == QLatin1String("kr")) {
        return i18nc("@item:intable game server region", "Asia");
    }
    if (code == QLatin1String("cn")) {
        return i18nc("@item:intable game server region", "China");
    }
    return code;
}
}

DCloneModel::DCloneModel(QObject *parent)
    : QAbstractListModel(parent)
{
    for (const RealmSpec &spec : RealmSpecs) {
        m_realms.append(Realm{QString::fromLatin1(spec.regionCode), spec.hardcore, spec.rotw, 0, 0});
    }
    rebuildRows();
}

int DCloneModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : m_rows.count();
}

QVariant DCloneModel::data(const QModelIndex &index, int role) const
{
    if (index.row() < 0 || index.row() >= m_rows.count()) {
        return QVariant();
    }

    const Realm &realm = m_realms.at(m_rows.at(index.row()));
    switch (role) {
    case RegionRole:
        return regionName(realm.regionCode);
    case ProgressRole:
        return m_ladder ? realm.ladder : realm.nonLadder;
    }
    return QVariant();
}

QHash<int, QByteArray> DCloneModel::roleNames() const
{
    static const QHash<int, QByteArray> roles{
        {RegionRole, "region"},
        {ProgressRole, "progress"},
    };
    return roles;
}

QList<DCloneModel::ProgressChange> DCloneModel::update(const QJsonObject &dclone)
{
    // A response without any realms would zero the whole table and then look
    // like a fresh climb on the next good fetch.
    if (dclone.isEmpty()) {
        qCWarning(LOG_GAMEINFO) << "no Diablo Clone data in the response, keeping the previous progress";
        return {};
    }

    const QList<Realm> before = m_realms;

    for (Realm &realm : m_realms) {
        realm.ladder = 0;
        realm.nonLadder = 0;
    }

    for (auto it = dclone.constBegin(); it != dclone.constEnd(); ++it) {
        // Keys look like "euLadder", "euNonLadderHardcore" or "usLadderRotw".
        const QString key = it.key();
        if (key.size() < 3) {
            continue;
        }

        const QString regionCode = key.left(2);
        const QString rest = key.mid(2);
        const bool hardcore = rest.contains(QLatin1String("Hardcore"));
        const bool rotw = rest.contains(QLatin1String("Rotw"));

        bool ladder = false;
        if (rest.startsWith(QLatin1String("Ladder"))) {
            ladder = true;
        } else if (!rest.startsWith(QLatin1String("Non"))) {
            qCWarning(LOG_GAMEINFO) << "unknown Diablo Clone key" << key;
            continue;
        }

        const int status = it.value().toObject()[QStringLiteral("status")].toInt();
        for (Realm &realm : m_realms) {
            if (realm.regionCode == regionCode && realm.hardcore == hardcore && realm.rotw == rotw) {
                (ladder ? realm.ladder : realm.nonLadder) = status;
                break;
            }
        }
    }

    if (!m_rows.isEmpty()) {
        Q_EMIT dataChanged(index(0), index(m_rows.count() - 1));
    }

    QList<ProgressChange> changes;
    for (int row : std::as_const(m_rows)) {
        const int previous = m_ladder ? before.at(row).ladder : before.at(row).nonLadder;
        const int current = m_ladder ? m_realms.at(row).ladder : m_realms.at(row).nonLadder;
        if (previous != current) {
            changes.append(ProgressChange{regionName(m_realms.at(row).regionCode), previous, current});
        }
    }
    return changes;
}

void DCloneModel::setRotw(bool rotw)
{
    if (m_rotw == rotw) {
        return;
    }
    m_rotw = rotw;
    beginResetModel();
    rebuildRows();
    endResetModel();
}

void DCloneModel::setMode(bool hardcore, bool ladder)
{
    if (m_hardcore == hardcore && m_ladder == ladder) {
        return;
    }

    // The ladder flag only picks which value each row reports, but the
    // hardcore flag changes which rows exist at all.
    const bool rowsChange = m_hardcore != hardcore;
    m_hardcore = hardcore;
    m_ladder = ladder;

    if (rowsChange) {
        beginResetModel();
        rebuildRows();
        endResetModel();
    } else if (!m_rows.isEmpty()) {
        Q_EMIT dataChanged(index(0), index(m_rows.count() - 1));
    }
}

void DCloneModel::rebuildRows()
{
    m_rows.clear();
    for (int i = 0; i < m_realms.count(); ++i) {
        const Realm &realm = m_realms.at(i);
        if (realm.rotw == m_rotw && realm.hardcore == m_hardcore) {
            m_rows.append(i);
        }
    }
}

GameInfo::GameInfo(QObject *parent)
    : QObject(parent)
    , m_network(new QNetworkAccessManager(this))
    , m_refreshTimer(new QTimer(this))
    , m_terrorZoneTimer(new QTimer(this))
    , m_dclone(new DCloneModel(this))
{
    m_terrorZoneTimer->setSingleShot(true);
    // A coarse timer may fire up to 5% early, which is minutes before the
    // prediction is out or the zone rotates.
    m_terrorZoneTimer->setTimerType(Qt::PreciseTimer);
    connect(m_terrorZoneTimer, &QTimer::timeout, this, &GameInfo::refresh);

    // Without a timeout a stalled request keeps loading() set, which blocks
    // every later refresh.
    m_network->setTransferTimeout(QNetworkRequest::DefaultTransferTimeout);

    m_dclone->setRotw(D2RLoaderConfig::self()->rotw());
    connect(D2RLoaderConfig::self(), &D2RLoaderConfig::rotwChanged, this, [this] {
        m_dclone->setRotw(D2RLoaderConfig::self()->rotw());
    });

    applyDCloneMode();
    connect(D2RLoaderConfig::self(), &D2RLoaderConfig::dcloneModeChanged, this, &GameInfo::applyDCloneMode);

    connect(m_refreshTimer, &QTimer::timeout, this, &GameInfo::refresh);
    connect(D2RLoaderConfig::self(), &D2RLoaderConfig::autoRefreshChanged, this, &GameInfo::applyRefreshSchedule);
    connect(D2RLoaderConfig::self(), &D2RLoaderConfig::refreshIntervalChanged, this, &GameInfo::applyRefreshSchedule);
    applyRefreshSchedule();

    if (D2RLoaderConfig::self()->autoRefresh()) {
        // Fetch once on startup, otherwise the first data only shows up after
        // a full interval has passed. Deferred so it does not run while the
        // singleton is still being constructed.
        QTimer::singleShot(0, this, &GameInfo::refresh);
    }
}

QString GameInfo::dcloneModeName() const
{
    switch (D2RLoaderConfig::self()->dcloneMode()) {
    case D2RLoaderConfig::EnumDcloneMode::Softcore:
        return i18nc("@title Diablo Clone game mode", "Softcore");
    case D2RLoaderConfig::EnumDcloneMode::Hardcore:
        return i18nc("@title Diablo Clone game mode", "Hardcore");
    case D2RLoaderConfig::EnumDcloneMode::SoftcoreLadder:
        return i18nc("@title Diablo Clone game mode", "Softcore Ladder");
    case D2RLoaderConfig::EnumDcloneMode::HardcoreLadder:
        return i18nc("@title Diablo Clone game mode", "Hardcore Ladder");
    }
    return QString();
}

void GameInfo::applyDCloneMode()
{
    const int mode = D2RLoaderConfig::self()->dcloneMode();
    const bool hardcore = mode == D2RLoaderConfig::EnumDcloneMode::Hardcore || mode == D2RLoaderConfig::EnumDcloneMode::HardcoreLadder;
    const bool ladder = mode == D2RLoaderConfig::EnumDcloneMode::SoftcoreLadder || mode == D2RLoaderConfig::EnumDcloneMode::HardcoreLadder;
    m_dclone->setMode(hardcore, ladder);
    Q_EMIT dcloneModeNameChanged();
}

void GameInfo::applyRefreshSchedule()
{
    scheduleTerrorZoneRefresh();

    if (!D2RLoaderConfig::self()->autoRefresh()) {
        m_refreshTimer->stop();
        return;
    }

    using namespace std::chrono;
    const minutes interval(std::max(1, D2RLoaderConfig::self()->refreshInterval()));
    m_refreshTimer->setInterval(interval);
    m_refreshTimer->start();
}

void GameInfo::scheduleTerrorZoneRefresh()
{
    m_terrorZoneTimer->stop();
    if (!D2RLoaderConfig::self()->autoRefresh()) {
        return;
    }

    const bool awaitingPrediction = m_nextZones.isEmpty();
    const QDateTime refreshTime = awaitingPrediction ? m_predictionAvailableTime : m_nextTerrorTime;
    if (!refreshTime.isValid()) {
        return;
    }

    const qint64 msecs = QDateTime::currentDateTime().msecsTo(refreshTime);
    if (msecs <= 0) {
        return;
    }

    if (awaitingPrediction) {
        qCInfo(LOG_GAMEINFO) << "Fetching the next Terror Zone prediction at" << refreshTime.toLocalTime().toString(Qt::ISODate);
    } else {
        qCInfo(LOG_GAMEINFO) << "Fetching the Terror Zone rotation at" << refreshTime.toLocalTime().toString(Qt::ISODate);
    }
    m_terrorZoneTimer->start(std::chrono::milliseconds(msecs) + ScheduledRefreshMargin);
}

QStringList GameInfo::currentZones() const
{
    return m_currentZones;
}

QStringList GameInfo::nextZones() const
{
    return m_nextZones;
}

QDateTime GameInfo::nextTerrorTime() const
{
    return m_nextTerrorTime;
}

QDateTime GameInfo::predictionAvailableTime() const
{
    return m_predictionAvailableTime;
}

QStringList GameInfo::currentImmunities() const
{
    return m_currentImmunities;
}

QStringList GameInfo::nextImmunities() const
{
    return m_nextImmunities;
}

QList<int> GameInfo::currentBossPacks() const
{
    return m_currentBossPacks;
}

QList<int> GameInfo::nextBossPacks() const
{
    return m_nextBossPacks;
}

QStringList GameInfo::currentSuperUniques() const
{
    return m_currentSuperUniques;
}

QStringList GameInfo::nextSuperUniques() const
{
    return m_nextSuperUniques;
}

QDateTime GameInfo::lastUpdated() const
{
    return m_lastUpdated;
}

bool GameInfo::loading() const
{
    return m_pending > 0;
}

QString GameInfo::errorString() const
{
    return m_errorString;
}

DCloneModel *GameInfo::dcloneModel() const
{
    return m_dclone;
}

QString GameInfo::levelName(const QString &id)
{
    bool ok = false;
    const int level = id.toInt(&ok);
    if (!ok || level < 1 || level > diabloLevels().count()) {
        return id;
    }
    return diabloLevels().at(level - 1);
}

void GameInfo::refresh()
{
    if (loading()) {
        return;
    }

    setError(QString());

    if (D2RLoaderConfig::self()->useD2RInfo()) {
        qCInfo(LOG_GAMEINFO) << "Refreshing Terror Zone and Diablo Clone data from" << CombinedUrl.toString();
        send(CombinedUrl, Request::Combined);
    } else {
        qCInfo(LOG_GAMEINFO) << "Refreshing Terror Zone data from" << TerrorZoneUrl.toString();
        send(TerrorZoneUrl, Request::TerrorZones);
        qCInfo(LOG_GAMEINFO) << "Refreshing Diablo Clone data from" << DiabloCloneUrl.toString();
        send(DiabloCloneUrl, Request::DiabloClone);
    }
}

void GameInfo::send(const QUrl &url, Request request)
{
    QNetworkRequest networkRequest(url);
    if (request == Request::Combined) {
        networkRequest.setHeader(QNetworkRequest::UserAgentHeader, CombinedUserAgent);
    } else {
        networkRequest.setHeader(QNetworkRequest::UserAgentHeader, D2EmuUserAgent);
        networkRequest.setRawHeader("x-emu-username", D2RLoaderConfig::self()->d2emuUser().toUtf8());
        networkRequest.setRawHeader("x-emu-token", D2RLoaderConfig::self()->d2emuToken().toUtf8());
    }

    ++m_pending;
    if (m_pending == 1) {
        Q_EMIT loadingChanged();
    }

    QNetworkReply *reply = m_network->get(networkRequest);
    connect(reply, &QNetworkReply::finished, this, [this, reply, request] {
        handle(reply, request);
    });
}

void GameInfo::handle(QNetworkReply *reply, Request request)
{
    reply->deleteLater();

    const int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    if (reply->error() != QNetworkReply::NoError || status != 200) {
        qCWarning(LOG_GAMEINFO) << "request failed" << reply->url() << status << reply->errorString();
        // The transfer timeout aborts the reply, which reports "Operation canceled".
        const QString reason =
            reply->error() == QNetworkReply::OperationCanceledError ? i18nc("@info", "The server did not respond in time.") : reply->errorString();
        setError(i18nc("@info", "Could not fetch game information: %1", reason));
        finishRequest();
        return;
    }

    QJsonParseError error;
    const QJsonDocument document = QJsonDocument::fromJson(reply->readAll(), &error);
    if (error.error != QJsonParseError::NoError) {
        qCWarning(LOG_GAMEINFO) << "malformed response from" << reply->url() << error.errorString();
        setError(i18nc("@info", "The server returned a malformed response."));
        finishRequest();
        return;
    }

    const QJsonObject root = document.object();
    switch (request) {
    case Request::TerrorZones:
        applyTerrorZones(root);
        break;
    case Request::DiabloClone:
        notifyDCloneProgress(m_dclone->update(root));
        break;
    case Request::Combined:
        applyTerrorZones(root[QStringLiteral("tz")].toObject());
        notifyDCloneProgress(m_dclone->update(root[QStringLiteral("dclone")].toObject()));
        break;
    }

    m_lastUpdated = QDateTime::currentDateTime();
    Q_EMIT lastUpdatedChanged();
    finishRequest();
}

void GameInfo::applyTerrorZones(const QJsonObject &tz)
{
    const auto names = [](const QJsonArray &ids) {
        QStringList result;
        result.reserve(ids.count());
        for (const QJsonValue &id : ids) {
            // Ids arrive as strings, but tolerate numbers too.
            result.append(levelName(id.isString() ? id.toString() : QString::number(id.toInt())));
        }
        return result;
    };
    const auto strings = [&tz](const QString &key) {
        const QJsonArray values = tz[key].toArray();
        QStringList result;
        result.reserve(values.count());
        for (const QJsonValue &value : values) {
            result.append(value.toString());
        }
        return result;
    };
    const auto integers = [&tz](const QString &key) {
        const QJsonArray values = tz[key].toArray();
        QList<int> result;
        result.reserve(values.count());
        for (const QJsonValue &value : values) {
            result.append(value.toInt());
        }
        return result;
    };

    m_currentImmunities = strings(QStringLiteral("current_immunities"));
    m_nextImmunities = strings(QStringLiteral("next_immunities"));
    m_currentBossPacks = integers(QStringLiteral("current_num_boss_packs"));
    m_nextBossPacks = integers(QStringLiteral("next_num_boss_packs"));
    m_currentSuperUniques = strings(QStringLiteral("current_superuniques"));
    m_nextSuperUniques = strings(QStringLiteral("next_superuniques"));

    const QStringList previousZones = m_currentZones;
    const QStringList previousNextZones = m_nextZones;
    m_currentZones = compactLevels(names(tz[QStringLiteral("current")].toArray()));
    m_nextZones = compactLevels(names(tz[QStringLiteral("next")].toArray()));

    const qint64 nextTerror = tz[QStringLiteral("next_terror_time_utc")].toInteger();
    m_nextTerrorTime = nextTerror > 0 ? QDateTime::fromSecsSinceEpoch(nextTerror) : QDateTime();

    const qint64 nextAvailable = tz[QStringLiteral("next_available_time_utc")].toInteger();
    m_predictionAvailableTime = nextAvailable > 0 ? QDateTime::fromSecsSinceEpoch(nextAvailable) : QDateTime();

    Q_EMIT terrorZonesChanged();
    scheduleTerrorZoneRefresh();
    notifyTerrorZones(previousZones, previousNextZones);
}

void GameInfo::notifyTerrorZones(const QStringList &previousZones, const QStringList &previousNextZones)
{
    if (!D2RLoaderConfig::self()->tzNotifications() || m_currentZones.isEmpty()) {
        return;
    }

    // The first fetch has nothing to compare against, and the zones it reports
    // are on screen anyway.
    if (previousZones.isEmpty()) {
        return;
    }

    const QString current = m_currentZones.join(QStringLiteral(", "));
    const QString next = m_nextZones.join(QStringLiteral(", "));

    if (previousZones != m_currentZones) {
        const QString text =
            m_nextZones.isEmpty() ? current : i18nc("@info the terror zone that is active now, then the one after it", "%1\nNext: %2", current, next);

        qCInfo(LOG_GAMEINFO) << "notifying about terror zone" << current;
        KNotification::event(QStringLiteral("terrorzone"), i18nc("@title:window", "Terror Zone Changed"), text);
        return;
    }

    // The prediction is published a while after the rotation, so it usually
    // shows up on a later fetch than the zone change itself.
    if (m_nextZones.isEmpty() || previousNextZones == m_nextZones) {
        return;
    }

    const QString text = m_nextTerrorTime.isValid() ? i18nc("@info the predicted terror zone, then the time it starts",
                                                            "%1\nStarts at %2",
                                                            next,
                                                            QLocale().toString(m_nextTerrorTime.toLocalTime().time(), QLocale::ShortFormat))
                                                    : next;

    qCInfo(LOG_GAMEINFO) << "notifying about next terror zone" << next;
    KNotification::event(QStringLiteral("nextterrorzone"), i18nc("@title:window", "Next Terror Zone Predicted"), text);
}

void GameInfo::notifyDCloneProgress(const QList<DCloneModel::ProgressChange> &changes)
{
    if (!D2RLoaderConfig::self()->dcloneNotifications()) {
        return;
    }

    const int threshold = D2RLoaderConfig::self()->dcloneThreshold();
    for (const DCloneModel::ProgressChange &change : changes) {
        if (change.current <= change.previous || change.current < threshold) {
            continue;
        }

        const bool walking = change.current == DCloneModel::MaxProgress;
        const QString realm = D2RLoaderConfig::self()->rotw() ? i18nc("@info a Return of the Warlock realm", "%1 (RotW)", change.region) : change.region;

        qCInfo(LOG_GAMEINFO) << "notifying about Diablo Clone progress" << realm << change.previous << "->" << change.current;

        auto *notification = new KNotification(QStringLiteral("dclone"));
        notification->setTitle(walking ? i18nc("@title:window", "Diablo Walks the Earth") : i18nc("@title:window", "Diablo Clone Progress"));
        notification->setText(i18nc("@info realm, game mode, progress out of the maximum",
                                    "%1 (%2) is at %3/%4.",
                                    realm,
                                    dcloneModeName(),
                                    change.current,
                                    DCloneModel::MaxProgress));
        notification->setUrgency(walking ? KNotification::CriticalUrgency : KNotification::NormalUrgency);
        notification->sendEvent();
    }
}

void GameInfo::setError(const QString &error)
{
    if (m_errorString == error) {
        return;
    }
    m_errorString = error;
    Q_EMIT errorStringChanged();
}

void GameInfo::finishRequest()
{
    if (m_pending > 0 && --m_pending == 0) {
        Q_EMIT loadingChanged();
    }
}
