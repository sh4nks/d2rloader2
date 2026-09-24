#include "launchsequencemanager.h"
#include "../core/logging.h"
#include "d2rloaderconfig.h"
#include "profilemanager.h"

#include <KLocalizedString>
#include <QDir>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSaveFile>

namespace
{
constexpr int SequencesFileVersion = 1;
}

LaunchSequenceManager::LaunchSequenceManager(QObject *parent)
    : QAbstractListModel(parent)
{
    if (const QString error = read(); !error.isEmpty()) {
        qCWarning(LOG_PROFILES) << "cannot load" << filePath() << ":" << error;
        m_loadError = i18nc("@info %1 is the file, %2 the reason",
                            "Could not load the launch sequences from \"%1\": %2\nChanges to the launch sequences will not be saved.",
                            filePath(),
                            error);
    }
    connect(&ProfileManager::instance(), &ProfileManager::profileRemoved, this, &LaunchSequenceManager::forgetProfile);
}

LaunchSequenceManager &LaunchSequenceManager::instance()
{
    static LaunchSequenceManager launchSequenceManager;
    return launchSequenceManager;
}

int LaunchSequenceManager::rowCount(const QModelIndex &) const
{
    return m_sequences.count();
}

QVariant LaunchSequenceManager::data(const QModelIndex &index, int role) const
{
    if (index.row() < 0 || index.row() >= m_sequences.count()) {
        return QVariant();
    }

    const Sequence &sequence = m_sequences.at(index.row());
    switch (role) {
    case Qt::DisplayRole:
    case NameRole:
        return sequence.name;
    case SequenceIdRole:
        return sequence.id;
    case ProfileIdsRole: {
        QVariantList ids;
        for (const int id : sequence.profileIds) {
            ids.append(id);
        }
        return ids;
    }
    case AccountCountRole:
        return sequence.profileIds.count();
    }
    return QVariant();
}

QHash<int, QByteArray> LaunchSequenceManager::roleNames() const
{
    static QHash<int, QByteArray> roles{
        {Qt::DisplayRole, "display"},
        {SequenceIdRole, "sequenceId"},
        {NameRole, "name"},
        {ProfileIdsRole, "profileIds"},
        {AccountCountRole, "accountCount"},
    };
    return roles;
}

int LaunchSequenceManager::currentIndex() const
{
    if (m_sequences.isEmpty()) {
        return -1;
    }

    const int selected = D2RLoaderConfig::self()->selectedSequence();
    for (int row = 0; row < m_sequences.count(); ++row) {
        if (m_sequences.at(row).id == selected) {
            return row;
        }
    }
    return 0;
}

void LaunchSequenceManager::setCurrentIndex(int row)
{
    if (row < 0 || row >= m_sequences.count() || row == currentIndex()) {
        return;
    }

    D2RLoaderConfig::self()->setSelectedSequence(m_sequences.at(row).id);
    D2RLoaderConfig::self()->save();
    Q_EMIT currentIndexChanged();
}

QString LaunchSequenceManager::loadError() const
{
    return m_loadError;
}

void LaunchSequenceManager::save(int row, const QString &name, const QList<int> &profileIds)
{
    if (row >= 0 && row < m_sequences.count()) {
        m_sequences[row].name = name;
        m_sequences[row].profileIds = profileIds;
        Q_EMIT dataChanged(index(row), index(row));
    } else {
        int nextId = 1;
        for (const Sequence &sequence : std::as_const(m_sequences)) {
            nextId = std::max(nextId, sequence.id + 1);
        }

        beginInsertRows(QModelIndex(), m_sequences.count(), m_sequences.count());
        m_sequences.append({nextId, name, profileIds});
        endInsertRows();
        Q_EMIT currentIndexChanged();
    }
    write();
}

void LaunchSequenceManager::remove(int row)
{
    if (row < 0 || row >= m_sequences.count()) {
        return;
    }

    beginRemoveRows(QModelIndex(), row, row);
    m_sequences.removeAt(row);
    endRemoveRows();
    Q_EMIT currentIndexChanged();
    write();
}

void LaunchSequenceManager::moveUp(int row)
{
    if (row <= 0 || row >= m_sequences.count()) {
        return;
    }

    if (beginMoveRows(QModelIndex(), row, row, QModelIndex(), row - 1)) {
        m_sequences.move(row, row - 1);
        endMoveRows();
        Q_EMIT currentIndexChanged();
        write();
    }
}

void LaunchSequenceManager::moveDown(int row)
{
    if (row < 0 || row >= m_sequences.count() - 1) {
        return;
    }

    if (beginMoveRows(QModelIndex(), row, row, QModelIndex(), row + 2)) {
        m_sequences.move(row, row + 1);
        endMoveRows();
        Q_EMIT currentIndexChanged();
        write();
    }
}

QString LaunchSequenceManager::name(int row) const
{
    if (row < 0 || row >= m_sequences.count()) {
        return QString();
    }
    return m_sequences.at(row).name;
}

QList<Profile *> LaunchSequenceManager::profiles(int row) const
{
    QList<Profile *> result;
    if (row < 0 || row >= m_sequences.count()) {
        return result;
    }

    const QList<int> &ids = m_sequences.at(row).profileIds;
    const QList<Profile *> all = ProfileManager::instance().profiles();
    for (Profile *profile : all) {
        if (ids.contains(profile->id())) {
            result.append(profile);
        }
    }
    return result;
}

void LaunchSequenceManager::forgetProfile(Profile *profile)
{
    bool changed = false;
    for (int row = 0; row < m_sequences.count(); ++row) {
        if (m_sequences[row].profileIds.removeAll(profile->id()) > 0) {
            Q_EMIT dataChanged(index(row), index(row));
            changed = true;
        }
    }
    if (changed) {
        write();
    }
}

QString LaunchSequenceManager::filePath() const
{
    return QFileInfo(D2RLoaderConfig::self()->profilePath()).dir().filePath(QStringLiteral("launch_sequences.json"));
}

QString LaunchSequenceManager::read()
{
    QFile file(filePath());
    if (!file.exists()) {
        m_loaded = true;
        return QString();
    }

    if (!file.open(QIODevice::ReadOnly)) {
        return file.errorString();
    }

    QJsonParseError error;
    const QJsonDocument document = QJsonDocument::fromJson(file.readAll(), &error);
    if (error.error != QJsonParseError::NoError) {
        return i18nc("@info %1 is the parser error", "The file is not valid JSON: %1", error.errorString());
    }

    const QJsonObject root = document.object();
    if (root[QStringLiteral("version")].toInt() > SequencesFileVersion) {
        return i18nc("@info", "The file was written by a newer version of D2RLoader.");
    }

    const QJsonArray sequences = root[QStringLiteral("sequences")].toArray();
    for (const QJsonValue &value : sequences) {
        const QJsonObject object = value.toObject();
        Sequence sequence{object[QStringLiteral("id")].toInt(), object[QStringLiteral("name")].toString(), {}};
        const QJsonArray ids = object[QStringLiteral("accounts")].toArray();
        for (const QJsonValue &id : ids) {
            sequence.profileIds.append(id.toInt());
        }
        m_sequences.append(sequence);
    }

    m_loaded = true;
    return QString();
}

bool LaunchSequenceManager::write()
{
    const QString path = filePath();

    // Overwriting a file that could not be read would destroy the sequences
    // in it.
    if (!m_loaded) {
        qCWarning(LOG_PROFILES) << "refusing to overwrite" << path << "because it could not be loaded";
        return false;
    }

    const QDir directory = QFileInfo(path).absoluteDir();
    if (!directory.exists() && !directory.mkpath(QStringLiteral("."))) {
        qCWarning(LOG_PROFILES) << "cannot create" << directory.absolutePath();
        return false;
    }

    QJsonArray sequences;
    for (const Sequence &sequence : std::as_const(m_sequences)) {
        QJsonArray ids;
        for (const int id : sequence.profileIds) {
            ids.append(id);
        }
        sequences.append(QJsonObject{
            {QStringLiteral("id"), sequence.id},
            {QStringLiteral("name"), sequence.name},
            {QStringLiteral("accounts"), ids},
        });
    }

    const QJsonObject root{
        {QStringLiteral("version"), SequencesFileVersion},
        {QStringLiteral("sequences"), sequences},
    };

    QSaveFile file(path);
    if (!file.open(QIODevice::WriteOnly)) {
        qCWarning(LOG_PROFILES) << "cannot write" << path << file.errorString();
        return false;
    }
    file.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
    if (!file.commit()) {
        qCWarning(LOG_PROFILES) << "cannot commit" << path << file.errorString();
        return false;
    }
    return true;
}
