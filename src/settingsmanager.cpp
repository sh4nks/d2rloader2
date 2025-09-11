#include "settingsmanager.h"
#include <QCoreApplication>
#include <QSettings>
#include <QLocale>
#include <QDebug>

#include "d2rloader.h"

SettingsManager *SettingsManager::instance = nullptr;

SettingsManager *SettingsManager::getInstance() {
    if (instance == nullptr) {
        instance = new SettingsManager();
    }

    return instance;
}

SettingsManager::SettingsManager() {
    readSettings();
}

SettingsManager::~SettingsManager() {
    //
}

bool SettingsManager::readSettings() {
    bool status = false;

    QSettings settings(QSettings::Format::IniFormat, QSettings::UserScope, D2RLoader::appName());

    if (settings.status() == QSettings::NoError) {
        if (settings.contains("ApplicationWindow/width")) {
            m_appSize.setWidth(settings.value("ApplicationWindow/width").toInt());
        }
        if (settings.contains("ApplicationWindow/height")) {
            m_appSize.setHeight(settings.value("ApplicationWindow/height").toInt());
        }
        if (settings.contains("settings/appTheme")) {
            m_appTheme = settings.value("settings/appTheme").toString();
        }
        if (settings.contains("settings/profilePath")) {
            m_profilePath = settings.value("settings/profilePath").toString();
        }
        if (settings.contains("settings/handlePath")) {
            m_handlePath = settings.value("settings/handlePath").toString();
        }
        if (settings.contains("settings/gamePath")) {
            m_gamePath = settings.value("settings/gamePath").toString();
        }
        if (settings.contains("settings/wineprefixPath")) {
            m_wineprefixPath = settings.value("settings/wineprefixPath").toString();
        }
        if (settings.contains("settings/d2emuToken")) {
            m_d2emuToken = settings.value("settings/d2emuToken").toString();
        }
        if (settings.contains("settings/d2emuUser")) {
            m_d2emuUser = settings.value("settings/d2emuUser").toString();
        }
        if (settings.contains("settings/logPath")) {
            m_logPath = settings.value("settings/logPath").toString();
        }
        if (settings.contains("settings/logLevel")) {
            m_logLevel = settings.value("settings/logLevel").toString();
        }
        qWarning() << "SettingsManager::readSettings() read settings from:" << settings.fileName();
        qWarning() << "SettingsManager::readSettings() read settings from:" << settings.allKeys();
        status = true;
    } else {
        qWarning() << "SettingsManager::readSettings() error:" << settings.status();
        qWarning() << "SettingsManager::readSettings() error:" << settings.fileName();
    }

    return status;
}


bool SettingsManager::writeSettings() const {
    bool status = false;

    QSettings settings(QSettings::Format::IniFormat, QSettings::UserScope, D2RLoader::appName());

    if (settings.isWritable()) {
        settings.setValue("settings/appTheme", m_appTheme);
        settings.setValue("settings/profilePath", m_profilePath);
        settings.setValue("settings/handlePath", m_handlePath);
        settings.setValue("settings/gamePath", m_gamePath);
        settings.setValue("settings/wineprefixPath", m_wineprefixPath);
        settings.setValue("settings/d2emuToken", m_d2emuToken);
        settings.setValue("settings/d2emuUser", m_d2emuUser);
        settings.setValue("settings/logPath", m_logPath);
        settings.setValue("settings/logLevel", m_logLevel);

        if (settings.status() == QSettings::NoError) {
            status = true;
        } else {
            qWarning() << "SettingsManager::writeSettings() error (fileName):" << settings.fileName();
            qWarning() << "SettingsManager::writeSettings() error (status):" << settings.status();
        }
    } else {
        qWarning() << "SettingsManager::writeSettings() error: read only file?";
    }

    return status;
}

void SettingsManager::resetSettings() {
    m_appTheme = "Fusion";
    Q_EMIT appThemeChanged();
}


void SettingsManager::setAppTheme(const QString &value) {
    if (m_appTheme != value) {
        m_appTheme = value;
        Q_EMIT appThemeChanged();
    }
}

QSize SettingsManager::getAppSize() const {
    return m_appSize;
}

void SettingsManager::setAppSize(const QSize &newAppSize) {
    if (m_appSize == newAppSize)
        return;
    m_appSize = newAppSize;
    emit appSizeChanged();
}

QSize SettingsManager::getAppPosition() const {
    return m_appPosition;
}

void SettingsManager::setAppPosition(const QSize &newAppPosition) {
    if (m_appPosition == newAppPosition)
        return;
    m_appPosition = newAppPosition;
    emit appPositionChanged();
}

QString SettingsManager::getProfilePath() const {
    return m_profilePath;
}

void SettingsManager::setProfilePath(const QString &newProfilePath) {
    if (m_profilePath == newProfilePath)
        return;
    m_profilePath = newProfilePath;
    emit profilePathChanged();
}

QString SettingsManager::getHandlePath() const {
    return m_handlePath;
}

void SettingsManager::setHandlePath(const QString &newHandlePath) {
    if (m_handlePath == newHandlePath)
        return;
    m_handlePath = newHandlePath;
    emit handlePathChanged();
}

QString SettingsManager::getGamePath() const {
    return m_gamePath;
}

void SettingsManager::setGamePath(const QString &newGamePath) {
    if (m_gamePath == newGamePath)
        return;
    m_gamePath = newGamePath;
    emit gamePathChanged();
}

QString SettingsManager::getWineprefixPath() const {
    return m_wineprefixPath;
}

void SettingsManager::setWineprefixPath(const QString &newWineprefixPath) {
    if (m_wineprefixPath == newWineprefixPath)
        return;
    m_wineprefixPath = newWineprefixPath;
    emit wineprefixPathChanged();
}

QString SettingsManager::getD2emuToken() const {
    return m_d2emuToken;
}

void SettingsManager::setD2emuToken(const QString &newD2emuToken) {
    if (m_d2emuToken == newD2emuToken)
        return;
    m_d2emuToken = newD2emuToken;
    emit d2emuTokenChanged();
}

QString SettingsManager::getD2emuUser() const {
    return m_d2emuUser;
}

void SettingsManager::setD2emuUser(const QString &newD2emuUser) {
    if (m_d2emuUser == newD2emuUser)
        return;
    m_d2emuUser = newD2emuUser;
    emit d2emuUserChanged();
}

QString SettingsManager::getLogPath() const {
    return m_logPath;
}

void SettingsManager::setLogPath(const QString &newLogPath) {
    if (m_logPath == newLogPath)
        return;
    m_logPath = newLogPath;
    emit logPathChanged();
}

QString SettingsManager::getLogLevel() const {
    return m_logLevel;
}

void SettingsManager::setLogLevel(const QString &newLogLevel) {
    if (m_logLevel == newLogLevel)
        return;
    m_logLevel = newLogLevel;
    emit logLevelChanged();
}
