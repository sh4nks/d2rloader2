#pragma once

#include <QObject>
#include <qqmlintegration.h>

/**
 * What the desktop session D2RLoader runs in offers to it.
 */
class DesktopSession : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

    Q_PROPERTY(bool isPlasma READ isPlasma CONSTANT)

public:
    using QObject::QObject;

    /**
     * Whether the session is KDE Plasma, where a KWin script looks after the
     * game windows.
     */
    static bool isPlasma()
    {
        return qEnvironmentVariable("XDG_CURRENT_DESKTOP").split(u':').contains(QStringLiteral("KDE"));
    }
};
