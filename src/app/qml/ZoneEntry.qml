pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import org.kde.kirigami as Kirigami
import org.kde.ki18n

/**
 * One terror zone row: a time badge followed by the zone's name and, when
 * known, its immunities, boss packs and superuniques.
 */
Rectangle {
    id: root

    property alias time: timeLabel.text
    property alias zoneName: zoneLabel.text

    /**
     * Whether this is the zone that is currently active, as opposed to a
     * prediction for a later slot.
     */
    property bool current: false

    /**
     * Immunity codes as the API reports them: f, c, l, p, m and ph.
     */
    property list<string> immunities

    /**
     * The minimum and maximum number of boss packs, or empty while unknown.
     */
    property list<int> bossPacks

    property list<string> superUniques

    readonly property var immunityKinds: ({
            "f": {
                name: KI18n.i18nc("@info monster immunity", "Fire"),
                color: "#d9443a"
            },
            "c": {
                name: KI18n.i18nc("@info monster immunity", "Cold"),
                color: "#4a90d9"
            },
            "l": {
                name: KI18n.i18nc("@info monster immunity", "Lightning"),
                color: "#e3c440"
            },
            "p": {
                name: KI18n.i18nc("@info monster immunity", "Poison"),
                color: "#4caf50"
            },
            "m": {
                name: KI18n.i18nc("@info monster immunity", "Magic"),
                color: "#e08a2e"
            },
            "ph": {
                name: KI18n.i18nc("@info monster immunity", "Physical"),
                color: "#a1887f"
            }
        })

    Layout.fillWidth: true
    implicitHeight: Math.max(Kirigami.Units.gridUnit * 3.5,
                             layout.implicitHeight + Kirigami.Units.gridUnit)
    color: Kirigami.Theme.alternateBackgroundColor
    border.color: Qt.rgba(Kirigami.Theme.textColor.r, Kirigami.Theme.textColor.g, Kirigami.Theme.textColor.b, 0.1)
    border.width: 1
    radius: 6

    RowLayout {
        id: layout

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.verticalCenter: parent.verticalCenter
        anchors.leftMargin: Kirigami.Units.gridUnit
        anchors.rightMargin: Kirigami.Units.gridUnit
        spacing: Kirigami.Units.gridUnit

        Rectangle {
            color: Qt.rgba(Kirigami.Theme.textColor.r, Kirigami.Theme.textColor.g, Kirigami.Theme.textColor.b, 0.1)
            radius: 4
            Layout.preferredWidth: Kirigami.Units.gridUnit * 4
            Layout.preferredHeight: Kirigami.Units.gridUnit * 1.8

            Label {
                id: timeLabel
                anchors.centerIn: parent
                font.bold: true
                font.family: Kirigami.Theme.fixedWidthFont.family
                color: root.current ? Kirigami.Theme.positiveTextColor : Kirigami.Theme.textColor
            }
        }

        Kirigami.Icon {
            source: "go-next"
            Layout.preferredWidth: Kirigami.Units.smallSpacing * 2
            Layout.preferredHeight: Kirigami.Units.smallSpacing * 2
            opacity: 0.5
            color: Kirigami.Theme.textColor
        }

        ColumnLayout {
            spacing: Kirigami.Units.smallSpacing
            Layout.fillWidth: true

            Label {
                id: zoneLabel
                font.pointSize: 12
                font.weight: Font.Medium
                color: Kirigami.Theme.textColor
                wrapMode: Text.Wrap
                Layout.fillWidth: true
            }

            GridLayout {
                columns: 2
                columnSpacing: Kirigami.Units.largeSpacing
                rowSpacing: Kirigami.Units.smallSpacing
                visible: root.immunities.length > 0 || root.bossPacks.length > 0 || root.superUniques.length > 0
                Layout.fillWidth: true

                Label {
                    text: KI18n.i18nc("@label", "Immunities:")
                    font: Kirigami.Theme.smallFont
                    opacity: 0.7
                    visible: root.immunities.length > 0
                }

                Flow {
                    spacing: Kirigami.Units.smallSpacing
                    visible: root.immunities.length > 0
                    Layout.fillWidth: true

                    Repeater {
                        model: root.immunities

                        delegate: Rectangle {
                            id: chip

                            required property string modelData
                            readonly property var kind: root.immunityKinds[modelData]
                            readonly property color tint: kind ? kind.color : Kirigami.Theme.disabledTextColor

                            implicitWidth: chipLabel.implicitWidth + Kirigami.Units.largeSpacing * 2
                            implicitHeight: chipLabel.implicitHeight + Kirigami.Units.smallSpacing
                            radius: height / 2
                            color: Qt.rgba(tint.r, tint.g, tint.b, 0.2)
                            border.color: Qt.rgba(tint.r, tint.g, tint.b, 0.7)
                            border.width: 1

                            Label {
                                id: chipLabel
                                anchors.centerIn: parent
                                text: chip.kind ? chip.kind.name : chip.modelData
                                font: Kirigami.Theme.smallFont
                                color: Kirigami.Theme.textColor
                            }
                        }
                    }
                }

                Label {
                    text: KI18n.i18nc("@label", "Boss packs:")
                    font: Kirigami.Theme.smallFont
                    opacity: 0.7
                    visible: root.bossPacks.length > 0
                }

                Label {
                    text: {
                        if (root.bossPacks.length < 2 || root.bossPacks[0] === root.bossPacks[1])
                            return String(root.bossPacks[0] ?? "");
                        return KI18n.i18nc("@info smallest and largest number of boss packs", "%1-%2", root.bossPacks[0], root.bossPacks[1]);
                    }
                    font: Kirigami.Theme.smallFont
                    visible: root.bossPacks.length > 0
                    Layout.fillWidth: true
                }

                Label {
                    text: KI18n.i18nc("@label", "Superuniques:")
                    font: Kirigami.Theme.smallFont
                    opacity: 0.7
                    visible: root.superUniques.length > 0
                }

                Label {
                    text: root.superUniques.join(", ")
                    font: Kirigami.Theme.smallFont
                    wrapMode: Text.Wrap
                    visible: root.superUniques.length > 0
                    Layout.fillWidth: true
                }
            }
        }
    }
}
