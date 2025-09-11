#ifndef D2RLOADER_H
#define D2RLOADER_H

#include <QObject>
#include <QUrl>
#include <QColor>
#include <QString>
#include <QSettings>
#include <QStringList>


class D2RLoader : public QObject
{
    Q_OBJECT

    QString m_appPath;

    // Singleton
    static D2RLoader *instance;
    D2RLoader();
    ~D2RLoader();

public:
    static D2RLoader *getInstance();

    // app info

    static Q_INVOKABLE QString appName();
    static Q_INVOKABLE QString appVersion();

    static Q_INVOKABLE QString appBuildDate();
    static Q_INVOKABLE QString appBuildDateTime();
    static Q_INVOKABLE QString appBuildMode();

    // Qt info
    static Q_INVOKABLE QString qtVersion();

    Q_INVOKABLE QString getAppPath() const { return m_appPath; }
    void setAppPath(const QString &value);

    static Q_INVOKABLE void appExit();

    static Q_INVOKABLE bool isOsThemeDark();

    QSettings appSettings;
    QSettings profileSettings;

    static void registerSettingFormats();
    bool readAppSettings(QIODevice &device, QSettings::SettingsMap &map);
    bool writeAppSettings(QIODevice &device, const QSettings::SettingsMap &map);
    bool readProfileSettings(QIODevice &device, QSettings::SettingsMap &map);
    bool writeProfileSettings(QIODevice &device, const QSettings::SettingsMap &map);
};

#endif
