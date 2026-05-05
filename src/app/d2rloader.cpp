#include "d2rloader.h"

#include <QColor>
#include <QDir>
#include <QSize>

#include <QCoreApplication>

#include <QGuiApplication>
#include <QPalette>
#include <QSettings>
#include <QStyleHints>
#include <qobject.h>
#include <qstringliteral.h>

/* ************************************************************************** */

D2RLoader *D2RLoader::instance = nullptr;

D2RLoader *D2RLoader::getInstance()
{
    if (instance == nullptr) {
        instance = new D2RLoader();
    }

    return instance;
}

D2RLoader::D2RLoader()
{
    // Set default application path
    m_appPath = QCoreApplication::applicationDirPath();
    qGuiApp->setApplicationName(appName());
    qGuiApp->setApplicationDisplayName(appName());
    qGuiApp->setApplicationVersion(appVersion());

    qDebug() << "qDebug: " << m_appPath;
    // Make sure the path is terminated with a separator?
    // if (!m_appPath.endsWith('/')) m_appPath += '/';
}

D2RLoader::~D2RLoader()
{
    //
}

QString D2RLoader::appName()
{
    return QString::fromLatin1(APP_NAME);
}

QString D2RLoader::appVersion()
{
    return QString::fromLatin1(APP_VERSION);
}

QString D2RLoader::appBuildDate()
{
    return QString::fromLatin1(__DATE__);
}

QString D2RLoader::appBuildDateTime()
{
    QString date = QString::fromLatin1(__DATE__);
    return date;
}

QString D2RLoader::appBuildMode()
{
#if !defined(QT_NO_DEBUG) && !defined(NDEBUG)
    return QStringLiteral("DEBUG");
#endif
    return QStringLiteral("");
}

QString D2RLoader::qtVersion()
{
    return QString::fromStdString(qVersion());
}

void D2RLoader::appExit()
{
    QCoreApplication::exit();
}

void D2RLoader::setAppPath(const QString &value)
{
    if (m_appPath != value) {
        QDir newPath(value);
        newPath.cdUp();
        m_appPath = newPath.absolutePath();

        // Make sure the path is terminated with a separator.
        if (!m_appPath.endsWith(QStringLiteral("/"))) {
            m_appPath = m_appPath + QStringLiteral("/");
        }
    }
}

bool D2RLoader::isOsThemeDark()
{
    const QStyleHints *styleHints = QGuiApplication::styleHints();
    return (styleHints && styleHints->colorScheme() == Qt::ColorScheme::Dark);
}

void D2RLoader::registerSettingFormats()
{
    QSettings appSettings(QStringLiteral("path1.ini"), QSettings::IniFormat);
    QSettings gameSettings(QStringLiteral("path1.ini"), QSettings::IniFormat);
}
