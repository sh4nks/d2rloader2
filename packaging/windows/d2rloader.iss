; Packs the folder build-zip.sh stages into a per-user installer. Compiled by
; build-installer.sh, which passes AppVersion and StageDir.

[Setup]
AppId={{59BA45B2-25D4-49E8-B8FC-E48986E500AB}
AppName=D2RLoader
AppVersion={#AppVersion}
AppPublisher=someblocks
AppPublisherURL=https://github.com/sh4nks/d2rloader2
AppSupportURL=https://github.com/sh4nks/d2rloader2/issues
AppUpdatesURL=https://github.com/sh4nks/d2rloader2/releases
VersionInfoVersion={#AppVersion}
; Installs for the current user only, into %LOCALAPPDATA%\Programs, so no
; administrator rights are needed.
PrivilegesRequired=lowest
DefaultDirName={autopf}\D2RLoader
DisableProgramGroupPage=yes
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
SetupIconFile={#StageDir}\build\src\app\D2RLOADER_ICON.ico
UninstallDisplayIcon={app}\d2rloader.exe
OutputDir={#StageDir}
OutputBaseFilename=D2RLoader-{#AppVersion}-windows-x86_64-setup
SolidCompression=yes
WizardStyle=modern

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked

[InstallDelete]
; The directories Qt and KDE Frameworks scan at runtime are replaced whole, so
; no plugin or QML module of the previous version stays behind.
Type: filesandordirs; Name: "{app}\data"
Type: filesandordirs; Name: "{app}\generic"
Type: filesandordirs; Name: "{app}\iconengines"
Type: filesandordirs; Name: "{app}\imageformats"
Type: filesandordirs; Name: "{app}\kf6"
Type: filesandordirs; Name: "{app}\kiconthemes6"
Type: filesandordirs; Name: "{app}\networkinformation"
Type: filesandordirs; Name: "{app}\platforms"
Type: filesandordirs; Name: "{app}\qml"
Type: filesandordirs; Name: "{app}\qmltooling"
Type: filesandordirs; Name: "{app}\styles"
Type: filesandordirs; Name: "{app}\tls"

[Files]
Source: "{#StageDir}\D2RLoader\*"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs createallsubdirs

[Icons]
Name: "{autoprograms}\D2RLoader"; Filename: "{app}\d2rloader.exe"
Name: "{autodesktop}\D2RLoader"; Filename: "{app}\d2rloader.exe"; Tasks: desktopicon

[Run]
Filename: "{app}\d2rloader.exe"; Description: "{cm:LaunchProgram,D2RLoader}"; Flags: nowait postinstall skipifsilent
