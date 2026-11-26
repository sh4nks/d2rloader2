#pragma once

#include <QDBusVirtualObject>
#include <QDeadlineTimer>
#include <QHash>

/**
 * Keeps the game windows of accounts where they were when their game last
 * closed, on KDE Plasma. A KWin script moves each game window once it carries
 * Profile::windowTitle() and reports where it was when it closes, both through
 * the D-Bus object registered here. The script also gives the focus back that
 * a game window takes when the intro is skipped.
 *
 * Handled as a virtual object because the script passes plain JavaScript
 * numbers, which arrive as int or double depending on their value.
 */
class KWinWindowPositions : public QDBusVirtualObject
{
    Q_OBJECT

public:
    /**
     * Whether the session is KDE Plasma, where KWin moves the game windows.
     */
    static bool isSupported();

    explicit KWinWindowPositions(QObject *parent = nullptr);
    ~KWinWindowPositions() override;

    /**
     * The window titled \a title was clicked to skip its intro and takes the
     * focus for it. When it does so before \a deadline, the script hands the
     * focus back to the window that had it.
     */
    void expectFocusSteal(const QString &title, QDeadlineTimer deadline);

    QString introspect(const QString &path) const override;
    bool handleMessage(const QDBusMessage &message, const QDBusConnection &connection) override;

private:
    void loadScript();

    bool m_scriptLoaded = false;
    QHash<QString, QDeadlineTimer> m_expectedFocusSteals;
};
