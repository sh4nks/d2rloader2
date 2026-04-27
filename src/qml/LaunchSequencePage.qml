pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard

FormCard.FormCardPage {
    id: root

    title: i18nc("@title", "Launch Sequences")

    FormCard.FormHeader {
        title: i18nc("@title:group", "Managed Sequences")
    }

    FormCard.FormCard {
        // Mocking the sequence model for the UI prototype
        Repeater {
            model: 3
            delegate: FormCard.AbstractFormDelegate {
                id: sequenceDelegate
                required property int index

                readonly property string sequenceName: [i18nc("@info", "Default Sequence"), i18nc("@info", "Multi-Box Farming"), i18nc("@info", "Mule Transfers")][sequenceDelegate.index]

                onClicked: {
                    // Logic to edit sequence details
                }

                contentItem: RowLayout {
                    spacing: Kirigami.Units.largeSpacing

                    Kirigami.Icon {
                        source: "media-playlist-play"
                        Layout.preferredWidth: Kirigami.Units.iconSizes.medium
                        Layout.preferredHeight: Kirigami.Units.iconSizes.medium
                    }

                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: Kirigami.Units.smallSpacing

                        QQC2.Label {
                            Layout.fillWidth: true
                            text: sequenceDelegate.sequenceName
                            font.bold: true
                            elide: Text.ElideRight
                        }

                        QQC2.Label {
                            Layout.fillWidth: true
                            text: i18nc("@info", "Configured accounts: %1", (index + 2))
                            color: Kirigami.Theme.disabledTextColor
                            font: Kirigami.Theme.smallFont
                            elide: Text.ElideRight
                        }
                    }

                    // Reorder Buttons
                    RowLayout {
                        spacing: 0
                        QQC2.ToolButton {
                            icon.name: "arrow-up"
                            enabled: sequenceDelegate.index > 0
                            onClicked: {
                                // Logic to move sequence up would go here
                            }
                        }
                        QQC2.ToolButton {
                            icon.name: "arrow-down"
                            enabled: sequenceDelegate.index < 2
                            onClicked: {
                                // Logic to move sequence down would go here
                            }
                        }
                    }

                    QQC2.ToolButton {
                        icon.name: "edit-entry"
                        QQC2.ToolTip.visible: hovered
                        QQC2.ToolTip.text: i18nc("@info:tooltip", "Edit Sequence")
                        onClicked: {
                            // Logic to open sequence editor would go here
                        }
                    }

                    QQC2.ToolButton {
                        icon.name: "edit-delete"
                        QQC2.ToolTip.visible: hovered
                        QQC2.ToolTip.text: i18nc("@info:tooltip", "Delete Sequence")
                        onClicked: {
                            // Logic to remove sequence would go here
                        }
                    }

                    FormCard.FormArrow {
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    }
                }
            }
        }

        FormCard.FormDelegateSeparator {
            above: addSequenceDelegate
        }

        FormCard.FormButtonDelegate {
            id: addSequenceDelegate
            text: i18nc("@action:button", "Add New Sequence")
            icon.name: "list-add"
            onClicked: {
                // Logic to create a new sequence would go here
            }
        }
    }

    FormCard.FormHeader {
        title: i18nc("@title:group", "Sequence Behavior")
    }

    FormCard.FormCard {
        FormCard.FormSpinBoxDelegate {
            label: i18nc("@label", "Launch Delay")
            description: i18nc("@info:label", "Seconds to wait between starting each account in the sequence.")
            from: 1
            to: 300
            value: 10
            stepSize: 1
        }

        FormCard.FormDelegateSeparator {}

        FormCard.FormCheckDelegate {
            text: i18nc("@label", "Stop Sequence on Failure")
            description: i18nc("@info:label", "Prevents further accounts from launching if one fails to start.")
            checked: true
        }

        FormCard.FormDelegateSeparator {}

        FormCard.FormCheckDelegate {
            text: i18nc("@label", "Randomize Delay")
            description: i18nc("@info:label", "Adds a small random offset to the launch delay for stealth.")
            checked: false
        }
    }
}
