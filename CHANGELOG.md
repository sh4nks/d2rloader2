# Changelog


## Unreleased

### Added

- The token field of an account accepts the whole Battle.net login URL and takes the token out of it
- Accounts and launch sequences can be started from the command line with `--launch` and `--sequence`,
  and `--minimized` starts D2RLoader hidden in the system tray or minimized. A D2RLoader that is
  running already takes over the launch.
- Launch sequences can start all their accounts at once with the new _Launch in Parallel_ setting (Linux only)


## 0.6.0

Released on Nov 18th, 2026

### Added

- The AppImage can update itself now with the provided .zsync file

### Changed

- Improve the main window UI by making it more compact

### Fixed

- In case there are more Terror Zones to display, we now show a scrollbar


## 0.5.2

Released on Nov 13th, 2026

### Fixed

- The game window icon was not read from D2R.exe when running from the AppImage.
- No information about the wait time was shown while umu-run updates its environment (Steam Runtime, Proton).


## 0.5.1

Released on Nov 12th, 2026

No changes. 0.5.0 was never published because of a GitHub Actions outage.


## 0.5.0

Released on Nov 11th, 2026

Nothing worthy to describe here


## 0.4.0

Released on Nov 10th, 2026

### Added

- Add an auto updater for Windows


## 0.3.0

Released on Nov 6th, 2026

### Added

- Add an update checker for non distribution and flatpak packages.

### Fixed

- The AppImage aborted on the missing platform plugin, because Qt never read `usr/bin/qt.conf`.
- File and folder dialogs in the AppImage go through xdg-desktop-portal.
- The settings page file and folder dialogs reopened at their first location rather than the configured path.
