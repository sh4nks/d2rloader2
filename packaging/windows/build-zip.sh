#!/usr/bin/env bash
# Builds D2RLoader and packs it with everything it needs at runtime into a
# portable zip. Run it from the MSYS2 UCRT64 shell with the packages listed in
# the README's "Windows" section and zip installed.
set -euo pipefail

if [[ ${MSYSTEM:-} != UCRT64 ]]; then
    echo "$0: run this from the MSYS2 UCRT64 shell" >&2
    exit 1
fi

# Only a login shell sets MINGW_PREFIX and puts the MSYS2 tools first. With
# MSYSTEM merely exported from elsewhere, find would be the one of Windows.
msys_bin=/ucrt64/bin
PATH=$msys_bin:/usr/bin:$PATH

root=$(realpath "$(dirname "$0")/../..")
work=${WORK_DIR:-$root/build-windows}
app=$work/D2RLoader
qt_plugins=$(cygpath -u "$(qmake6 -query QT_INSTALL_PLUGINS)")
qt_qml=$(cygpath -u "$(qmake6 -query QT_INSTALL_QML)")
qt_libexecs=$(cygpath -u "$(qmake6 -query QT_HOST_LIBEXECS)")

cmake -B "$work/build" -S "$root" -G Ninja -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=OFF \
    -DCMAKE_INSTALL_PREFIX="$(cygpath -m "$work/install")"
cmake --build "$work/build"
rm -rf "$work/install" "$app"
cmake --install "$work/build"

# On Windows KDEInstallDirs puts the data files under bin/data, where
# QStandardPaths finds them next to the executable, so bin/ is the whole app.
mkdir -p "$app"
cp -a "$work/install/bin/." "$app/"
[[ -f $app/data/knotifications6/d2rloader.notifyrc ]]
# Only meaningful to Linux desktops.
rm -rf "$app/data/applications" "$app/data/metainfo"

# QQuickStyle::setStyle() picks the style by name, so no QML file imports it.
mkdir -p "$work/qml-imports"
printf 'import QtQuick\nimport org.kde.desktop\nItem {}\n' >"$work/qml-imports/Style.qml"

# The OpenSSL TLS backend is left out, so HTTPS goes through the Schannel one.
windeployqt=$(command -v windeployqt-qt6 || command -v windeployqt)
"$windeployqt" --release --qmldir "$root/src" --qmldir "$work/qml-imports" \
    --no-translations --no-system-d3d-compiler --no-system-dxc-compiler \
    "$app/d2rloader.exe"

# Qt derives its paths from where Qt6Core.dll is, now the app directory, so
# without this the modules windeployqt put into qml/ are not an import path.
# The QML engine then falls back to the qmldir copies compiled into the Qt
# libraries, next to which it cannot find the plugins.
cat >"$app/qt.conf" <<'QTCONF'
[Paths]
Prefix = .
Plugins = .
QmlImports = qml
QTCONF

# Loaded by name at runtime, so windeployqt does not find them.
runtime_plugins=(
    kf6/kirigami/platform/org.kde.desktop.dll
    kiconthemes6/iconengines/KIconEnginePlugin.dll
    styles/breeze6.dll
)
for plugin in "${runtime_plugins[@]}"; do
    install -Dm644 "$qt_plugins/$plugin" "$app/$plugin"
done

# The KDE Frameworks QML modules: the ones the app imports with everything
# below them, plus every org.kde module qmlimportscanner reaches from the app's
# QML and the QQC2 style.
kf_modules=(
    org/kde/kirigami
    org/kde/kirigamiaddons
    org/kde/desktop
    org/kde/qqc2desktopstyle
    org/kde/ki18n
    org/kde/coreaddons
    org/kde/config
)
while IFS= read -r module; do
    kf_modules+=("$module")
done < <("$qt_libexecs/qmlimportscanner" -rootPath "$root/src" -rootPath "$work/qml-imports" -importPath "$qt_qml" \
    | sed -n 's/^[[:space:]]*"relativePath": "\(org\/kde\/[^"]*\)".*/\1/p')
for module in "${kf_modules[@]}"; do
    [[ -d $qt_qml/$module ]] || continue
    mkdir -p "$app/qml/$module"
    cp -a "$qt_qml/$module/." "$app/qml/$module/"
done

# Every DLL from the MSYS2 bin directory that a binary in the app imports,
# directly or through another one. System DLLs are not in that directory, so
# they are skipped. The import tables are read instead of running ldd, which
# loads each binary to list them.
declare -A msys_dlls=()
for dll in "$msys_bin"/*.dll; do
    name=${dll##*/}
    msys_dlls[${name,,}]=$name
done

copy_dependencies() {
    local queue=("$@") binary dep name
    while ((${#queue[@]})); do
        binary=${queue[0]}
        queue=("${queue[@]:1}")
        while IFS= read -r dep; do
            name=${msys_dlls[${dep,,}]:-}
            [[ -n $name && ! -e $app/$name ]] || continue
            cp "$msys_bin/$name" "$app/"
            queue+=("$app/$name")
        done < <(objdump -p "$binary" | sed -n 's/^[[:space:]]*DLL Name: //p')
    done
}

mapfile -t binaries < <(find "$app" -type f \( -name '*.exe' -o -name '*.dll' \))
copy_dependencies "${binaries[@]}"

version=$(sed -n 's/^CMAKE_PROJECT_VERSION:STATIC=//p' "$work/build/CMakeCache.txt")
zip_file=$work/D2RLoader-$version-windows-x86_64.zip
rm -f "$zip_file"
(cd "$work" && zip -qr "$zip_file" D2RLoader)

echo "$zip_file"
