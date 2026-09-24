import QtQuick 6.6
import QtQuick.Controls 6.6
import QtQuick.Window 6.6
import QtQuick.Layouts 6.6
import org.kde.kirigami as Kirigami
import org.kde.ki18n

import com.someblocks.d2rloader.settings as Settings

ApplicationWindow {
    id: aboutDialog
    minimumWidth: 480
    maximumWidth: 480
    minimumHeight: 300
    maximumHeight: 300
    modality: Qt.ApplicationModal
    x: Screen.width / 2 - width / 2
    y: Screen.height / 2 - height / 2
    title: KI18n.i18nc("@title:window", "About")
    visible: false

    RowLayout {
        id: mainRow
        anchors.fill: parent
        anchors.topMargin: 10
        anchors.leftMargin: 10
        anchors.rightMargin: 10
        anchors.bottomMargin: 10
        spacing: 20

        Rectangle {
            id: appIcon
            Layout.alignment: Qt.AlignTop
            color: 'red'
            Layout.minimumWidth: 60
            Layout.preferredWidth: 60
            Layout.maximumWidth: 60
            Layout.minimumHeight: 60
            Text {
                anchors.centerIn: parent
                text: parent.width + 'x' + parent.height
            }
        }

        ColumnLayout {
            id: mainColumn
            Layout.alignment: Qt.AlignTop
            Layout.preferredWidth: parent.width - appIcon.width

            Column {
                Layout.fillHeight: true
                Kirigami.Heading {
                    text: KI18n.i18nc("@title", "D2RLoader")
                    level: 1
                    wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                    width: aboutDialog.width - (appIcon.width + mainRow.spacing + mainRow.anchors.leftMargin + mainRow.anchors.rightMargin)
                    horizontalAlignment: Text.AlignHCenter
                }

                Label {
                    topPadding: 5
                    bottomPadding: 20
                    horizontalAlignment: Text.AlignHCenter
                    width: aboutDialog.width - (appIcon.width + mainRow.spacing + mainRow.anchors.leftMargin + mainRow.anchors.rightMargin)
                    wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                    text: KI18n.i18nc("@info:label", "A Cross-platform and Open Source Diablo 2 Resurrected Loader written in C++/Qt")
                    font.pointSize: 8
                }

                Label {
                    horizontalAlignment: Text.AlignHCenter
                    wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                    width: aboutDialog.width - (appIcon.width + mainRow.spacing + mainRow.anchors.leftMargin + mainRow.anchors.rightMargin)
                    textFormat: Text.RichText
                    text: KI18n.i18nc("@info %1 is the version number", "<strong>Version:</strong> %1", Application.version)
                }
                Label {
                    horizontalAlignment: Text.AlignHCenter
                    wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                    width: aboutDialog.width - (appIcon.width + mainRow.spacing + mainRow.anchors.leftMargin + mainRow.anchors.rightMargin)
                    textFormat: Text.RichText
                    text: KI18n.i18nc("@info %1 is the Qt version number", "<strong>Qt:</strong> %1", Settings.About.qtVersion)
                }
                Label {
                    horizontalAlignment: Text.AlignHCenter
                    wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                    width: aboutDialog.width - (appIcon.width + mainRow.spacing + mainRow.anchors.leftMargin + mainRow.anchors.rightMargin)
                    textFormat: Text.RichText
                    text: KI18n.i18nc("@info", "<strong>Install Path:</strong> <code>/usr/bin/d2rloader</code>")
                }
                Label {
                    horizontalAlignment: Text.AlignHCenter
                    wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                    width: aboutDialog.width - (appIcon.width + mainRow.spacing + mainRow.anchors.leftMargin + mainRow.anchors.rightMargin)
                    textFormat: Text.RichText
                    text: KI18n.i18nc("@info", "<strong>License:</strong> MIT")
                }
                Label {
                    horizontalAlignment: Text.AlignHCenter
                    wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                    width: aboutDialog.width - (appIcon.width + mainRow.spacing + mainRow.anchors.leftMargin + mainRow.anchors.rightMargin)
                    textFormat: Text.RichText
                    text: KI18n.i18nc("@info", "<strong>Source Code:</strong> <a href=\"https://github.com/sh4nks/d2rloader\">GitHub</a>")
                }
                Label {
                    horizontalAlignment: Text.AlignHCenter
                    wrapMode: Text.WordWrap
                    text: KI18n.i18nc("@info", "TZ Info and DClone Info provided by <a href=\"https://d2emu.com\">D2Emu.com</a>")
                    textFormat: Text.RichText
                    onLinkActivated: link => Qt.openUrlExternally(link)
                    width: aboutDialog.width - (appIcon.width + mainRow.spacing + mainRow.anchors.leftMargin + mainRow.anchors.rightMargin)

                    MouseArea {
                        anchors.fill: parent
                        acceptedButtons: Qt.NoButton
                        cursorShape: parent.hoveredLink ? Qt.PointingHandCursor : Qt.ArrowCursor
                    }
                }

                Label {
                    horizontalAlignment: Text.AlignHCenter
                    wrapMode: Text.WordWrap
                    text: KI18n.i18nc("@info", "Please report bugs or your suggestions on <a href=\"https://github.com/sh4nks/d2rloader/issues\">sh4nks/d2rloader</a>")
                    textFormat: Text.RichText
                    onLinkActivated: link => Qt.openUrlExternally(link)
                    opacity: 0.7
                    topPadding: 35
                    width: aboutDialog.width - (appIcon.width + mainRow.spacing + mainRow.anchors.leftMargin + mainRow.anchors.rightMargin)

                    MouseArea {
                        anchors.fill: parent
                        acceptedButtons: Qt.NoButton
                        cursorShape: parent.hoveredLink ? Qt.PointingHandCursor : Qt.ArrowCursor
                    }
                }
            }

            RowLayout {
                Layout.alignment: Qt.AlignBottom

                Item {
                    Layout.fillWidth: true
                }

                Button {
                    id: closeButton
                    onClicked: aboutDialog.close()
                    text: KI18n.i18nc("@action:button", "Close")
                }
            }
        }
    }
}
