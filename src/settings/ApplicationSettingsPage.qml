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

    // Helper function to convert URL to local path
    function urlToPath(url) {
        if (!url)
            return "";
        let path = url.toString();
        if (path.startsWith("file://")) {
            path = path.substring(7);
        }
        // Decode URI components (like %20 to space)
        return decodeURIComponent(path);
    }

    FolderDialog {
        id: gamePathDialog
        title: KI18n.i18nc("@title:window", "Select Game Directory")
        onAccepted: {
            D2RLoaderConfig.gamePath = root.urlToPath(selectedFolder);
            D2RLoaderConfig.save();
        }
    }

    FileDialog {
        id: handlePathDialog
        title: KI18n.i18nc("@title:window", "Select handle.exe")
        nameFilters: [KI18n.i18nc("@item", "Executable files (*.exe)"), KI18n.i18nc("@item", "All files (*)")]
        onAccepted: {
            D2RLoaderConfig.handlePath = root.urlToPath(selectedFile);
            D2RLoaderConfig.save();
        }
    }

    FileDialog {
        id: accountSettingsDialog
        title: KI18n.i18nc("@title:window", "Select Account Settings File")
        nameFilters: [KI18n.i18nc("@item", "JSON files (*.json)"), KI18n.i18nc("@item", "All files (*)")]
        onAccepted: {
            D2RLoaderConfig.profilePath = root.urlToPath(selectedFile);
            D2RLoaderConfig.save();
        }
    }

    FolderDialog {
        id: winePrefixDialog
        title: KI18n.i18nc("@title:window", "Select Wineprefix Directory")
        onAccepted: {
            D2RLoaderConfig.wineprefixPath = root.urlToPath(selectedFolder);
            D2RLoaderConfig.save();
        }
    }

    FolderDialog {
        id: protonPathDialog
        title: KI18n.i18nc("@title:window", "Select Proton Runtime Directory")
        onAccepted: {
            D2RLoaderConfig.protonPath = root.urlToPath(selectedFolder);
            D2RLoaderConfig.save();
        }
    }

    FolderDialog {
        id: logPathDialog
        title: KI18n.i18nc("@title:window", "Select Log Directory")
        onAccepted: {
            D2RLoaderConfig.logPath = root.urlToPath(selectedFolder);
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
                QQC2.Label {
                    text: KI18n.i18nc("@info:label", "The directory where Diablo II: Resurrected is installed.")
                    font: Kirigami.Theme.smallFont
                    opacity: 0.7
                    wrapMode: Text.WordWrap
                    Layout.fillWidth: true
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
                        onClicked: gamePathDialog.open()
                        QQC2.ToolTip.visible: hovered
                        QQC2.ToolTip.text: KI18n.i18nc("@info:tooltip", "Browse...")
                    }
                }
            }
        }

        FormCard.FormDelegateSeparator {
            visible: root.isWindows
        }

        FormCard.AbstractFormDelegate {
            visible: root.isWindows

            contentItem: ColumnLayout {
                spacing: Kirigami.Units.smallSpacing
                QQC2.Label {
                    text: KI18n.i18nc("@label", "Handle.exe Path")
                }
                QQC2.Label {
                    text: KI18n.i18nc("@info:label", "Path to Sysinternals handle.exe, used to kill game mutexes for multi-instance play.")
                    font: Kirigami.Theme.smallFont
                    opacity: 0.7
                    wrapMode: Text.WordWrap
                    Layout.fillWidth: true
                }
                RowLayout {
                    QQC2.TextField {
                        id: handlePathField
                        Layout.fillWidth: true
                        text: D2RLoaderConfig.handlePath
                        onEditingFinished: {
                            D2RLoaderConfig.handlePath = text;
                            D2RLoaderConfig.save();
                        }
                        placeholderText: KI18n.i18nc("@info:placeholder", "Path to handle.exe...")
                    }
                    QQC2.Button {
                        icon.name: "document-open"
                        onClicked: handlePathDialog.open()
                        QQC2.ToolTip.visible: hovered
                        QQC2.ToolTip.text: KI18n.i18nc("@info:tooltip", "Browse...")
                    }
                    QQC2.Button {
                        icon.name: "download"
                        onClicked: Qt.openUrlExternally("https://download.sysinternals.com/files/Handle.zip")
                        QQC2.ToolTip.visible: hovered
                        QQC2.ToolTip.text: KI18n.i18nc("@info:tooltip", "Download from Sysinternals...")
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
                QQC2.Label {
                    text: KI18n.i18nc("@info:label", "The JSON file where all account and profile configurations are stored.")
                    font: Kirigami.Theme.smallFont
                    opacity: 0.7
                    wrapMode: Text.WordWrap
                    Layout.fillWidth: true
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
                        onClicked: accountSettingsDialog.open()
                        QQC2.ToolTip.visible: hovered
                        QQC2.ToolTip.text: KI18n.i18nc("@info:tooltip", "Browse...")
                    }
                }
            }
        }

        FormCard.FormDelegateSeparator {}

        FormCard.FormCheckDelegate {
            text: KI18n.i18nc("@label", "Show Menu Bar")
            description: KI18n.i18nc("@info:label", "Show the Settings and Help menus at the top of the main window. Press Ctrl+M to toggle it.")
            checked: D2RLoaderConfig.showMenuBar
            onToggled: {
                D2RLoaderConfig.showMenuBar = checked;
                D2RLoaderConfig.save();
            }
        }
    }

    FormCard.FormHeader {
        title: KI18n.i18nc("@title:group", "Advanced Settings")
    }

    FormCard.FormCard {
        FormCard.AbstractFormDelegate {
            contentItem: ColumnLayout {
                spacing: Kirigami.Units.smallSpacing
                QQC2.Label {
                    text: KI18n.i18nc("@label", "Wineprefix Location")
                }
                QQC2.Label {
                    text: KI18n.i18nc("@info:label", "The directory where the per-account wineprefixes are stored. Defaults to the wineprefixes folder next to the configuration file.")
                    font: Kirigami.Theme.smallFont
                    opacity: 0.7
                    wrapMode: Text.WordWrap
                    Layout.fillWidth: true
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
                        onClicked: winePrefixDialog.open()
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
                    text: KI18n.i18nc("@label", "Proton Runtime")
                }
                QQC2.Label {
                    text: KI18n.i18nc("@info:label", "The directory containing the Proton/Wine compatibility layer runtime.")
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
                        placeholderText: KI18n.i18nc("@info:placeholder", "e.g. GE-Proton or UMU-Latest")
                    }
                    QQC2.Button {
                        icon.name: "folder-open"
                        onClicked: protonPathDialog.open()
                        QQC2.ToolTip.visible: hovered
                        QQC2.ToolTip.text: KI18n.i18nc("@info:tooltip", "Browse...")
                    }
                }
            }
        }

        FormCard.FormDelegateSeparator {}

        FormCard.FormComboBoxDelegate {
            text: KI18n.i18nc("@label", "Log Level")
            description: KI18n.i18nc("@info:label", "Set the verbosity of the application logs.")
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
            description: KI18n.i18nc("@info:label", "Save application logs to a file on disk for troubleshooting.")
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
                QQC2.Label {
                    text: KI18n.i18nc("@info:label", "The directory d2rloader.log is written to.")
                    font: Kirigami.Theme.smallFont
                    opacity: 0.7
                    wrapMode: Text.WordWrap
                    Layout.fillWidth: true
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
                        onClicked: logPathDialog.open()
                        QQC2.ToolTip.visible: hovered
                        QQC2.ToolTip.text: KI18n.i18nc("@info:tooltip", "Browse...")
                    }
                }
            }
        }

        FormCard.FormDelegateSeparator {}

        FormCard.FormCheckDelegate {
            text: KI18n.i18nc("@label", "Check for Updates")
            description: KI18n.i18nc("@info:label", "Automatically check for new versions of D2RLoader on startup.")
            checked: D2RLoaderConfig.checkForUpdates
            onToggled: {
                D2RLoaderConfig.checkForUpdates = checked;
                D2RLoaderConfig.save();
            }
        }
    }
}
