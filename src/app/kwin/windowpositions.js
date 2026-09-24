// Keeps the game windows of D2RLoader accounts where they were when their
// game last closed. D2RLoader renames every game window to
// "Account (server)" and answers for those titles over D-Bus.
const service = "com.someblocks.d2rloader";
const path = "/WindowPositions";
const iface = "com.someblocks.d2rloader.WindowPositions";
const gameTitle = / \([a-z]+\.actual\.battle\.net\)$/;

// Fullscreen windows sit where their screen is, which is not worth keeping.
function isGameWindow(window) {
    return window.normalWindow && !window.fullScreen && gameTitle.test(window.caption);
}

function isOnScreen(rect) {
    return workspace.screens.some(screen => {
        const area = screen.geometry;
        return rect.x < area.x + area.width && area.x < rect.x + rect.width
            && rect.y < area.y + area.height && area.y < rect.y + rect.height;
    });
}

function restore(window) {
    if (!isGameWindow(window)) {
        return;
    }
    const caption = window.caption;
    callDBus(service, path, iface, "windowPosition", caption, (found, x, y) => {
        // The window may have closed or changed while D2RLoader answered.
        if (!found || window.deleted || !isGameWindow(window) || window.caption !== caption) {
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
    if (!isGameWindow(window)) {
        return;
    }
    const geometry = window.frameGeometry;
    callDBus(service, path, iface, "saveWindowPosition", window.caption, Math.round(geometry.x), Math.round(geometry.y));
}

// The game window is renamed a while after it shows up.
function track(window) {
    window.captionChanged.connect(() => restore(window));
}

// Windows that are open already are left where they are.
workspace.stackingOrder.forEach(track);
workspace.windowAdded.connect(window => {
    track(window);
    restore(window);
});
workspace.windowRemoved.connect(save);
