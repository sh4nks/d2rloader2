#pragma once

#include <QDBusVirtualObject>

/**
 * Keeps the game windows of accounts where they were when their game last
 * closed, on KDE Plasma. A KWin script moves each game window once it carries
 * Profile::windowTitle() and reports where it was when it closes, both through
 * the D-Bus object registered here.
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

    QString introspect(const QString &path) const override;
    bool handleMessage(const QDBusMessage &message, const QDBusConnection &connection) override;

private:
    void loadScript();

    bool m_scriptLoaded = false;
};
