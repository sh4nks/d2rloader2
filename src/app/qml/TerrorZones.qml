pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import org.kde.kirigami as Kirigami

Kirigami.Card {
    id: root

    header: CardHeader {
        title: i18nc("@title", "Terror Zones")

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

            ZoneEntry {
                current: true
                time: "13:30"
                zoneName: i18nc("@info", "The Chaos Sanctuary")
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

            ZoneEntry {
                time: "14:00"
                zoneName: i18nc("@info", "Tal Rasha's Tombs")
            }
        }

        Item {
            Layout.fillHeight: true
        }
    }
}
