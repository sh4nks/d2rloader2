pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import QtQuick.Dialogs
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard

FormCard.FormCardPage {
    id: root

    title: i18nc("@title", "Application Settings")

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
        title: i18nc("@title:window", "Select Game Directory")
        onAccepted: gamePathField.text = root.urlToPath(selectedFolder)
    }

    FileDialog {
        id: handlePathDialog
        title: i18nc("@title:window", "Select handle.exe")
        nameFilters: [i18nc("@item", "Executable files (*.exe)"), i18nc("@item", "All files (*)")]
        onAccepted: handlePathField.text = root.urlToPath(selectedFile)
    }

    FileDialog {
        id: accountSettingsDialog
        title: i18nc("@title:window", "Select Account Settings File")
        nameFilters: [i18nc("@item", "JSON files (*.json)"), i18nc("@item", "All files (*)")]
        onAccepted: accountSettingsField.text = root.urlToPath(selectedFile)
    }

    FolderDialog {
        id: winePrefixDialog
        title: i18nc("@title:window", "Select Wineprefix Directory")
        onAccepted: winePrefixField.text = root.urlToPath(selectedFolder)
    }

    FolderDialog {
        id: protonPathDialog
        title: i18nc("@title:window", "Select Proton Runtime Directory")
        onAccepted: protonPathField.text = root.urlToPath(selectedFolder)
    }

    FormCard.FormHeader {
        title: i18nc("@title:group", "General")
    }

    FormCard.FormCard {
        FormCard.AbstractFormDelegate {
            contentItem: ColumnLayout {
                spacing: Kirigami.Units.smallSpacing
                QQC2.Label {
                    text: i18nc("@label", "Game Path")
                }
                QQC2.Label {
                    text: i18nc("@info:label", "The directory where Diablo II: Resurrected is installed.")
                    font: Kirigami.Theme.smallFont
                    opacity: 0.7
                    wrapMode: Text.WordWrap
                    Layout.fillWidth: true
                }
                RowLayout {
                    QQC2.TextField {
                        id: gamePathField
                        Layout.fillWidth: true
                        placeholderText: i18nc("@info:placeholder", "Path to Diablo II Resurrected...")
                    }
                    QQC2.Button {
                        icon.name: "folder-open"
                        onClicked: gamePathDialog.open()
                        QQC2.ToolTip.visible: hovered
                        QQC2.ToolTip.text: i18nc("@info:tooltip", "Browse...")
                    }
                }
            }
        }

        FormCard.FormDelegateSeparator {}

        FormCard.AbstractFormDelegate {
            contentItem: ColumnLayout {
                spacing: Kirigami.Units.smallSpacing
                QQC2.Label {
                    text: i18nc("@label", "Handle.exe Path")
                }
                QQC2.Label {
                    text: i18nc("@info:label", "Path to Sysinternals handle.exe, used to kill game mutexes for multi-instance play.")
                    font: Kirigami.Theme.smallFont
                    opacity: 0.7
                    wrapMode: Text.WordWrap
                    Layout.fillWidth: true
                }
                RowLayout {
                    QQC2.TextField {
                        id: handlePathField
                        Layout.fillWidth: true
                        placeholderText: i18nc("@info:placeholder", "Path to handle.exe...")
                    }
                    QQC2.Button {
                        icon.name: "document-open"
                        onClicked: handlePathDialog.open()
                        QQC2.ToolTip.visible: hovered
                        QQC2.ToolTip.text: i18nc("@info:tooltip", "Browse...")
                    }
                    QQC2.Button {
                        icon.name: "download"
                        onClicked: Qt.openUrlExternally("https://download.sysinternals.com/files/Handle.zip")
                        QQC2.ToolTip.visible: hovered
                        QQC2.ToolTip.text: i18nc("@info:tooltip", "Download from Sysinternals...")
                    }
                }
            }
        }

        FormCard.FormDelegateSeparator {}

        FormCard.AbstractFormDelegate {
            contentItem: ColumnLayout {
                spacing: Kirigami.Units.smallSpacing
                QQC2.Label {
                    text: i18nc("@label", "Account Settings File")
                }
                QQC2.Label {
                    text: i18nc("@info:label", "The JSON file where all account and profile configurations are stored.")
                    font: Kirigami.Theme.smallFont
                    opacity: 0.7
                    wrapMode: Text.WordWrap
                    Layout.fillWidth: true
                }
                RowLayout {
                    QQC2.TextField {
                        id: accountSettingsField
                        Layout.fillWidth: true
                        placeholderText: i18nc("@info:placeholder", "Path to accounts.json...")
                    }
                    QQC2.Button {
                        icon.name: "document-open"
                        onClicked: accountSettingsDialog.open()
                        QQC2.ToolTip.visible: hovered
                        QQC2.ToolTip.text: i18nc("@info:tooltip", "Browse...")
                    }
                }
            }
        }
    }

    FormCard.FormHeader {
        title: i18nc("@title:group", "Advanced Settings")
    }

    FormCard.FormCard {
        FormCard.AbstractFormDelegate {
            contentItem: ColumnLayout {
                spacing: Kirigami.Units.smallSpacing
                QQC2.Label {
                    text: i18nc("@label", "Wineprefix Location")
                }
                QQC2.Label {
                    text: i18nc("@info:label", "The directory containing the Wine configuration for the game (Linux/Steam Deck).")
                    font: Kirigami.Theme.smallFont
                    opacity: 0.7
                    wrapMode: Text.WordWrap
                    Layout.fillWidth: true
                }
                RowLayout {
                    QQC2.TextField {
                        id: winePrefixField
                        Layout.fillWidth: true
                        placeholderText: i18nc("@info:placeholder", "Path to Wineprefix...")
                    }
                    QQC2.Button {
                        icon.name: "folder-open"
                        onClicked: winePrefixDialog.open()
                        QQC2.ToolTip.visible: hovered
                        QQC2.ToolTip.text: i18nc("@info:tooltip", "Browse...")
                    }
                }
            }
        }

        FormCard.FormDelegateSeparator {}

        FormCard.AbstractFormDelegate {
            contentItem: ColumnLayout {
                spacing: Kirigami.Units.smallSpacing
                QQC2.Label {
                    text: i18nc("@label", "Proton Runtime")
                }
                QQC2.Label {
                    text: i18nc("@info:label", "The directory containing the Proton/Wine compatibility layer runtime.")
                    font: Kirigami.Theme.smallFont
                    opacity: 0.7
                    wrapMode: Text.WordWrap
                    Layout.fillWidth: true
                }
                RowLayout {
                    QQC2.TextField {
                        id: protonPathField
                        Layout.fillWidth: true
                        placeholderText: i18nc("@info:placeholder", "e.g. GE-Proton or UMU-Latest")
                    }
                    QQC2.Button {
                        icon.name: "folder-open"
                        onClicked: protonPathDialog.open()
                        QQC2.ToolTip.visible: hovered
                        QQC2.ToolTip.text: i18nc("@info:tooltip", "Browse...")
                    }
                }
            }
        }

        FormCard.FormDelegateSeparator {}

        FormCard.FormComboBoxDelegate {
            text: i18nc("@label", "Log Level")
            description: i18nc("@info:label", "Set the verbosity of the application logs.")
            model: [i18nc("@item", "DEBUG"), i18nc("@item", "INFO"), i18nc("@item", "WARN"), i18nc("@item", "ERROR")]
        }

        FormCard.FormDelegateSeparator {}

        FormCard.FormCheckDelegate {
            text: i18nc("@label", "Log to File")
            description: i18nc("@info:label", "Save application logs to a file on disk for troubleshooting.")
        }

        FormCard.FormDelegateSeparator {}

        FormCard.FormCheckDelegate {
            text: i18nc("@label", "Check for Updates")
            description: i18nc("@info:label", "Automatically check for new versions of D2RLoader on startup.")
        }
    }
}
