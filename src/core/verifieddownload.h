#pragma once

#include <QByteArray>
#include <QCryptographicHash>
#include <QFile>
#include <QObject>
#include <QString>
#include <QUrl>

class QNetworkAccessManager;
class QNetworkReply;

/**
 * Downloads a file and keeps it only when its SHA-256 is the expected one.
 * The transfer starts with the object and ends with finished().
 */
class VerifiedDownload : public QObject
{
    Q_OBJECT

public:
    VerifiedDownload(QNetworkAccessManager *network, const QUrl &url, const QByteArray &sha256, const QString &target, QObject *parent = nullptr);

    /**
     * Aborts the transfer, which then ends with an error like any other.
     */
    void cancel();

Q_SIGNALS:
    void progress(qint64 received, qint64 total);

    /**
     * \a error is empty when the target holds the verified file. Otherwise
     * the target has been removed.
     */
    void finished(const QString &error);

private:
    void read();
    void finish();

    QByteArray m_sha256;
    QFile m_file;
    QCryptographicHash m_hash;
    QNetworkReply *m_reply = nullptr;
    QString m_error;
};
