#!/usr/bin/env bash
# Packs the folder build-zip.sh staged into a per-user installer with Inno
# Setup. Run it from the MSYS2 UCRT64 shell after build-zip.sh. Inno Setup is
# not an MSYS2 package: install it from https://jrsoftware.org (the GitHub
# Windows runner has it); outside its default folder, point ISCC at ISCC.exe.
set -euo pipefail

root=$(realpath "$(dirname "$0")/../..")
work=${WORK_DIR:-$root/build-windows}
iscc=${ISCC:-}
if [[ -z $iscc ]]; then
    for dir in "/c/Program Files/Inno Setup 7" "/c/Program Files (x86)/Inno Setup 7" "/c/Program Files (x86)/Inno Setup 6"; do
        if [[ -f $dir/ISCC.exe ]]; then
            iscc=$dir/ISCC.exe
            break
        fi
    done
fi

if [[ ! -f $work/D2RLoader/d2rloader.exe ]]; then
    echo "$0: no staged app in $work, run packaging/windows/build-zip.sh first" >&2
    exit 1
fi
if [[ ! -f $iscc ]]; then
    echo "$0: Inno Setup not found, set ISCC to its ISCC.exe" >&2
    exit 1
fi

version=$(sed -n 's/^CMAKE_PROJECT_VERSION:STATIC=//p' "$work/build/CMakeCache.txt")
# MSYS2 would otherwise rewrite the /D options as paths.
MSYS2_ARG_CONV_EXCL='*' "$iscc" /Q "/DAppVersion=$version" "/DStageDir=$(cygpath -w "$work")" \
    "$(cygpath -w "$root/packaging/windows/d2rloader.iss")"

echo "$work/D2RLoader-$version-windows-x86_64-setup.exe"
