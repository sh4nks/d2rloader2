pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import org.kde.kirigami as Kirigami

Kirigami.Card {
    id: root
    Layout.fillWidth: true
    Layout.fillHeight: true

    header: CardHeader {
        title: i18nc("@title", "Diablo Clone Tracker")

        Button {
            text: i18nc("@action:button", "Refresh")
            icon.name: "view-refresh"
            onClicked: {
                // Refresh logic
            }
        }
    }
    contentItem: ColumnLayout {
        spacing: Kirigami.Units.gridUnit
        Layout.margins: Kirigami.Units.smallSpacing

        Kirigami.Theme.colorSet: Kirigami.Theme.View
        Kirigami.Theme.inherit: true

        RowLayout {
            Layout.fillWidth: true
            Layout.leftMargin: Kirigami.Units.smallSpacing
            Layout.rightMargin: Kirigami.Units.smallSpacing
            spacing: Kirigami.Units.gridUnit

            Kirigami.Icon {
                source: "view-media-artist"
                Layout.preferredWidth: Kirigami.Units.gridUnit * 3
                Layout.preferredHeight: Kirigami.Units.gridUnit * 3
                color: Kirigami.Theme.negativeTextColor
            }

            ColumnLayout {
                Label {
                    text: i18nc("@label", "Current Progress:")
                    font.bold: true
                    color: Kirigami.Theme.textColor
                }
                Label {
                    text: i18nc("@info", "Stage 3/6: Terror begins to form within Sanctuary")
                    font.pointSize: 12
                    color: Kirigami.Theme.textColor
                    wrapMode: Text.WordWrap
                    Layout.fillWidth: true
                }
            }
        }

        Kirigami.Separator {
            Layout.fillWidth: true
        }

        Label {
            text: i18nc("@title", "Regional Progress (Softcore Ladder):")
            font.bold: true
            color: Kirigami.Theme.textColor
        }

        ColumnLayout {
            Layout.fillWidth: true
            spacing: Kirigami.Units.smallSpacing

            Repeater {
                model: [
                    {
                        region: i18nc("@label", "Americas:"),
                        progress: 0.5
                    },
                    {
                        region: i18nc("@label", "Europe:"),
                        progress: 0.33
                    },
                    {
                        region: i18nc("@label", "Asia:"),
                        progress: 0.16
                    }
                ]

                delegate: RowLayout {
                    id: regionRow
                    required property var modelData

                    Layout.fillWidth: true
                    spacing: Kirigami.Units.gridUnit

                    Label {
                        text: regionRow.modelData.region
                        color: Kirigami.Theme.textColor
                        Layout.preferredWidth: Kirigami.Units.gridUnit * 5
                    }

                    ProgressBar {
                        value: regionRow.modelData.progress
                        Layout.fillWidth: true
                        Kirigami.Theme.colorSet: Kirigami.Theme.View
                        Kirigami.Theme.inherit: false
                    }
                }
            }
        }

        Item {
            Layout.fillHeight: true
        }

        Label {
            text: i18nc("@info", "Last updated: 2 minutes ago")
            font.italic: true
            color: Kirigami.Theme.textColor
            opacity: 0.6
            Layout.alignment: Qt.AlignRight
        }
    }
}
