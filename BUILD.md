# Building D2RLoader

For installing a released version see the [README](README.md#installation).

## Building

```bash
$ cmake -B build/
$ cmake --build build/
```

### Windows

The Windows build uses the UCRT64 environment of [MSYS2](https://www.msys2.org/) (and pacman therefore :-))

Install MSYS2 (for example with ``winget install MSYS2.MSYS2``), open the _MSYS2 UCRT64_ shell and
install the toolchain and the dependencies:

```bash
$ pacman -S --needed mingw-w64-ucrt-x86_64-{gcc,libgomp,cmake,ninja,qt6-base,qt6-declarative,qt6-svg,extra-cmake-modules,kirigami,kirigami-addons,ki18n,knotifications,kcolorscheme,kiconthemes,kcoreaddons,kconfig,qqc2-desktop-style,breeze,breeze-icons,icoutils}
```

Then build and run it from the same shell, which has the Qt and KDE libraries in its ``PATH``:

```bash
$ cmake -B build/ -G Ninja -DCMAKE_BUILD_TYPE=Release
$ cmake --build build/
$ ./build/bin/d2rloader.exe
```


## Packaging

### Building the AppImage

```bash
$ packaging/appimage/build-appimage.sh
```


### Building the Windows portable

From the MSYS UCRT64 shell run:

```bash
$ packaging/windows/build-zip.sh

# or alternatively
$ make windows
```

The portable zip can be found in:

```bash
build-windows/D2RLoader-<version>-windows-x86_64.zip
```


### Building the Windows installer

The installer needs [Inno Setup](https://jrsoftware.org/isinfo.php) to be installed. Also make sure that you
have it in your path otherwise it won't work.

From the MSYS UCRT64 shell run:

```bash
$ packaging/windows/build-installer.sh

# or alternatively
$ make windows-installer
```

The installer can be found in:

```bash
build-windows/D2RLoader-<version>-windows-x86_64-setup.exe
```


### Building the Flatpak

Needs flatpak-builder and the Flathub remote.

```bash
$ flatpak install --user flathub org.flatpak.Builder
```

Build and install it for the current user:

```bash
$ flatpak run org.flatpak.Builder --user --install-deps-from=flathub --install --force-clean build-flatpak packaging/flatpak/com.someblocks.d2rloader.yml
$ flatpak run com.someblocks.d2rloader
```

Or export it to a local repo and create a single-file bundle:

```bash
$ flatpak run org.flatpak.Builder --user --install-deps-from=flathub --force-clean --repo=repo build-flatpak packaging/flatpak/com.someblocks.d2rloader.yml
$ flatpak build-bundle --runtime-repo=https://dl.flathub.org/repo/flathub.flatpakrepo repo D2RLoader-x86_64.flatpak com.someblocks.d2rloader
$ flatpak install --user D2RLoader-x86_64.flatpak
```
