pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import org.kde.kirigami as Kirigami

Kirigami.Card {
    id: root
    Layout.fillWidth: true
    Layout.fillHeight: true

    header: RowLayout {
        Layout.fillWidth: true
        Layout.margins: Kirigami.Units.smallSpacing

        Kirigami.Heading {
            text: i18nc("@title", "Terror Zones")
            level: 2
            Layout.alignment: Qt.AlignVCenter
        }

        Item {
            Layout.fillWidth: true
        }

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

        // --- Current Terror Zone Section ---
        ColumnLayout {
            spacing: Kirigami.Units.smallSpacing
            Layout.fillWidth: true

            Label {
                text: i18nc("@label", "Current Terror Zone:")
                font.bold: true
                opacity: 0.7
                color: Kirigami.Theme.textColor
            }

            Rectangle {
                Layout.fillWidth: true
                implicitHeight: Kirigami.Units.gridUnit * 3.5
                color: Kirigami.Theme.alternateBackgroundColor
                border.color: Qt.rgba(Kirigami.Theme.textColor.r, Kirigami.Theme.textColor.g, Kirigami.Theme.textColor.b, 0.1)
                border.width: 1
                radius: 6

                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: Kirigami.Units.gridUnit
                    anchors.rightMargin: Kirigami.Units.gridUnit
                    spacing: Kirigami.Units.gridUnit

                    // Time Badge
                    Rectangle {
                        color: Qt.rgba(Kirigami.Theme.textColor.r, Kirigami.Theme.textColor.g, Kirigami.Theme.textColor.b, 0.1)
                        radius: 4
                        Layout.preferredWidth: Kirigami.Units.gridUnit * 4
                        Layout.preferredHeight: Kirigami.Units.gridUnit * 1.8

                        Label {
                            anchors.centerIn: parent
                            text: "13:30"
                            font.bold: true
                            font.family: "monospace"
                            color: Kirigami.Theme.positiveTextColor
                        }
                    }

                    Kirigami.Icon {
                        source: "go-next"
                        Layout.preferredWidth: Kirigami.Units.smallSpacing * 2
                        Layout.preferredHeight: Kirigami.Units.smallSpacing * 2
                        opacity: 0.5
                        color: Kirigami.Theme.textColor
                    }

                    Label {
                        text: i18nc("@info", "The Chaos Sanctuary")
                        font.pointSize: 12
                        font.weight: Font.Medium
                        color: Kirigami.Theme.textColor
                        Layout.fillWidth: true
                    }
                }
            }
        }

        Kirigami.Separator {
            Layout.fillWidth: true
        }

        // --- Next Predicted Zone Section ---
        ColumnLayout {
            spacing: Kirigami.Units.smallSpacing
            Layout.fillWidth: true

            Label {
                text: i18nc("@label", "Next Predicted Zone:")
                font.bold: true
                opacity: 0.7
                color: Kirigami.Theme.textColor
            }

            Rectangle {
                Layout.fillWidth: true
                implicitHeight: Kirigami.Units.gridUnit * 3.5
                color: Kirigami.Theme.alternateBackgroundColor
                border.color: Qt.rgba(Kirigami.Theme.textColor.r, Kirigami.Theme.textColor.g, Kirigami.Theme.textColor.b, 0.1)
                border.width: 1
                radius: 6

                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: Kirigami.Units.gridUnit
                    anchors.rightMargin: Kirigami.Units.gridUnit
                    spacing: Kirigami.Units.gridUnit

                    // Time Badge
                    Rectangle {
                        color: Qt.rgba(Kirigami.Theme.textColor.r, Kirigami.Theme.textColor.g, Kirigami.Theme.textColor.b, 0.1)
                        radius: 4
                        Layout.preferredWidth: Kirigami.Units.gridUnit * 4
                        Layout.preferredHeight: Kirigami.Units.gridUnit * 1.8

                        Label {
                            anchors.centerIn: parent
                            text: "14:00"
                            font.bold: true
                            font.family: "monospace"
                            color: Kirigami.Theme.textColor
                        }
                    }

                    Kirigami.Icon {
                        source: "go-next"
                        Layout.preferredWidth: Kirigami.Units.smallSpacing * 2
                        Layout.preferredHeight: Kirigami.Units.smallSpacing * 2
                        opacity: 0.5
                        color: Kirigami.Theme.textColor
                    }

                    Label {
                        text: i18nc("@info", "Tal Rasha's Tombs")
                        font.pointSize: 12
                        font.weight: Font.Medium
                        color: Kirigami.Theme.textColor
                        Layout.fillWidth: true
                    }
                }
            }
        }

        Item {
            Layout.fillHeight: true
        }
    }
}
