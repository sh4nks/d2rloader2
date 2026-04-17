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
        text: "Diablo Clone Tracker"
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
                source: "view-media-artist"
                Layout.preferredWidth: Kirigami.Units.gridUnit * 3
                Layout.preferredHeight: Kirigami.Units.gridUnit * 3
                color: Kirigami.Theme.negativeTextColor
            }

            ColumnLayout {
                Label {
                    text: "Current Progress:"
                    font.bold: true
                    color: Kirigami.Theme.textColor
                }
                Label {
                    text: "Stage 3/6: Terror begins to form within Sanctuary"
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
            text: "Regional Progress (Softcore Ladder):"
            font.bold: true
            color: Kirigami.Theme.textColor
        }

        GridLayout {
            columns: 2
            Layout.fillWidth: true
            rowSpacing: Kirigami.Units.smallSpacing
            columnSpacing: Kirigami.Units.gridUnit

            Label {
                text: "Americas:"
                color: Kirigami.Theme.textColor
            }
            ProgressBar {
                value: 0.5
                Layout.fillWidth: true
                Kirigami.Theme.colorSet: Kirigami.Theme.View
                Kirigami.Theme.inherit: false
            }

            Label {
                text: "Europe:"
                color: Kirigami.Theme.textColor
            }
            ProgressBar {
                value: 0.33
                Layout.fillWidth: true
                Kirigami.Theme.colorSet: Kirigami.Theme.View
                Kirigami.Theme.inherit: false
            }

            Label {
                text: "Asia:"
                color: Kirigami.Theme.textColor
            }
            ProgressBar {
                value: 0.16
                Layout.fillWidth: true
                Kirigami.Theme.colorSet: Kirigami.Theme.View
                Kirigami.Theme.inherit: false
            }
        }

        Item {
            Layout.fillHeight: true
        }

        Label {
            text: "Last updated: 2 minutes ago"
            font.italic: true
            color: Kirigami.Theme.textColor
            opacity: 0.6
            Layout.alignment: Qt.AlignRight
        }
    }
}
