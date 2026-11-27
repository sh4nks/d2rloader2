#include "x11window.h"
#include "../core/logging.h"

#include <QList>

#include <xcb/xcb.h>

#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <memory>

namespace
{
struct Disconnect {
    void operator()(xcb_connection_t *connection) const
    {
        xcb_disconnect(connection);
    }
};

struct Free {
    void operator()(void *reply) const
    {
        std::free(reply);
    }
};

template<typename T>
using Reply = std::unique_ptr<T, Free>;

/**
 * Properties are read in units of 4 bytes. This covers every title and a
 * list of 4096 windows.
 */
const uint32_t PropertyLength = 4096;

/**
 * wine names the class of an X11 window after its program.
 */
const QByteArray GameProgram = "d2r.exe";

xcb_atom_t atom(xcb_connection_t *connection, const char *name)
{
    const Reply<xcb_intern_atom_reply_t> reply(
        xcb_intern_atom_reply(connection, xcb_intern_atom(connection, true, static_cast<uint16_t>(strlen(name)), name), nullptr));
    return reply ? reply->atom : xcb_atom_t{XCB_ATOM_NONE};
}

QByteArray property(xcb_connection_t *connection, xcb_window_t window, xcb_atom_t name)
{
    const Reply<xcb_get_property_reply_t> reply(
        xcb_get_property_reply(connection, xcb_get_property(connection, false, window, name, XCB_GET_PROPERTY_TYPE_ANY, 0, PropertyLength), nullptr));
    if (!reply) {
        return {};
    }
    return QByteArray(static_cast<const char *>(xcb_get_property_value(reply.get())), xcb_get_property_value_length(reply.get()));
}

/**
 * The windows or atoms a property lists.
 */
QList<uint32_t> ids(const QByteArray &property)
{
    QList<uint32_t> list(property.size() / qsizetype(sizeof(uint32_t)));
    std::copy_n(property.constData(), list.size() * qsizetype(sizeof(uint32_t)), reinterpret_cast<char *>(list.data()));
    return list;
}
}

X11Window::Click X11Window::click(const QString &title, QPoint position)
{
    const std::unique_ptr<xcb_connection_t, Disconnect> connection(xcb_connect(nullptr, nullptr));
    xcb_connection_t *x = connection.get();
    if (xcb_connection_has_error(x)) {
        qCDebug(LOG_GAME) << "no native click into the window" << title << "- there is no X11 display to connect to";
        return Click::NotSent;
    }
    const xcb_window_t root = xcb_setup_roots_iterator(xcb_get_setup(x)).data->root;

    const QByteArray name = title.toUtf8();
    const xcb_atom_t nameAtom = atom(x, "_NET_WM_NAME");
    const QList<xcb_window_t> clients = ids(property(x, root, atom(x, "_NET_CLIENT_LIST")));
    const auto match = std::ranges::find_if(clients, [x, nameAtom, &name](xcb_window_t client) {
        return property(x, client, nameAtom) == name && property(x, client, XCB_ATOM_WM_CLASS).toLower().split('\0').contains(GameProgram);
    });
    if (match == clients.end()) {
        qCDebug(LOG_GAME) << "no native click into the window" << title << "- no X11 window of the game carries that title";
        return Click::NotSent;
    }
    const xcb_window_t window = *match;

    // wine looks for the clicked window where it is on the screen, and a minimized one is not there.
    if (ids(property(x, window, atom(x, "_NET_WM_STATE"))).contains(atom(x, "_NET_WM_STATE_HIDDEN"))) {
        qCDebug(LOG_GAME) << "no native click into the window" << title << "- it is minimized";
        return Click::NotSent;
    }

    // wine works out which of its windows is clicked from the position on the root window.
    const Reply<xcb_translate_coordinates_reply_t> onRoot(
        xcb_translate_coordinates_reply(x,
                                        xcb_translate_coordinates(x, window, root, static_cast<int16_t>(position.x()), static_cast<int16_t>(position.y())),
                                        nullptr));
    if (!onRoot) {
        qCDebug(LOG_GAME) << "no native click into the window" << title << "- it is gone";
        return Click::NotSent;
    }
    const bool active = ids(property(x, root, atom(x, "_NET_ACTIVE_WINDOW"))).value(0) == window;

    xcb_button_press_event_t event{};
    event.response_type = XCB_BUTTON_PRESS;
    event.detail = XCB_BUTTON_INDEX_1;
    event.time = XCB_CURRENT_TIME;
    event.root = root;
    event.event = window;
    event.root_x = onRoot->dst_x;
    event.root_y = onRoot->dst_y;
    event.event_x = static_cast<int16_t>(position.x());
    event.event_y = static_cast<int16_t>(position.y());
    event.same_screen = 1;
    xcb_send_event(x, true, window, XCB_EVENT_MASK_BUTTON_PRESS, reinterpret_cast<const char *>(&event));
    event.response_type = XCB_BUTTON_RELEASE;
    event.state = XCB_BUTTON_MASK_1;
    xcb_send_event(x, true, window, XCB_EVENT_MASK_BUTTON_RELEASE, reinterpret_cast<const char *>(&event));

    // The server answers once it has handled the events before it.
    const Reply<xcb_get_input_focus_reply_t> handled(xcb_get_input_focus_reply(x, xcb_get_input_focus(x), nullptr));

    return active ? Click::SentToActiveWindow : Click::SentToInactiveWindow;
}
