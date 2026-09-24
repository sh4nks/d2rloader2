#!/usr/bin/env bash
set -euo pipefail

root=$(realpath "$(dirname "$0")/../..")
work=${WORK_DIR:-$root/build-appimage}
tools=$work/tools
appdir=$work/AppDir
qmake=${QMAKE:-qmake6}
qt_plugins=$("$qmake" -query QT_INSTALL_PLUGINS)

fetch() {
    if [[ ! -x $tools/$1 ]]; then
        curl -fL --create-dirs -o "$tools/$1" "$2"
        chmod +x "$tools/$1"
    fi
}

fetch linuxdeploy-x86_64.AppImage https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage
fetch linuxdeploy-plugin-qt-x86_64.AppImage https://github.com/linuxdeploy/linuxdeploy-plugin-qt/releases/download/continuous/linuxdeploy-plugin-qt-x86_64.AppImage

cmake -B "$work/build" -S "$root" -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr -DBUILD_TESTING=OFF
cmake --build "$work/build"
rm -rf "$appdir"
DESTDIR=$appdir cmake --install "$work/build"

# Loaded by name at runtime, so linuxdeploy-plugin-qt does not find them.
runtime_plugins=(
    kf6/kirigami/platform/org.kde.desktop.so
    kiconthemes6/iconengines/KIconEnginePlugin.so
    styles/breeze6.so
)
for plugin in "${runtime_plugins[@]}"; do
    install -Dm755 "$qt_plugins/$plugin" "$appdir/usr/plugins/$plugin"
done

# QQuickStyle::setStyle() picks the style by name, so no QML file imports it.
mkdir -p "$work/qml-imports"
printf 'import QtQuick\nimport org.kde.desktop\nItem {}\n' >"$work/qml-imports/Style.qml"

# The notifyrc and the icons are looked up through XDG_DATA_DIRS.
mkdir -p "$appdir/apprun-hooks"
cat >"$appdir/apprun-hooks/xdg-data-dirs.sh" <<'EOF'
export XDG_DATA_DIRS="$APPDIR/usr/share:${XDG_DATA_DIRS:-/usr/local/share:/usr/share}"
EOF

# linuxdeploy-plugin-qt copies every plugin of a module and gives up on one
# whose optional dependency is not installed (kimageformats' jxr plugin), so it
# is pointed at a copy of the plugin directory without those.
plugins=$work/qt-plugins
rm -rf "$plugins"
while IFS= read -r -d '' plugin; do
    if ! ldd "$plugin" | grep -q 'not found'; then
        install -Dm755 "$plugin" "$plugins/${plugin#"$qt_plugins"/}"
    fi
done < <(find "$qt_plugins" -name '*.so' -print0)

cat >"$work/qmake" <<EOF
#!/bin/sh
"$(command -v "$qmake")" "\$@" | sed 's|^QT_INSTALL_PLUGINS:.*|QT_INSTALL_PLUGINS:$plugins|'
EOF
chmod +x "$work/qmake"

version=$(sed -n 's/^CMAKE_PROJECT_VERSION:STATIC=//p' "$work/build/CMakeCache.txt")

export APPIMAGE_EXTRACT_AND_RUN=1
# The strip bundled with linuxdeploy cannot handle .relr.dyn sections.
export NO_STRIP=1
export QMAKE=$work/qmake
export QML_SOURCES_PATHS="$root/src:$work/qml-imports"
export QML_MODULES_PATHS="$work/build/src"
export EXTRA_QT_MODULES="svg;waylandcompositor"
export EXTRA_PLATFORM_PLUGINS="libqwayland.so"
export LINUXDEPLOY_OUTPUT_VERSION=$version
export LDAI_OUTPUT="$work/D2RLoader-$version-x86_64.AppImage"

"$tools/linuxdeploy-x86_64.AppImage" \
    --appdir "$appdir" \
    --desktop-file "$appdir/usr/share/applications/com.someblocks.d2rloader.desktop" \
    --deploy-deps-only "$appdir/usr/plugins/kf6" \
    --deploy-deps-only "$appdir/usr/plugins/kiconthemes6" \
    --deploy-deps-only "$appdir/usr/plugins/styles" \
    --plugin qt \
    --output appimage

echo "$LDAI_OUTPUT"
