import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import org.kde.kirigami as Kirigami
import org.kde.ki18n

/**
 * A Diablo Clone progress value rendered as a bar with its "n/max" reading.
 * Progress 0 means the realm has not been reported on yet.
 */
RowLayout {
    id: root

    property int progress: 0
    property int maxProgress: 6

    spacing: Kirigami.Units.smallSpacing

    ProgressBar {
        value: root.maxProgress > 0 ? root.progress / root.maxProgress : 0
        Layout.fillWidth: true

        Kirigami.Theme.colorSet: Kirigami.Theme.View
        Kirigami.Theme.inherit: false
    }

    Label {
        text: root.progress > 0 ? KI18n.i18nc("@info:status Diablo Clone progress, e.g. 4/6", "%1/%2", root.progress, root.maxProgress) : KI18n.i18nc("@info:status no Diablo Clone data for this realm", "-")
        color: Kirigami.Theme.textColor
        opacity: 0.7
        font.family: Kirigami.Theme.fixedWidthFont.family
        Layout.preferredWidth: Kirigami.Units.gridUnit * 2
        horizontalAlignment: Text.AlignRight
    }
}
