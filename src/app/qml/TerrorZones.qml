pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import org.kde.kirigami as Kirigami
import org.kde.ki18n
import com.someblocks.d2rloader.core

Kirigami.Card {
    id: root

    header: CardHeader {
        title: KI18n.i18nc("@title", "Terror Zones")

        Label {
            text: KI18n.i18nc("@info terror zone data source", "Terror Zone data provided by d2emu.com")
            font: Kirigami.Theme.smallFont
            color: Kirigami.Theme.textColor
            opacity: 0.6
        }

        Button {
            text: KI18n.i18nc("@action:button", "Refresh")
            icon.name: "view-refresh"
            enabled: !GameInfo.loading
            onClicked: GameInfo.refresh()
        }
    }
    contentItem: ColumnLayout {
        spacing: Kirigami.Units.largeSpacing
        Layout.margins: Kirigami.Units.smallSpacing

        Kirigami.Theme.colorSet: Kirigami.Theme.View
        Kirigami.Theme.inherit: true

        // --- Current Terror Zone Section ---
        ColumnLayout {
            spacing: Kirigami.Units.smallSpacing
            Layout.fillWidth: true

            Label {
                text: KI18n.i18nc("@label", "Current Terror Zone:")
                font.bold: true
                opacity: 0.7
                color: Kirigami.Theme.textColor
            }

            ZoneEntry {
                current: true
                time: KI18n.i18nc("@info:status the terror zone that is active right now", "now")
                zoneName: GameInfo.currentZones.length > 0 ? GameInfo.currentZones.join(", ") : KI18n.i18nc("@info:placeholder", "No data yet - press Refresh")
                immunities: GameInfo.currentImmunities
                bossPacks: GameInfo.currentBossPacks
                superUniques: GameInfo.currentSuperUniques
            }
        }

        // --- Next Predicted Zone Section ---
        ColumnLayout {
            spacing: Kirigami.Units.smallSpacing
            Layout.fillWidth: true

            Label {
                text: KI18n.i18nc("@label", "Next Predicted Zone:")
                font.bold: true
                opacity: 0.7
                color: Kirigami.Theme.textColor
            }

            ZoneEntry {
                time: GameInfo.nextTerrorTime.getTime() > 0 ? Qt.formatTime(GameInfo.nextTerrorTime, "hh:mm") : "--:--"
                zoneName: {
                    if (GameInfo.nextZones.length > 0)
                        return GameInfo.nextZones.join(", ");
                    if (GameInfo.lastUpdated.getTime() <= 0)
                        return KI18n.i18nc("@info:placeholder", "No data yet - press Refresh");
                    if (availability.minutesLeft > 0)
                        return KI18n.i18ncp("@info:status", "Available in %1 minute", "Available in %1 minutes", availability.minutesLeft);
                    return KI18n.i18nc("@info:status the next terror zone is about to be published", "Available shortly");
                }
                immunities: GameInfo.nextImmunities
                bossPacks: GameInfo.nextBossPacks
                superUniques: GameInfo.nextSuperUniques
            }
        }

        Timer {
            id: availability

            property int minutesLeft: 0

            interval: Kirigami.Units.humanMoment
            repeat: true
            triggeredOnStart: true
            running: GameInfo.nextZones.length === 0

            onTriggered: {
                const msecs = GameInfo.predictionAvailableTime.getTime() - Date.now();
                minutesLeft = msecs > 0 ? Math.ceil(msecs / 60000) : 0;
            }
        }

        Item {
            Layout.fillHeight: true
        }

        Kirigami.InlineMessage {
            Layout.fillWidth: true
            type: Kirigami.MessageType.Error
            text: GameInfo.errorString
            visible: GameInfo.errorString.length > 0
        }
    }
}
