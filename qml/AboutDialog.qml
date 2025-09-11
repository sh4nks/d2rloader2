import QtQuick 6.6
import QtQuick.Controls 6.6
import QtQuick.Window 6.6
import QtQuick.Layouts 6.6

ApplicationWindow {
    id: aboutDialog
    minimumWidth: 480
    maximumWidth: 480
    minimumHeight: 280
    maximumHeight: 280
    modality: Qt.ApplicationModal
    x: Screen.width / 2 - width / 2
    y: Screen.height / 2 - height / 2
    title: "About"
    visible: true

    Units {
        id: units
    }

    onVisibilityChanged: console.log(units.gridUnit * 22, Math.max(420, units.gridUnit * 22))

    ColumnLayout {
        id: mainColumn
        anchors.fill: parent
        anchors.topMargin: 10
        anchors.leftMargin: 10
        anchors.rightMargin: 10
        anchors.bottomMargin: 10
        //spacing: units.gridUnit

        Column {
            Layout.alignment: Qt.AlignTop
            spacing: 10

            Heading {
                Layout.alignment: Qt.AlignHCenter
                text: qsTr("D2RLoader")
                level: 3
                wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                width: mainColumn.width - units.gridUnit * 2
            }

            Label {
                width: mainColumn.width - units.gridUnit * 2
                wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                text: qsTr("Version %1").arg("APP_VERSION")
            }

            Label {
                width: mainColumn.width - units.gridUnit * 2
                wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                visible: true
                text: qsTr("Fedora Media Writer is now checking for new releases")
            }

            Label {
                width: mainColumn.width - units.gridUnit * 2
                wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                text: qsTr("Please report bugs or your suggestions on %1").arg("<a href=\"https://github.com/sh4nks/d2rloader/issues\">sh4nks/d2rloader</a>")
                textFormat: Text.RichText
                onLinkActivated: Qt.openUrlExternally(link)
                opacity: 0.6

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
                text: qsTr("Close")
            }
        }
    }
}
