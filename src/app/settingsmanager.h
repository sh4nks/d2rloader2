#ifndef SETTINGSMANAGER_H
#define SETTINGSMANAGER_H

#include <QObject>

#include <QObject>
#include <QSize>
#include <QString>

/*!
 * \brief The SettingsManager class
 */
class SettingsManager : public QObject
{
    Q_OBJECT

    QSize m_appSize = QSize(1280, 720);
    QSize m_appPosition = QSize(64, 64);
    QString m_appTheme;
    QString m_profilePath;
    QString m_handlePath;
    QString m_gamePath;
    QString m_wineprefixPath;
    QString m_d2emuToken;
    QString m_d2emuUser;
    QString m_logPath;
    QString m_logLevel;

    // Singleton
    static SettingsManager *instance;
    SettingsManager();
    ~SettingsManager();

    bool readSettings();
    bool writeSettings() const;

Q_SIGNALS:
    void firstLaunchChanged();
    void initialSizeChanged();
    void appThemeChanged();
    void appThemeAutoChanged();

    void appSizeChanged();
    void appPositionChanged();
    void profilePathChanged();
    void handlePathChanged();
    void gamePathChanged();
    void wineprefixPathChanged();
    void d2emuTokenChanged();
    void d2emuUserChanged();
    void logPathChanged();
    void logLevelChanged();
    void instanceChanged();

public:
    static SettingsManager *getInstance();

    Q_INVOKABLE void resetSettings();
    QString getAppTheme() const
    {
        return m_appTheme;
    }
    void setAppTheme(const QString &value);
    QSize getAppSize() const;
    void setAppSize(const QSize &newAppSize);
    QSize getAppPosition() const;
    void setAppPosition(const QSize &newAppPosition);
    QString getProfilePath() const;
    void setProfilePath(const QString &newProfilePath);
    QString getHandlePath() const;
    void setHandlePath(const QString &newHandlePath);
    QString getGamePath() const;
    void setGamePath(const QString &newGamePath);
    QString getWineprefixPath() const;
    void setWineprefixPath(const QString &newWineprefixPath);
    QString getD2emuToken() const;
    void setD2emuToken(const QString &newD2emuToken);
    QString getD2emuUser() const;
    void setD2emuUser(const QString &newD2emuUser);
    QString getLogPath() const;
    void setLogPath(const QString &newLogPath);
    QString getLogLevel() const;
    void setLogLevel(const QString &newLogLevel);

private:
    Q_PROPERTY(QSize appSize READ getAppSize WRITE setAppSize NOTIFY appSizeChanged FINAL)
    Q_PROPERTY(QSize appPosition READ getAppPosition WRITE setAppPosition NOTIFY appPositionChanged FINAL)
    Q_PROPERTY(QString appTheme READ getAppTheme WRITE setAppTheme NOTIFY appThemeChanged)
    Q_PROPERTY(QString profilePath READ getProfilePath WRITE setProfilePath NOTIFY profilePathChanged FINAL)
    Q_PROPERTY(QString handlePath READ getHandlePath WRITE setHandlePath NOTIFY handlePathChanged FINAL)
    Q_PROPERTY(QString gamePath READ getGamePath WRITE setGamePath NOTIFY gamePathChanged FINAL)
    Q_PROPERTY(QString wineprefixPath READ getWineprefixPath WRITE setWineprefixPath NOTIFY wineprefixPathChanged FINAL)
    Q_PROPERTY(QString d2emuToken READ getD2emuToken WRITE setD2emuToken NOTIFY d2emuTokenChanged FINAL)
    Q_PROPERTY(QString d2emuUser READ getD2emuUser WRITE setD2emuUser NOTIFY d2emuUserChanged FINAL)
    Q_PROPERTY(QString logPath READ getLogPath WRITE setLogPath NOTIFY logPathChanged FINAL)
    Q_PROPERTY(QString logLevel READ getLogLevel WRITE setLogLevel NOTIFY logLevelChanged FINAL)
};

#endif // SETTINGSMANAGER_H
