pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import QtQuick.Dialogs
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard
import org.kde.ki18n
import com.someblocks.d2rloader.core

FormCard.FormCardPage {
    id: root

    title: KI18n.i18nc("@title", "Application Settings")

    readonly property bool isWindows: Qt.platform.os === "windows"

    // FolderDialog applies currentFolder only on its first show, so a reopen
    // needs selectedFolder as well.
    function browseFolderFrom(dialog, path) {
        const folder = Paths.folderUrl(path);
        if (folder.toString().length > 0) {
            dialog.currentFolder = folder;
            dialog.selectedFolder = folder;
        }
        dialog.open();
    }

    function browseFileFrom(dialog, path) {
        const folder = Paths.folderUrl(path);
        if (folder.toString().length > 0) {
            dialog.currentFolder = folder;
        }
        dialog.open();
    }

    FolderDialog {
        id: gamePathDialog
        title: KI18n.i18nc("@title:window", "Select Game Directory")
        onAccepted: {
            D2RLoaderConfig.gamePath = Paths.toLocalFile(selectedFolder);
            D2RLoaderConfig.save();
        }
    }

    FileDialog {
        id: accountSettingsDialog
        title: KI18n.i18nc("@title:window", "Select Account Settings File")
        nameFilters: [KI18n.i18nc("@item", "JSON files (*.json)"), KI18n.i18nc("@item", "All files (*)")]
        onAccepted: {
            D2RLoaderConfig.profilePath = Paths.toLocalFile(selectedFile);
            D2RLoaderConfig.save();
        }
    }

    FolderDialog {
        id: winePrefixDialog
        title: KI18n.i18nc("@title:window", "Select Wineprefix Directory")
        onAccepted: {
            D2RLoaderConfig.wineprefixPath = Paths.toLocalFile(selectedFolder);
            D2RLoaderConfig.save();
        }
    }

    FolderDialog {
        id: protonPathDialog
        title: KI18n.i18nc("@title:window", "Select Proton Runtime Directory")
        onAccepted: {
            D2RLoaderConfig.protonPath = Paths.toLocalFile(selectedFolder);
            D2RLoaderConfig.save();
        }
    }

    FolderDialog {
        id: logPathDialog
        title: KI18n.i18nc("@title:window", "Select Log Directory")
        onAccepted: {
            D2RLoaderConfig.logPath = Paths.toLocalFile(selectedFolder);
            D2RLoaderConfig.save();
        }
    }

    FormCard.FormHeader {
        title: KI18n.i18nc("@title:group", "General")
    }

    FormCard.FormCard {
        FormCard.AbstractFormDelegate {
            contentItem: ColumnLayout {
                spacing: Kirigami.Units.smallSpacing
                QQC2.Label {
                    text: KI18n.i18nc("@label", "Game Path")
                }
                RowLayout {
                    QQC2.TextField {
                        id: gamePathField
                        Layout.fillWidth: true
                        text: D2RLoaderConfig.gamePath
                        onEditingFinished: {
                            D2RLoaderConfig.gamePath = text;
                            D2RLoaderConfig.save();
                        }
                        placeholderText: KI18n.i18nc("@info:placeholder", "Path to Diablo II Resurrected...")
                    }
                    QQC2.Button {
                        icon.name: "folder-open"
                        onClicked: root.browseFolderFrom(gamePathDialog, D2RLoaderConfig.gamePath)
                        QQC2.ToolTip.visible: hovered
                        QQC2.ToolTip.text: KI18n.i18nc("@info:tooltip", "Browse...")
                    }
                }
            }
        }

        FormCard.FormDelegateSeparator {}

        FormCard.AbstractFormDelegate {
            contentItem: ColumnLayout {
                spacing: Kirigami.Units.smallSpacing
                QQC2.Label {
                    text: KI18n.i18nc("@label", "Account Settings File")
                }
                RowLayout {
                    QQC2.TextField {
                        id: accountSettingsField
                        Layout.fillWidth: true
                        text: D2RLoaderConfig.profilePath
                        onEditingFinished: {
                            D2RLoaderConfig.profilePath = text;
                            D2RLoaderConfig.save();
                        }
                        placeholderText: KI18n.i18nc("@info:placeholder", "Path to accounts.json...")
                    }
                    QQC2.Button {
                        icon.name: "document-open"
                        onClicked: root.browseFileFrom(accountSettingsDialog, D2RLoaderConfig.profilePath)
                        QQC2.ToolTip.visible: hovered
                        QQC2.ToolTip.text: KI18n.i18nc("@info:tooltip", "Browse...")
                    }
                }
            }
        }

        FormCard.FormDelegateSeparator {}

        FormCard.FormCheckDelegate {
            text: KI18n.i18nc("@label", "Show Menu Bar")
            description: KI18n.i18nc("@info:label", "Ctrl+M toggles it.")
            checked: D2RLoaderConfig.showMenuBar
            onToggled: {
                D2RLoaderConfig.showMenuBar = checked;
                D2RLoaderConfig.save();
            }
        }

        FormCard.FormDelegateSeparator {}

        FormCard.FormCheckDelegate {
            text: KI18n.i18nc("@label", "Show Info Panel")
            checked: D2RLoaderConfig.showInfoPanel
            onToggled: {
                D2RLoaderConfig.showInfoPanel = checked;
                D2RLoaderConfig.save();
            }
        }

        FormCard.FormDelegateSeparator {}

        FormCard.FormCheckDelegate {
            text: KI18n.i18nc("@label", "Close to System Tray")
            checked: D2RLoaderConfig.closeToTray
            onToggled: {
                D2RLoaderConfig.closeToTray = checked;
                D2RLoaderConfig.save();
            }
        }
    }

    FormCard.FormHeader {
        title: KI18n.i18nc("@title:group", "Advanced Settings")
    }

    FormCard.FormCard {
        FormCard.FormTextFieldDelegate {
            id: windowTitleField
            label: KI18n.i18nc("@label", "Window Title")
            description: KI18n.i18nc("@info:label", "{name} is the account name, {region} the region (EU, US, KR) and {server} the server address.")
            text: D2RLoaderConfig.windowTitleTemplate
            placeholderText: D2RLoaderConfig.defaultWindowTitleTemplateValue
            onEditingFinished: {
                D2RLoaderConfig.windowTitleTemplate = text.trim().length > 0 ? text : D2RLoaderConfig.defaultWindowTitleTemplateValue;
                D2RLoaderConfig.save();
                text = D2RLoaderConfig.windowTitleTemplate;
            }
            status: Kirigami.MessageType.Warning
            statusMessage: text.includes("{name}") ? "" : KI18n.i18nc("@info", "Window titles have to be unique among running games. Without {name} they are the same, and a number is appended.")
        }

        FormCard.FormDelegateSeparator {}

        // The click is sent to the X11 window of the game, and a KWin script
        // gives the focus back that the window takes for it. A game on the
        // Wayland driver of wine has no X11 window, and d2rreg clicks into it
        // from inside its wineprefix.
        FormCard.FormCheckDelegate {
            visible: Qt.platform.os === "linux"
            enabled: DesktopSession.isPlasma
            text: KI18n.i18nc("@label", "Skip Intro")
            description: KI18n.i18nc("@info:label", "Clicks into the game window while it starts. Only on KDE Plasma.")
            checked: enabled && D2RLoaderConfig.skipIntro
            onToggled: {
                D2RLoaderConfig.skipIntro = checked;
                D2RLoaderConfig.save();
            }
        }

        FormCard.FormDelegateSeparator {
            visible: Qt.platform.os === "linux"
        }

        // The game runs natively on Windows, without a wineprefix or Proton.
        FormCard.AbstractFormDelegate {
            visible: !root.isWindows
            contentItem: ColumnLayout {
                spacing: Kirigami.Units.smallSpacing
                QQC2.Label {
                    text: KI18n.i18nc("@label", "Wineprefix Location")
                }
                RowLayout {
                    QQC2.TextField {
                        id: winePrefixField
                        Layout.fillWidth: true
                        text: D2RLoaderConfig.wineprefixPath
                        onEditingFinished: {
                            D2RLoaderConfig.wineprefixPath = text;
                            D2RLoaderConfig.save();
                        }
                        placeholderText: KI18n.i18nc("@info:placeholder", "Path to wineprefixes directory...")
                    }
                    QQC2.Button {
                        icon.name: "folder-open"
                        onClicked: root.browseFolderFrom(winePrefixDialog, D2RLoaderConfig.wineprefixPath)
                        QQC2.ToolTip.visible: hovered
                        QQC2.ToolTip.text: KI18n.i18nc("@info:tooltip", "Browse...")
                    }
                }
            }
        }

        FormCard.FormDelegateSeparator {
            visible: !root.isWindows
        }

        FormCard.AbstractFormDelegate {
            visible: !root.isWindows
            contentItem: ColumnLayout {
                spacing: Kirigami.Units.smallSpacing
                QQC2.Label {
                    text: KI18n.i18nc("@label", "Proton Runtime")
                }
                QQC2.Label {
                    text: KI18n.i18nc("@info:label", "UMU-Latest, UMU-Proton, GE-Latest, GE-Proton or the path to a Proton directory.")
                    font: Kirigami.Theme.smallFont
                    opacity: 0.7
                    wrapMode: Text.WordWrap
                    Layout.fillWidth: true
                }
                RowLayout {
                    QQC2.TextField {
                        id: protonPathField
                        Layout.fillWidth: true
                        text: D2RLoaderConfig.protonPath
                        onEditingFinished: {
                            D2RLoaderConfig.protonPath = text;
                            D2RLoaderConfig.save();
                        }
                        placeholderText: "UMU-Latest"
                    }
                    QQC2.Button {
                        icon.name: "folder-open"
                        onClicked: root.browseFolderFrom(protonPathDialog, D2RLoaderConfig.protonPath)
                        QQC2.ToolTip.visible: hovered
                        QQC2.ToolTip.text: KI18n.i18nc("@info:tooltip", "Browse...")
                    }
                }
            }
        }

        FormCard.FormDelegateSeparator {
            visible: !root.isWindows
        }

        FormCard.FormComboBoxDelegate {
            text: KI18n.i18nc("@label", "Log Level")
            model: [KI18n.i18nc("@item", "DEBUG"), KI18n.i18nc("@item", "INFO"), KI18n.i18nc("@item", "WARN"), KI18n.i18nc("@item", "ERROR")]
            currentIndex: D2RLoaderConfig.logLevel
            onActivated: {
                D2RLoaderConfig.logLevel = currentIndex;
                D2RLoaderConfig.save();
            }
        }

        FormCard.FormDelegateSeparator {}

        FormCard.FormCheckDelegate {
            text: KI18n.i18nc("@label", "Log to File")
            checked: D2RLoaderConfig.logToFile
            onToggled: {
                D2RLoaderConfig.logToFile = checked;
                D2RLoaderConfig.save();
            }
        }

        FormCard.FormDelegateSeparator {}

        FormCard.AbstractFormDelegate {
            enabled: D2RLoaderConfig.logToFile

            contentItem: ColumnLayout {
                spacing: Kirigami.Units.smallSpacing
                QQC2.Label {
                    text: KI18n.i18nc("@label", "Log Directory")
                }
                RowLayout {
                    QQC2.TextField {
                        id: logPathField
                        Layout.fillWidth: true
                        text: D2RLoaderConfig.logPath
                        onEditingFinished: {
                            D2RLoaderConfig.logPath = text;
                            D2RLoaderConfig.save();
                        }
                        placeholderText: KI18n.i18nc("@info:placeholder", "Path to log directory...")
                    }
                    QQC2.Button {
                        icon.name: "folder-open"
                        onClicked: root.browseFolderFrom(logPathDialog, D2RLoaderConfig.logPath)
                        QQC2.ToolTip.visible: hovered
                        QQC2.ToolTip.text: KI18n.i18nc("@info:tooltip", "Browse...")
                    }
                }
            }
        }

        FormCard.FormDelegateSeparator {}

        FormCard.FormTextDelegate {
            visible: UpdateChecker.managedExternally
            text: KI18n.i18nc("@info", "Updates are handled outside D2RLoader.")
            description: KI18n.i18nc("@info:label", "This copy was installed by Flatpak or a distribution package, which updates it.")
        }

        FormCard.FormCheckDelegate {
            visible: !UpdateChecker.managedExternally
            text: KI18n.i18nc("@label", "Check for Updates on Startup")
            checked: D2RLoaderConfig.checkForUpdates
            onToggled: {
                D2RLoaderConfig.checkForUpdates = checked;
                D2RLoaderConfig.save();
            }
        }

        FormCard.FormDelegateSeparator {
            visible: !UpdateChecker.managedExternally
        }

        FormCard.FormButtonDelegate {
            visible: !UpdateChecker.managedExternally
            enabled: !UpdateChecker.checking
            text: KI18n.i18nc("@action:button", "Check for Updates Now")
            description: UpdateChecker.statusText
            icon.name: "view-refresh"
            onClicked: UpdateChecker.checkNow()
        }

        FormCard.FormDelegateSeparator {
            visible: installDelegate.visible
        }

        FormCard.FormButtonDelegate {
            id: installDelegate
            visible: UpdateChecker.updateAvailable && UpdateChecker.canInstall
            text: UpdateChecker.installing ? KI18n.i18nc("@action:button", "Cancel Update") : KI18n.i18nc("@action:button", "Install Update")
            description: UpdateChecker.installStatus.length > 0 ? UpdateChecker.installStatus : KI18n.i18nc("@info:label", "Downloads the installer, runs it and starts D2RLoader again.")
            icon.name: UpdateChecker.installing ? "dialog-cancel" : "system-software-update"
            onClicked: UpdateChecker.installing ? UpdateChecker.cancelInstall() : UpdateChecker.installUpdate()
        }

        FormCard.FormDelegateSeparator {
            visible: UpdateChecker.updateAvailable
        }

        FormCard.FormButtonDelegate {
            visible: UpdateChecker.updateAvailable
            text: KI18n.i18nc("@action:button", "Open Release Page")
            icon.name: "cloud-download"
            onClicked: Qt.openUrlExternally(UpdateChecker.releaseUrl)
        }
    }
}
