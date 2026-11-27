// Keeps the game windows of D2RLoader accounts where they were when their
// game last closed. D2RLoader renames every game window to a title of its
// own and answers for those titles over D-Bus. Also gives the focus back
// that a game window takes when D2RLoader clicks into it to skip the intro.
const service = "com.someblocks.d2rloader";
const path = "/WindowPositions";
const iface = "com.someblocks.d2rloader.WindowPositions";
const gameTitle = "Diablo II: Resurrected";
// wine names the class of an X11 window after its program.
const gameProgram = "d2r.exe";

// Windows that showed up as the game, by its own title or by its program. A
// window renamed before it showed up only has the program left.
const games = new Set();
// Windows that were open already may be games an earlier script saw start.
const openBefore = new Set();

// Fullscreen and maximized windows sit where their screen is, which is not
// worth keeping.
function isWindowed(window) {
    return window.normalWindow && !window.fullScreen && !window.maximizeMode;
}

function isOnScreen(rect) {
    return workspace.screens.some(screen => {
        const area = screen.geometry;
        return rect.x < area.x + area.width && area.x < rect.x + rect.width
            && rect.y < area.y + area.height && area.y < rect.y + rect.height;
    });
}

function restore(window) {
    const id = window.internalId.toString();
    // KWin numbers windows that share a caption: "Diablo II: Resurrected <2>".
    if (window.caption.startsWith(gameTitle) || window.resourceName.toLowerCase() === gameProgram) {
        games.add(id);
    }
    if (!games.has(id) || !isWindowed(window)) {
        return;
    }
    const caption = window.caption;
    callDBus(service, path, iface, "windowPosition", caption, (found, x, y) => {
        // The window may have closed or changed while D2RLoader answered.
        if (!found || window.deleted || !isWindowed(window) || window.caption !== caption) {
            return;
        }
        const geometry = window.frameGeometry;
        const target = {x: x, y: y, width: geometry.width, height: geometry.height};
        if (isOnScreen(target)) {
            window.frameGeometry = target;
        }
    });
}

function save(window) {
    const id = window.internalId.toString();
    const isGame = games.delete(id);
    const wasOpenBefore = openBefore.delete(id);
    if (!(isGame || wasOpenBefore) || !isWindowed(window)) {
        return;
    }
    // D2RLoader ignores the titles that belong to none of its accounts.
    const geometry = window.frameGeometry;
    callDBus(service, path, iface, "saveWindowPosition", window.caption, Math.round(geometry.x), Math.round(geometry.y));
}

// The game window is renamed a while after it shows up.
function track(window) {
    window.captionChanged.connect(() => restore(window));
}

// The window the user gave the focus to. wine activates a window that is
// clicked, so each click that skips an intro pulls the focus to its game.
let focused = workspace.activeWindow;

function keepFocus(window) {
    if (!window || !games.has(window.internalId.toString())) {
        focused = window;
        return;
    }
    // Without what KWin appends to the title, like " (Not Responding)".
    callDBus(service, path, iface, "tookFocusForSkipIntro", window.captionNormal, expected => {
        if (workspace.activeWindow !== window) {
            return;
        }
        if (expected && focused && !focused.deleted) {
            workspace.activeWindow = focused;
        } else {
            focused = window;
        }
    });
}

workspace.windowActivated.connect(keepFocus);
workspace.windowRemoved.connect(window => {
    if (focused === window) {
        focused = null;
    }
});

// Windows that are open already are left where they are.
workspace.stackingOrder.forEach(window => {
    openBefore.add(window.internalId.toString());
    track(window);
});
workspace.windowAdded.connect(window => {
    track(window);
    restore(window);
});
workspace.windowRemoved.connect(save);
