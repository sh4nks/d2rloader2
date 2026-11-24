// Keeps the game windows of D2RLoader accounts where they were when their
// game last closed. D2RLoader renames every game window to a title of its
// own and answers for those titles over D-Bus.
const service = "com.someblocks.d2rloader";
const path = "/WindowPositions";
const iface = "com.someblocks.d2rloader.WindowPositions";
const gameTitle = "Diablo II: Resurrected";

// Windows that showed up with the game's own title, which is all that tells
// them apart once they are renamed.
const games = new Set();
// Windows that were open already may be games an earlier script saw start.
const openBefore = new Set();

// Fullscreen windows sit where their screen is, which is not worth keeping.
function isWindowed(window) {
    return window.normalWindow && !window.fullScreen;
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
    if (window.caption.startsWith(gameTitle)) {
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
