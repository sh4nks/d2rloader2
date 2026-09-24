#pragma once

#include <QDateTime>
#include <QFile>
#include <QJSEngine>
#include <QList>
#include <QObject>
#include <QQmlEngine>
#include <QString>
#include <QVariantList>
#include <qqmlintegration.h>

/**
 * The application log as shown in the "Application Log" tab.
 *
 * Entries come from Qt's logging: install() puts a message handler in front
 * of the one that was active before, so everything logged still reaches
 * stderr as well, filtered or not.
 *
 * Only the application's own categories are kept - see isApplicationCategory()
 * in logbuffer.cpp and the categories in logging.h. The same entries go to
 * d2rloader.log in logPath while logToFile is set, and logLevel decides which
 * severities those categories emit at all, stderr included.
 */
class LogBuffer : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

public:
    /**
     * Entries kept before the oldest ones are dropped, and how many are
     * dropped at once. Trimming in chunks keeps the view from rebuilding
     * itself on every message once the buffer is full.
     */
    static constexpr int MaxEntries = 2000;
    static constexpr int TrimChunk = 200;

    /**
     * Size past which d2rloader.log is moved to d2rloader.log.1 when the
     * file is opened, replacing the previous one.
     */
    static constexpr qint64 MaxFileSize = 5 * 1024 * 1024;

    static LogBuffer &instance();
    static LogBuffer *create(QQmlEngine *, QJSEngine *);

    /**
     * Routes Qt logging into the buffer. Call once, after the application
     * object exists.
     */
    static void install();

    /**
     * Every entry kept so far, oldest first, as {timestamp, level, message}.
     */
    Q_INVOKABLE QVariantList entries() const;

    Q_INVOKABLE void clear();

    /**
     * Called from the message handler, on whatever thread logged. The entry
     * is queued onto the buffer's thread before it is appended.
     */
    void post(QtMsgType type, const QMessageLogContext &context, const QString &message);

Q_SIGNALS:
    /**
     * A single new entry, for a view that appends rather than rebuilds.
     */
    void entryAdded(const QString &timestamp, const QString &level, const QString &message);

    /**
     * Entries were dropped or cleared; a view has to rebuild from entries().
     */
    void reset();

private:
    struct Entry {
        QDateTime time;
        QString level;
        QString message;
    };

    explicit LogBuffer(QObject *parent = nullptr);

    void append(Entry entry);
    void applyLogLevel();
    void applyLogFile();

    QList<Entry> m_entries;
    QFile m_file;
};
