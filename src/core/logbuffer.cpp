#include "logbuffer.h"

#include "d2rloaderconfig.h"
#include "logging.h"

#include <QCoreApplication>
#include <QDir>
#include <QLoggingCategory>
#include <QMetaObject>
#include <QStringList>
#include <QVariantMap>

namespace
{
QtMessageHandler s_previousHandler = nullptr;

QString levelName(QtMsgType type)
{
    switch (type) {
    case QtDebugMsg:
        return QStringLiteral("DEBUG");
    case QtInfoMsg:
        return QStringLiteral("INFO");
    case QtWarningMsg:
        return QStringLiteral("WARNING");
    case QtCriticalMsg:
        return QStringLiteral("ERROR");
    case QtFatalMsg:
        return QStringLiteral("FATAL");
    }
    return QStringLiteral("INFO");
}

/**
 * The categories the "Application Log" tab shows. Everything else - Qt, KF,
 * QML engine warnings, and QML console output that was not given the Log
 * category - is left to stderr alone.
 *
 * "default", where an uncategorized qDebug()/qWarning() lands, is deliberately
 * not here: the QML engine reports binding errors through it, so allowing it
 * fills the tab with warnings from Kirigami and Qt internals. Log through one
 * of the categories in logging.h instead.
 */
bool isApplicationCategory(const QString &category)
{
    static const QStringList allowed{
        QStringLiteral("d2rloader.app"),
        QStringLiteral("d2rloader.profiles"),
        QStringLiteral("d2rloader.gameinfo"),
        QStringLiteral("d2rloader.game"),
        QStringLiteral("d2rloader.qml"),
    };
    return allowed.contains(category);
}

void messageHandler(QtMsgType type, const QMessageLogContext &context, const QString &message)
{
    if (s_previousHandler) {
        s_previousHandler(type, context, message);
    }
    LogBuffer::instance().post(type, context, message);
}
}

LogBuffer::LogBuffer(QObject *parent)
    : QObject(parent)
{
}

LogBuffer &LogBuffer::instance()
{
    static LogBuffer self;
    return self;
}

LogBuffer *LogBuffer::create(QQmlEngine *, QJSEngine *)
{
    auto *self = &instance();
    QJSEngine::setObjectOwnership(self, QJSEngine::CppOwnership);
    return self;
}

void LogBuffer::install()
{
    LogBuffer &self = instance();
    self.moveToThread(QCoreApplication::instance()->thread());
    s_previousHandler = qInstallMessageHandler(messageHandler);

    self.applyLogLevel();
    self.applyLogFile();

    auto *config = D2RLoaderConfig::self();
    connect(config, &D2RLoaderConfig::logLevelChanged, &self, &LogBuffer::applyLogLevel);
    connect(config, &D2RLoaderConfig::logToFileChanged, &self, &LogBuffer::applyLogFile);
    connect(config, &D2RLoaderConfig::logPathChanged, &self, &LogBuffer::applyLogFile);
}

QVariantList LogBuffer::entries() const
{
    QVariantList list;
    list.reserve(m_entries.size());
    for (const Entry &entry : m_entries) {
        list.append(QVariantMap{
            {QStringLiteral("timestamp"), entry.time.toString(QStringLiteral("hh:mm:ss"))},
            {QStringLiteral("level"), entry.level},
            {QStringLiteral("message"), entry.message},
        });
    }
    return list;
}

void LogBuffer::clear()
{
    m_entries.clear();
    Q_EMIT reset();
}

void LogBuffer::post(QtMsgType type, const QMessageLogContext &context, const QString &message)
{
    const QString category = QString::fromUtf8(context.category ? context.category : "default");
    if (!isApplicationCategory(category)) {
        return;
    }

    // "d2rloader.gameinfo" reads as "[gameinfo]".
    const QString label = category.mid(category.indexOf(QLatin1Char('.')) + 1);

    Entry entry{
        QDateTime::currentDateTime(),
        levelName(type),
        QStringLiteral("[%1] %2").arg(label, message),
    };

    QMetaObject::invokeMethod(
        this,
        [this, entry = std::move(entry)]() mutable {
            append(std::move(entry));
        },
        Qt::QueuedConnection);
}

void LogBuffer::append(Entry entry)
{
    if (m_file.isOpen()) {
        const QString line = QStringLiteral("%1 %2 %3\n").arg(entry.time.toString(Qt::ISODate), entry.level.leftJustified(7), entry.message);
        m_file.write(line.toUtf8());
        m_file.flush();
    }

    m_entries.append(std::move(entry));

    if (m_entries.size() > MaxEntries) {
        m_entries.remove(0, TrimChunk);
        Q_EMIT reset();
        return;
    }

    const Entry &added = m_entries.last();
    Q_EMIT entryAdded(added.time.toString(QStringLiteral("hh:mm:ss")), added.level, added.message);
}

/**
 * Errors are always emitted. Rules from QT_LOGGING_RULES or QT_LOGGING_CONF
 * still override these, so a debug run does not need the setting changed.
 */
void LogBuffer::applyLogLevel()
{
    using Level = D2RLoaderConfig::EnumLogLevel;
    const int lowest = D2RLoaderConfig::self()->logLevel();
    const auto rule = [lowest](const QString &type, int level) {
        return QStringLiteral("d2rloader.*.%1=%2\n").arg(type, level >= lowest ? QStringLiteral("true") : QStringLiteral("false"));
    };

    QLoggingCategory::setFilterRules(rule(QStringLiteral("debug"), Level::Debug) + rule(QStringLiteral("info"), Level::Info)
                                     + rule(QStringLiteral("warning"), Level::Warn));
}

void LogBuffer::applyLogFile()
{
    m_file.close();

    const auto *config = D2RLoaderConfig::self();
    if (!config->logToFile()) {
        return;
    }

    const QDir dir(config->logPath().isEmpty() ? config->defaultLogPathValue() : config->logPath());
    if (!dir.mkpath(QStringLiteral("."))) {
        qCWarning(LOG_APP) << "cannot create log directory" << dir.path();
        return;
    }

    m_file.setFileName(dir.filePath(QStringLiteral("d2rloader.log")));
    if (m_file.size() > MaxFileSize) {
        const QString previous = m_file.fileName() + QStringLiteral(".1");
        QFile::remove(previous);
        QFile::rename(m_file.fileName(), previous);
    }

    if (!m_file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
        qCWarning(LOG_APP) << "cannot open log file" << m_file.fileName() << m_file.errorString();
    }
}
