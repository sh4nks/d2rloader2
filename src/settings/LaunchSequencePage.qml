pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard
import org.kde.ki18n
import com.someblocks.d2rloader.accounts as Accounts
import com.someblocks.d2rloader.core

FormCard.FormCardPage {
    id: root

    title: KI18n.i18nc("@title", "Launch Sequences")

    function editSequence(index: int, name: string, profileIds: var) {
        (root.QQC2.ApplicationWindow.window as Kirigami.ApplicationWindow).pageStack.layers.push(sequenceEditorComponent, {
            sequenceIndex: index,
            sequenceName: name,
            selectedIds: Array.from(profileIds)
        });
    }

    FormCard.FormHeader {
        title: KI18n.i18nc("@title:group", "Managed Sequences")
    }

    FormCard.FormCard {
        Repeater {
            id: sequenceRepeater
            model: Accounts.LaunchSequenceManager

            delegate: FormCard.AbstractFormDelegate {
                id: sequenceDelegate
                required property int index
                required property string name
                required property var profileIds
                required property int accountCount

                onClicked: root.editSequence(sequenceDelegate.index, sequenceDelegate.name, sequenceDelegate.profileIds)

                contentItem: RowLayout {
                    spacing: Kirigami.Units.largeSpacing

                    Kirigami.Icon {
                        source: "media-playlist-play"
                        Layout.preferredWidth: Kirigami.Units.iconSizes.medium
                        Layout.preferredHeight: Kirigami.Units.iconSizes.medium
                    }

                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: Kirigami.Units.smallSpacing

                        QQC2.Label {
                            Layout.fillWidth: true
                            text: sequenceDelegate.name
                            font.bold: true
                            elide: Text.ElideRight
                        }

                        QQC2.Label {
                            Layout.fillWidth: true
                            text: KI18n.i18ncp("@info", "%1 account", "%1 accounts", sequenceDelegate.accountCount)
                            color: Kirigami.Theme.disabledTextColor
                            font: Kirigami.Theme.smallFont
                            elide: Text.ElideRight
                        }
                    }

                    RowLayout {
                        spacing: 0
                        QQC2.ToolButton {
                            icon.name: "arrow-up"
                            QQC2.ToolTip.visible: hovered && enabled
                            QQC2.ToolTip.text: KI18n.i18nc("@info:tooltip", "Move Up")
                            enabled: sequenceDelegate.index > 0
                            onClicked: Accounts.LaunchSequenceManager.moveUp(sequenceDelegate.index)
                        }
                        QQC2.ToolButton {
                            icon.name: "arrow-down"
                            QQC2.ToolTip.visible: hovered && enabled
                            QQC2.ToolTip.text: KI18n.i18nc("@info:tooltip", "Move Down")
                            enabled: sequenceDelegate.index < sequenceRepeater.count - 1
                            onClicked: Accounts.LaunchSequenceManager.moveDown(sequenceDelegate.index)
                        }
                    }

                    QQC2.ToolButton {
                        icon.name: "edit-delete"
                        QQC2.ToolTip.visible: hovered
                        QQC2.ToolTip.text: KI18n.i18nc("@info:tooltip", "Delete Sequence")
                        onClicked: {
                            deletePrompt.row = sequenceDelegate.index;
                            deletePrompt.sequenceName = sequenceDelegate.name;
                            deletePrompt.open();
                        }
                    }

                    FormCard.FormArrow {
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    }
                }
            }
        }

        FormCard.FormDelegateSeparator {
            above: addSequenceDelegate
            visible: sequenceRepeater.count > 0
        }

        FormCard.FormButtonDelegate {
            id: addSequenceDelegate
            text: KI18n.i18nc("@action:button", "Add New Sequence")
            icon.name: "list-add"
            onClicked: root.editSequence(-1, "", [])
        }
    }

    FormCard.FormHeader {
        title: KI18n.i18nc("@title:group", "Sequence Behavior")
    }

    FormCard.FormCard {
        FormCard.FormSpinBoxDelegate {
            label: KI18n.i18nc("@label", "Launch Delay")
            description: KI18n.i18nc("@info:label", "Seconds to wait after an account has started before starting the next one.")
            from: 1
            to: 300
            stepSize: 1
            value: D2RLoaderConfig.sequenceDelay
            onValueChanged: if (value !== D2RLoaderConfig.sequenceDelay) {
                D2RLoaderConfig.sequenceDelay = value;
                D2RLoaderConfig.save();
            }
        }

        FormCard.FormDelegateSeparator {}

        FormCard.FormCheckDelegate {
            text: KI18n.i18nc("@label", "Stop Sequence on Failure")
            description: KI18n.i18nc("@info:label", "Prevents further accounts from launching if one fails to start.")
            checked: D2RLoaderConfig.sequenceStopOnFailure
            onToggled: {
                D2RLoaderConfig.sequenceStopOnFailure = checked;
                D2RLoaderConfig.save();
            }
        }

        FormCard.FormDelegateSeparator {}

        FormCard.FormCheckDelegate {
            text: KI18n.i18nc("@label", "Randomize Delay")
            description: KI18n.i18nc("@info:label", "Adds up to half the launch delay at random.")
            checked: D2RLoaderConfig.sequenceRandomizeDelay
            onToggled: {
                D2RLoaderConfig.sequenceRandomizeDelay = checked;
                D2RLoaderConfig.save();
            }
        }
    }

    Kirigami.PromptDialog {
        id: deletePrompt

        property int row: -1
        property string sequenceName

        title: KI18n.i18nc("@title:window", "Delete Launch Sequence")
        subtitle: KI18n.i18nc("@info", "Delete the launch sequence \"%1\"?", deletePrompt.sequenceName)
        standardButtons: Kirigami.Dialog.Cancel
        showCloseButton: false

        customFooterActions: [
            Kirigami.Action {
                text: KI18n.i18nc("@action:button", "Delete")
                icon.name: "edit-delete"
                onTriggered: {
                    Accounts.LaunchSequenceManager.remove(deletePrompt.row);
                    deletePrompt.close();
                }
            }
        ]
    }

    Component {
        id: sequenceEditorComponent
        LaunchSequenceEditorPage {}
    }
}
