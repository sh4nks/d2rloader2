pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import QtQuick.Dialogs
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard

FormCard.FormCardPage {
    id: root

    title: "Application Settings"

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
        title: "Select Game Directory"
        onAccepted: gamePathField.text = root.urlToPath(selectedFolder)
    }

    FileDialog {
        id: handlePathDialog
        title: "Select handle.exe"
        nameFilters: ["Executable files (*.exe)", "All files (*)"]
        onAccepted: handlePathField.text = root.urlToPath(selectedFile)
    }

    FileDialog {
        id: accountSettingsDialog
        title: "Select Account Settings File"
        nameFilters: ["JSON files (*.json)", "All files (*)"]
        onAccepted: accountSettingsField.text = root.urlToPath(selectedFile)
    }

    FolderDialog {
        id: winePrefixDialog
        title: "Select Wineprefix Directory"
        onAccepted: winePrefixField.text = root.urlToPath(selectedFolder)
    }

    FolderDialog {
        id: protonPathDialog
        title: "Select Proton Runtime Directory"
        onAccepted: protonPathField.text = root.urlToPath(selectedFolder)
    }

    FormCard.FormHeader {
        title: "General"
    }

    FormCard.FormCard {
        FormCard.AbstractFormDelegate {
            contentItem: ColumnLayout {
                spacing: Kirigami.Units.smallSpacing
                QQC2.Label {
                    text: "Game Path"
                }
                QQC2.Label {
                    text: "The directory where Diablo II: Resurrected is installed."
                    font: Kirigami.Theme.smallFont
                    opacity: 0.7
                    wrapMode: Text.WordWrap
                    Layout.fillWidth: true
                }
                RowLayout {
                    QQC2.TextField {
                        id: gamePathField
                        Layout.fillWidth: true
                        placeholderText: "Path to Diablo II Resurrected..."
                    }
                    QQC2.Button {
                        icon.name: "folder-open"
                        onClicked: gamePathDialog.open()
                        QQC2.ToolTip.visible: hovered
                        QQC2.ToolTip.text: "Browse..."
                    }
                }
            }
        }

        FormCard.FormDelegateSeparator {}

        FormCard.AbstractFormDelegate {
            contentItem: ColumnLayout {
                spacing: Kirigami.Units.smallSpacing
                QQC2.Label {
                    text: "Handle.exe Path"
                }
                QQC2.Label {
                    text: "Path to Sysinternals handle.exe, used to kill game mutexes for multi-instance play."
                    font: Kirigami.Theme.smallFont
                    opacity: 0.7
                    wrapMode: Text.WordWrap
                    Layout.fillWidth: true
                }
                RowLayout {
                    QQC2.TextField {
                        id: handlePathField
                        Layout.fillWidth: true
                        placeholderText: "Path to handle.exe..."
                    }
                    QQC2.Button {
                        icon.name: "document-open"
                        onClicked: handlePathDialog.open()
                        QQC2.ToolTip.visible: hovered
                        QQC2.ToolTip.text: "Browse..."
                    }
                    QQC2.Button {
                        icon.name: "download"
                        onClicked: Qt.openUrlExternally("https://download.sysinternals.com/files/Handle.zip")
                        QQC2.ToolTip.visible: hovered
                        QQC2.ToolTip.text: "Download from Sysinternals..."
                    }
                }
            }
        }

        FormCard.FormDelegateSeparator {}

        FormCard.AbstractFormDelegate {
            contentItem: ColumnLayout {
                spacing: Kirigami.Units.smallSpacing
                QQC2.Label {
                    text: "Account Settings File"
                }
                QQC2.Label {
                    text: "The JSON file where all account and profile configurations are stored."
                    font: Kirigami.Theme.smallFont
                    opacity: 0.7
                    wrapMode: Text.WordWrap
                    Layout.fillWidth: true
                }
                RowLayout {
                    QQC2.TextField {
                        id: accountSettingsField
                        Layout.fillWidth: true
                        placeholderText: "Path to accounts.json..."
                    }
                    QQC2.Button {
                        icon.name: "document-open"
                        onClicked: accountSettingsDialog.open()
                        QQC2.ToolTip.visible: hovered
                        QQC2.ToolTip.text: "Browse..."
                    }
                }
            }
        }
    }

    FormCard.FormHeader {
        title: "Advanced Settings"
    }

    FormCard.FormCard {
        FormCard.AbstractFormDelegate {
            contentItem: ColumnLayout {
                spacing: Kirigami.Units.smallSpacing
                QQC2.Label {
                    text: "Wineprefix Location"
                }
                QQC2.Label {
                    text: "The directory containing the Wine configuration for the game (Linux/Steam Deck)."
                    font: Kirigami.Theme.smallFont
                    opacity: 0.7
                    wrapMode: Text.WordWrap
                    Layout.fillWidth: true
                }
                RowLayout {
                    QQC2.TextField {
                        id: winePrefixField
                        Layout.fillWidth: true
                        placeholderText: "Path to Wineprefix..."
                    }
                    QQC2.Button {
                        icon.name: "folder-open"
                        onClicked: winePrefixDialog.open()
                        QQC2.ToolTip.visible: hovered
                        QQC2.ToolTip.text: "Browse..."
                    }
                }
            }
        }

        FormCard.FormDelegateSeparator {}

        FormCard.AbstractFormDelegate {
            contentItem: ColumnLayout {
                spacing: Kirigami.Units.smallSpacing
                QQC2.Label {
                    text: "Proton Runtime"
                }
                QQC2.Label {
                    text: "The directory containing the Proton/Wine compatibility layer runtime."
                    font: Kirigami.Theme.smallFont
                    opacity: 0.7
                    wrapMode: Text.WordWrap
                    Layout.fillWidth: true
                }
                RowLayout {
                    QQC2.TextField {
                        id: protonPathField
                        Layout.fillWidth: true
                        placeholderText: "e.g. GE-Proton or UMU-Latest"
                    }
                    QQC2.Button {
                        icon.name: "folder-open"
                        onClicked: protonPathDialog.open()
                        QQC2.ToolTip.visible: hovered
                        QQC2.ToolTip.text: "Browse..."
                    }
                }
            }
        }

        FormCard.FormDelegateSeparator {}

        FormCard.FormComboBoxDelegate {
            text: "Log Level"
            description: "Set the verbosity of the application logs."
            model: ["DEBUG", "INFO", "WARN", "ERROR"]
        }

        FormCard.FormDelegateSeparator {}

        FormCard.FormCheckDelegate {
            text: "Log to File"
            description: "Save application logs to a file on disk for troubleshooting."
        }

        FormCard.FormDelegateSeparator {}

        FormCard.FormCheckDelegate {
            text: "Check for Updates"
            description: "Automatically check for new versions of D2RLoader on startup."
        }
    }
}
