#pragma once

#include <QPoint>
#include <QString>

/**
 * Input for the X11 windows of games, sent from outside their wineprefix.
 * wine shows a game through Xwayland on a Wayland session, so it has an X11
 * window there as well.
 */
namespace X11Window
{
enum class Click {
    NotSent,
    SentToActiveWindow,
    SentToInactiveWindow,
};

/**
 * Sends a left click at \a position inside the window titled \a title. The
 * click goes to that window alone and the mouse pointer stays where it is.
 * wine activates a window that is clicked, so an inactive one takes the focus.
 */
Click click(const QString &title, QPoint position);
}
