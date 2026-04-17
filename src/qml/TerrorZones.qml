pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import org.kde.kirigami as Kirigami

Kirigami.Card {
    id: root
    Layout.fillWidth: true
    Layout.fillHeight: true

    header: Kirigami.Heading {
        text: "Terror Zones"
        level: 2
        Layout.margins: Kirigami.Units.smallSpacing
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
                source: "view-calendar-day"
                Layout.preferredWidth: Kirigami.Units.gridUnit * 3
                Layout.preferredHeight: Kirigami.Units.gridUnit * 3
                color: Kirigami.Theme.textColor
            }

            ColumnLayout {
                Label {
                    text: "Current Terror Zone:"
                    font.bold: true
                    color: Kirigami.Theme.textColor
                }
                Label {
                    text: "The Chaos Sanctuary"
                    font.pointSize: 14
                    color: Kirigami.Theme.textColor
                }
            }

            Item {
                Layout.fillWidth: true
            }

            Label {
                text: "Ends in: 45m"
                color: Kirigami.Theme.positiveTextColor
            }
        }

        Kirigami.Separator {
            Layout.fillWidth: true
        }

        Label {
            text: "Next Predicted Zones:"
            font.bold: true
            color: Kirigami.Theme.textColor
        }

        ListView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            model: [
                {
                    "zone": "Tal Rasha's Tombs",
                    "time": "14:00"
                },
                {
                    "zone": "Cows",
                    "time": "15:00"
                },
                {
                    "zone": "Durance of Hate",
                    "time": "16:00"
                }
            ]
            delegate: ItemDelegate {
                id: zoneDelegate
                required property var modelData
                width: ListView.view.width
                background: null
                contentItem: RowLayout {
                    Label {
                        text: zoneDelegate.modelData.time ?? ""
                        color: Kirigami.Theme.textColor
                        Layout.preferredWidth: Kirigami.Units.gridUnit * 3
                    }
                    Label {
                        text: zoneDelegate.modelData.zone ?? ""
                        color: Kirigami.Theme.textColor
                        Layout.fillWidth: true
                    }
                }
            }
        }
    }
}
