# Changelog


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
