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
        title: KI18n.i18nc("@title", "Application Log")

        Button {
            text: KI18n.i18nc("@action:button", "Clear Logs")
            icon.name: "edit-clear-list"
            onClicked: LogBuffer.clear()
        }
    }
    contentItem: ColumnLayout {
        spacing: 0

        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true

            Kirigami.Theme.colorSet: Kirigami.Theme.View
            Kirigami.Theme.inherit: true

            color: Kirigami.Theme.backgroundColor
            border.color: Qt.rgba(Kirigami.Theme.textColor.r, Kirigami.Theme.textColor.g, Kirigami.Theme.textColor.b, 0.1)
            radius: 2

            ScrollView {
                id: logScroll
                anchors.fill: parent
                clip: true

                TextEdit {
                    id: logText

                    readOnly: true
                    selectByMouse: true
                    textFormat: TextEdit.RichText
                    wrapMode: TextEdit.NoWrap
                    color: Kirigami.Theme.textColor
                    padding: Kirigami.Units.largeSpacing

                    // The colours are baked into the HTML, so a theme change
                    // needs the whole document built again.
                    property color themeColor: Kirigami.Theme.textColor
                    onThemeColorChanged: logText.rebuild()

                    function escapeHtml(text: string): string {
                        return text.replace(/&/g, "&amp;").replace(/</g, "&lt;").replace(/>/g, "&gt;");
                    }

                    // Qt's HTML subset does not take a colour with an alpha
                    // channel, which is what toString() gives for one.
                    function opaque(color): string {
                        return Qt.rgba(color.r, color.g, color.b, 1).toString();
                    }

                    function levelColor(level: string): color {
                        if (level === "DEBUG")
                            return Kirigami.Theme.disabledTextColor;
                        if (level === "ERROR" || level === "FATAL")
                            return Kirigami.Theme.negativeTextColor;
                        if (level === "WARNING")
                            return Kirigami.Theme.neutralTextColor;
                        return Kirigami.Theme.positiveTextColor;
                    }

                    function entryHtml(timestamp: string, level: string, message: string): string {
                        const style = "margin-top:0; margin-bottom:0; font-family:'" + Kirigami.Theme.fixedWidthFont.family + "'";
                        return "<pre style=\"" + style + "\">" + "<span style=\"color:" + logText.opaque(Kirigami.Theme.disabledTextColor) + "\">" + timestamp + "</span>  "
                            + "<b style=\"color:" + logText.opaque(logText.levelColor(level)) + "\">" + (level + "       ").substring(0, 7) + "</b>  " + logText.escapeHtml(message) + "</pre>";
                    }

                    function appendEntry(timestamp: string, level: string, message: string) {
                        const wasAtEnd = logText.atEnd();
                        // append() starts a new paragraph; insert() would merge
                        // the entry into the last one.
                        logText.append(logText.entryHtml(timestamp, level, message));
                        if (wasAtEnd) {
                            Qt.callLater(logText.scrollToEnd);
                        }
                    }

                    function rebuild() {
                        const entries = LogBuffer.entries();
                        let html = "";
                        for (let i = 0; i < entries.length; ++i) {
                            html += logText.entryHtml(entries[i].timestamp, entries[i].level, entries[i].message);
                        }
                        logText.text = html;
                        Qt.callLater(logText.scrollToEnd);
                    }

                    function atEnd(): bool {
                        const bar = logScroll.ScrollBar.vertical;
                        return bar.size >= 1 || bar.position + bar.size >= 0.999;
                    }

                    function scrollToEnd() {
                        const bar = logScroll.ScrollBar.vertical;
                        bar.position = Math.max(0, 1 - bar.size);
                    }

                    Component.onCompleted: logText.rebuild()

                    Connections {
                        target: LogBuffer

                        function onEntryAdded(timestamp: string, level: string, message: string) {
                            logText.appendEntry(timestamp, level, message);
                        }

                        function onReset() {
                            logText.rebuild();
                        }
                    }
                }
            }
        }
    }
}
