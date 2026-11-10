#include "verifieddownload.h"

#include <KLocalizedString>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>

VerifiedDownload::VerifiedDownload(QNetworkAccessManager *network, const QUrl &url, const QByteArray &sha256, const QString &target, QObject *parent)
    : QObject(parent)
    , m_sha256(sha256)
    , m_file(target)
    , m_hash(QCryptographicHash::Sha256)
{
    if (!m_file.open(QIODevice::WriteOnly)) {
        // Queued, because nothing can be connected to finished() yet.
        QMetaObject::invokeMethod(
            this,
            [this] {
                Q_EMIT finished(m_file.errorString());
            },
            Qt::QueuedConnection);
        return;
    }

    m_reply = network->get(QNetworkRequest(url));
    connect(m_reply, &QNetworkReply::readyRead, this, &VerifiedDownload::read);
    connect(m_reply, &QNetworkReply::downloadProgress, this, &VerifiedDownload::progress);
    connect(m_reply, &QNetworkReply::finished, this, &VerifiedDownload::finish);
}

void VerifiedDownload::cancel()
{
    if (m_reply) {
        m_reply->abort();
    }
}

void VerifiedDownload::read()
{
    const QByteArray chunk = m_reply->readAll();
    m_hash.addData(chunk);
    if (m_file.write(chunk) != chunk.size()) {
        m_error = m_file.errorString();
        // Ends in finish(), which reports m_error.
        m_reply->abort();
    }
}

void VerifiedDownload::finish()
{
    if (m_error.isEmpty()) {
        read();
    }
    m_file.close();

    QString error = m_error;
    if (error.isEmpty() && m_reply->error() != QNetworkReply::NoError) {
        error = m_reply->errorString();
    }
    if (error.isEmpty() && m_hash.result() != m_sha256) {
        error = i18nc("@info", "The download does not match the checksum of the release.");
    }
    if (!error.isEmpty()) {
        m_file.remove();
    }

    m_reply->deleteLater();
    m_reply = nullptr;
    Q_EMIT finished(error);
}
