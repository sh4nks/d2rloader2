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
        title: i18nc("@title", "Application Log")

        Button {
            text: i18nc("@action:button", "Clear Logs")
            icon.name: "edit-clear-list"
            onClicked: logModel.clear()
        }
    }
    contentItem: ColumnLayout {
        spacing: 0
        Layout.margins: 0

        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true

            Kirigami.Theme.colorSet: Kirigami.Theme.View
            Kirigami.Theme.inherit: true

            color: Kirigami.Theme.backgroundColor
            border.color: Qt.rgba(Kirigami.Theme.textColor.r, Kirigami.Theme.textColor.g, Kirigami.Theme.textColor.b, 0.1)
            radius: 2

            ScrollView {
                anchors.fill: parent
                clip: true

                ListView {
                    id: logView
                    model: ListModel {
                        id: logModel
                        ListElement {
                            timestamp: "12:00:01"
                            level: "INFO"
                            message: "Application started."
                        }
                        ListElement {
                            timestamp: "12:00:05"
                            level: "DEBUG"
                            message: "Loading account configurations..."
                        }
                        ListElement {
                            timestamp: "12:00:10"
                            level: "INFO"
                            message: "Account 'cow' connected successfully."
                        }
                        ListElement {
                            timestamp: "12:05:22"
                            level: "WARNING"
                            message: "Connection latency detected for region: Europe."
                        }
                        ListElement {
                            timestamp: "12:10:00"
                            level: "ERROR"
                            message: "Failed to launch 'goat': Authentication token expired."
                        }
                    }

                    delegate: ItemDelegate {
                        id: logDelegate
                        required property string timestamp
                        required property string level
                        required property string message

                        width: ListView.view.width
                        topPadding: 0
                        bottomPadding: 0
                        background: null
                        contentItem: RowLayout {
                            spacing: Kirigami.Units.gridUnit

                            Kirigami.Theme.inherit: true
                            Kirigami.Theme.colorSet: Kirigami.Theme.View

                            Label {
                                text: logDelegate.timestamp
                                color: Kirigami.Theme.textColor
                                opacity: 0.6
                                font.family: "monospace"
                                Layout.preferredWidth: Kirigami.Units.gridUnit * 4
                            }

                            Label {
                                text: logDelegate.level
                                color: {
                                    if (logDelegate.level === "DEBUG")
                                        return Kirigami.Theme.disabledTextColor;
                                    if (logDelegate.level === "ERROR")
                                        return Kirigami.Theme.negativeTextColor;
                                    if (logDelegate.level === "WARNING")
                                        return Kirigami.Theme.neutralTextColor;
                                    return Kirigami.Theme.positiveTextColor;
                                }
                                font.bold: true
                                font.family: "monospace"
                                Layout.preferredWidth: Kirigami.Units.gridUnit * 4
                            }
                            TextEdit {
                                Layout.fillWidth: true
                                text: logDelegate.message
                                font.family: "monospace"
                                color: Kirigami.Theme.textColor
                                readOnly: true
                                wrapMode: Text.WordWrap
                                selectByMouse: true
                            }

                            // Label {
                            //     text: logDelegate.message
                            //     Layout.fillWidth: true
                            //     wrapMode: Text.WordWrap
                            //     font.family: "monospace"
                            //     color: Kirigami.Theme.textColor
                            // }
                        }
                    }

                    // Auto-scroll to bottom on new entries
                    onCountChanged: {
                        Qt.callLater(logView.positionViewAtEnd);
                    }
                }
            }
        }
    }
}
