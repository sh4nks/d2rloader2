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
        title: KI18n.i18nc("@title", "Diablo Clone Tracker")

        Label {
            text: GameInfo.lastUpdated.getTime() > 0 ? KI18n.i18nc("@info", "Last updated: %1", Qt.formatTime(GameInfo.lastUpdated, "hh:mm:ss")) : KI18n.i18nc("@info", "Not fetched yet")
            font.italic: true
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

        RowLayout {
            Layout.fillWidth: true
            spacing: Kirigami.Units.smallSpacing

            Label {
                text: KI18n.i18nc("@title", "Realm Progress")
                font.bold: true
                color: Kirigami.Theme.textColor
                Layout.fillWidth: true
            }

            CheckBox {
                text: KI18n.i18nc("@option:check a set of alternative realms", "Return of the Warlock")
                checked: D2RLoaderConfig.rotw
                onToggled: {
                    D2RLoaderConfig.rotw = checked;
                    D2RLoaderConfig.save();
                }
            }
        }

        // Header, sharing its column width with RealmProgress.
        RowLayout {
            Layout.fillWidth: true
            spacing: Kirigami.Units.gridUnit

            Label {
                text: KI18n.i18nc("@title:column", "Region")
                font.bold: true
                opacity: 0.7
                color: Kirigami.Theme.textColor
                Layout.preferredWidth: Kirigami.Units.gridUnit * 5
            }
            Label {
                text: GameInfo.dcloneModeName
                font.bold: true
                opacity: 0.7
                color: Kirigami.Theme.textColor
                Layout.fillWidth: true
            }
        }

        Repeater {
            model: GameInfo.dcloneModel

            delegate: RealmProgress {
                required property var model

                regionName: model.region
                progress: model.progress
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
