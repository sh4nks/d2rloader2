pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick.Dialogs
import QtQuick.Window
import Qt.labs.platform as Platform
import org.kde.kirigami as Kirigami
import org.kde.ki18n

import com.someblocks.d2rloader.accounts as Accounts
import com.someblocks.d2rloader.settings as Settings
import com.someblocks.d2rloader.core

Kirigami.ApplicationWindow {
    id: mainWindow
    visible: true
    width: D2RLoaderConfig.appSize.width
    height: D2RLoaderConfig.appSize.height
    // title: `${Application.name}`

    // a resize fires per pixel, so the size is written once the drag has settled
    onWidthChanged: saveSizeTimer.restart()
    onHeightChanged: saveSizeTimer.restart()

    Timer {
        id: saveSizeTimer
        interval: 500
        onTriggered: mainWindow.saveSize()
    }

    function saveSize() {
        saveSizeTimer.stop();
        if (visibility !== Window.Windowed) {
            return;
        }
        if (width === D2RLoaderConfig.appSize.width && height === D2RLoaderConfig.appSize.height) {
            return;
        }
        D2RLoaderConfig.appSize = Qt.size(width, height);
        D2RLoaderConfig.save();
    }

    function quit() {
        saveSize();
        Qt.quit();
    }

    // set by the Help menu, whose check has no other place to show its result
    property bool reportUpdateCheck: false

    onClosing: close => {
        saveSize();
        if (!D2RLoaderConfig.closeToTray) {
            Qt.quit();
            return;
        }
        close.accepted = false;
        mainWindow.hide();
    }

    function toggleHidden() {
        if (visible) {
            hide();
            return;
        }
        // a window closed from the taskbar while minimized would come back minimized with show()
        showNormal();
        raise();
        requestActivate();
    }

    Shortcut {
        sequence: "Ctrl+M"
        onActivated: {
            D2RLoaderConfig.showMenuBar = !D2RLoaderConfig.showMenuBar;
            D2RLoaderConfig.save();
        }
    }

    Platform.SystemTrayIcon {
        id: trayIcon
        // Windows toasts hang off the tray icon, so notifications need it shown too
        visible: D2RLoaderConfig.closeToTray || (Qt.platform.os === "windows" && (D2RLoaderConfig.tzNotifications || D2RLoaderConfig.dcloneNotifications || D2RLoaderConfig.checkForUpdates))
        icon.name: "com.someblocks.d2rloader"
        // used where the icon theme does not know the app icon
        icon.source: "qrc:/icons/64-apps-com.someblocks.d2rloader.png"
        tooltip: Application.displayName
        onActivated: reason => {
            if (reason === Platform.SystemTrayIcon.Trigger) {
                mainWindow.toggleHidden();
            }
        }

        menu: Platform.Menu {
            // Plasma's SystemTrayMenu applies this to its QMenu widget, so the
            // default of true pops the menu open the moment the icon is shown.
            // The menu itself still reaches the tray over DBus.
            visible: false

            Platform.MenuItem {
                text: mainWindow.visible ? KI18n.i18nc("@action:inmenu", "Hide &Window") : KI18n.i18nc("@action:inmenu", "Show &Window")
                onTriggered: mainWindow.toggleHidden()
            }
            Platform.MenuItem {
                text: KI18n.i18nc("@action:inmenu", "&Settings")
                icon.name: "settings-configure"
                onTriggered: Settings.SettingsWindow.open()
            }
            Platform.MenuSeparator {}
            Platform.MenuItem {
                text: KI18n.i18nc("@action:inmenu", "&Exit")
                icon.name: "application-exit"
                onTriggered: mainWindow.quit()
            }
        }
    }

    Connections {
        target: GameInfo

        function onNotificationRequested(title: string, text: string, critical: bool) {
            trayIcon.showMessage(title, text, critical ? Platform.SystemTrayIcon.Warning : Platform.SystemTrayIcon.Information);
        }
    }

    Connections {
        target: UpdateChecker

        function onNotificationRequested(title: string, text: string) {
            trayIcon.showMessage(title, text, Platform.SystemTrayIcon.Information);
        }

        function onCheckFinished() {
            if (mainWindow.reportUpdateCheck) {
                mainWindow.reportUpdateCheck = false;
                mainWindow.showPassiveNotification(UpdateChecker.statusText);
            }
        }
    }

    Component {
        id: debugMenu

        Menu {
            title: "&Debug"

            MenuItem {
                text: "Simulate Terror Zone change"
                onTriggered: GameInfo.debugTerrorZoneChange()
            }
            MenuItem {
                text: "Simulate next Terror Zone prediction"
                onTriggered: GameInfo.debugNextTerrorZone()
            }
            MenuItem {
                text: "Simulate Diablo Clone progress"
                onTriggered: GameInfo.debugDCloneProgress(false)
            }
            MenuItem {
                text: "Simulate Diablo walking the earth"
                onTriggered: GameInfo.debugDCloneProgress(true)
            }
        }
    }

    // only with --debug on the command line
    Component.onCompleted: {
        if (GameInfo.debugMode) {
            mainMenuBar.addMenu(debugMenu.createObject(mainMenuBar));
        }
    }

    menuBar: MenuBar {
        id: mainMenuBar
        visible: D2RLoaderConfig.showMenuBar

        Menu {
            title: KI18n.i18nc("@title:menu", "&Settings")
            MenuItem {
                text: KI18n.i18nc("@action:inmenu", "&Settings")
                icon.name: "settings-configure"
                onTriggered: Settings.SettingsWindow.open()
            }
            MenuSeparator {}
            MenuItem {
                text: KI18n.i18nc("@action:inmenu", "&Load Account Settings...")
                icon.name: "document-open"
                onTriggered: importDialog.open()
            }
            MenuItem {
                text: KI18n.i18nc("@action:inmenu", "&Save Account Settings...")
                icon.name: "username-copy"
                onTriggered: exportDialog.open()
            }
            MenuSeparator {}
            MenuItem {
                text: KI18n.i18nc("@action:inmenu", "&Exit")
                icon.name: "application-exit"
                onTriggered: mainWindow.quit()
            }
        }
        Menu {
            title: KI18n.i18nc("@title:menu", "&Help")

            MenuItem {
                text: KI18n.i18nc("@action:inmenu", "&Open README")
                icon.name: "help-contents"
                onTriggered: Qt.openUrlExternally("https://github.com/sh4nks/d2rloader2#readme")
            }
            MenuSeparator {}
            MenuItem {
                text: KI18n.i18nc("@action:inmenu", "Check for &Updates")
                icon.name: "view-refresh"
                visible: !UpdateChecker.managedExternally
                height: visible ? implicitHeight : 0
                enabled: !UpdateChecker.checking
                onTriggered: {
                    mainWindow.reportUpdateCheck = true;
                    UpdateChecker.checkNow();
                }
            }
            MenuItem {
                text: KI18n.i18nc("@action:inmenu", "&About...")
                icon.name: "help-about"
                onTriggered: Settings.SettingsWindow.show("about")
            }
        }
    }

    ColumnLayout {
        anchors.fill: parent
        // Kirigami's contentItem fills the whole window and only makes room for header, not menuBar
        anchors.topMargin: mainWindow.menuBar.visible ? mainWindow.menuBar.height : 0
        spacing: 0

        Kirigami.InlineMessage {
            Layout.fillWidth: true
            Layout.margins: Kirigami.Units.smallSpacing
            type: Kirigami.MessageType.Error
            text: Accounts.ProfileManager.loadError
            visible: text.length > 0
        }

        Kirigami.InlineMessage {
            Layout.fillWidth: true
            Layout.margins: Kirigami.Units.smallSpacing
            type: Kirigami.MessageType.Error
            text: Accounts.LaunchSequenceManager.loadError
            visible: text.length > 0
        }

        Kirigami.InlineMessage {
            Layout.fillWidth: true
            Layout.margins: Kirigami.Units.smallSpacing
            type: Kirigami.MessageType.Information
            text: Accounts.ProfileManager.notice
            visible: text.length > 0
            showCloseButton: true
        }

        Kirigami.InlineMessage {
            Layout.fillWidth: true
            Layout.margins: Kirigami.Units.smallSpacing
            type: Kirigami.MessageType.Information
            text: GameManager.launchProgress
            visible: text.length > 0
        }

        Kirigami.InlineMessage {
            Layout.fillWidth: true
            Layout.margins: Kirigami.Units.smallSpacing
            type: Kirigami.MessageType.Positive
            text: UpdateChecker.installStatus.length > 0 ? UpdateChecker.installStatus : KI18n.i18nc("@info new version", "D2RLoader %1 is available.", UpdateChecker.latestVersion)
            visible: UpdateChecker.updateAvailable && !UpdateChecker.dismissed
            actions: [
                Kirigami.Action {
                    text: KI18n.i18nc("@action:button", "Install")
                    icon.name: "system-software-update"
                    visible: UpdateChecker.canInstall && !UpdateChecker.installing
                    onTriggered: UpdateChecker.installUpdate()
                },
                Kirigami.Action {
                    text: KI18n.i18nc("@action:button", "Cancel")
                    icon.name: "dialog-cancel"
                    visible: UpdateChecker.installing
                    onTriggered: UpdateChecker.cancelInstall()
                },
                Kirigami.Action {
                    text: KI18n.i18nc("@action:button", "Download")
                    icon.name: "cloud-download"
                    visible: !UpdateChecker.canInstall
                    onTriggered: Qt.openUrlExternally(UpdateChecker.releaseUrl)
                },
                Kirigami.Action {
                    text: KI18n.i18nc("@action:button", "Skip This Version")
                    icon.name: "dialog-cancel"
                    visible: !UpdateChecker.installing
                    onTriggered: UpdateChecker.dismiss()
                }
            ]
        }

        // Fixed Upper Section: Accounts Table
        Accounts.AccountTable {
            id: accountTable
            modelData: Accounts.ProfileManager
            Layout.topMargin: 0
            Layout.fillWidth: true
            Layout.fillHeight: true
            onSettingsClicked: Settings.SettingsWindow.open()
            onAddAccountClicked: Settings.SettingsWindow.openAccountEditor()
            onEditAccountClicked: rowIndex => {
                Settings.SettingsWindow.openAccountEditor(rowIndex);
            }
            onStartStopClicked: rowIndex => {
                GameManager.toggle(Accounts.ProfileManager.getProfile(rowIndex));
            }
            sequenceRunning: GameManager.sequenceRunning
            onLaunchSequenceClicked: sequenceIndex => GameManager.startSequence(sequenceIndex)
            onCancelSequenceClicked: GameManager.cancelSequence()
        }

        Connections {
            target: GameManager

            function onLaunchFailed(profileName: string, error: string) {
                mainWindow.showPassiveNotification(KI18n.i18nc("@info %1 is the account name, %2 the reason", "Could not start %1: %2", profileName, error), "long");
            }
        }

        // TabBar beneath the accounts table
        TabBar {
            id: tabBar
            Layout.fillWidth: true
            visible: D2RLoaderConfig.showInfoPanel

            TabButton {
                implicitHeight: Kirigami.Units.gridUnit * 2
                text: KI18n.i18nc("@title:tab", "Terror Zones")
                icon.name: "view-calendar-day"
            }
            TabButton {
                implicitHeight: Kirigami.Units.gridUnit * 2
                text: KI18n.i18nc("@title:tab", "Diablo Clone")
                icon.name: "view-media-artist"
            }
            TabButton {
                implicitHeight: Kirigami.Units.gridUnit * 2
                text: KI18n.i18nc("@title:tab", "Application Log")
                icon.name: "utilities-log-viewer"
            }
        }

        // Content Area for Tabs. Not a StackLayout: hidden Kirigami cards
        // neither lay out nor report their height, so every tab stays visible
        // to size the area by the tallest one and only the current one shows.
        Item {
            Layout.fillWidth: true
            visible: D2RLoaderConfig.showInfoPanel
            implicitHeight: Math.max(terrorZonesTab.implicitHeight, diabloCloneTab.implicitHeight, applicationLogTab.implicitHeight)

            TerrorZones {
                id: terrorZonesTab
                anchors.fill: parent
                opacity: tabBar.currentIndex === 0 ? 1 : 0
                enabled: tabBar.currentIndex === 0
            }

            DiabloClone {
                id: diabloCloneTab
                anchors.fill: parent
                opacity: tabBar.currentIndex === 1 ? 1 : 0
                enabled: tabBar.currentIndex === 1
            }

            ApplicationLog {
                id: applicationLogTab
                anchors.fill: parent
                opacity: tabBar.currentIndex === 2 ? 1 : 0
                enabled: tabBar.currentIndex === 2
            }
        }

        RowLayout {
            Layout.fillWidth: true
            Layout.leftMargin: Kirigami.Units.smallSpacing
            Layout.rightMargin: Kirigami.Units.smallSpacing
            Layout.topMargin: Kirigami.Units.smallSpacing
            Layout.bottomMargin: Kirigami.Units.smallSpacing
            spacing: Kirigami.Units.largeSpacing
            visible: D2RLoaderConfig.showInfoPanel

            Label {
                text: GameInfo.lastUpdated.getTime() > 0 ? KI18n.i18nc("@info", "Last updated: %1", Qt.formatTime(GameInfo.lastUpdated, "hh:mm:ss")) : KI18n.i18nc("@info", "Not fetched yet")
                font: Kirigami.Theme.smallFont
                opacity: 0.6
                elide: Text.ElideRight
                Layout.fillWidth: true
            }

            Label {
                text: KI18n.i18nc("@info game data source", "Terror Zone and Diablo Clone data provided by d2emu.com")
                font: Kirigami.Theme.smallFont
                opacity: 0.6
                elide: Text.ElideRight
            }
        }
    }

    FileDialog {
        id: importDialog
        title: KI18n.i18nc("@title:window", "Load Account Settings")
        nameFilters: [KI18n.i18nc("@item", "JSON files (*.json)"), KI18n.i18nc("@item", "All files (*)")]
        onAccepted: {
            importModePrompt.file = importDialog.selectedFile;
            importModePrompt.open();
        }
    }

    FileDialog {
        id: exportDialog
        title: KI18n.i18nc("@title:window", "Save Account Settings")
        fileMode: FileDialog.SaveFile
        defaultSuffix: "json"
        nameFilters: [KI18n.i18nc("@item", "JSON files (*.json)"), KI18n.i18nc("@item", "All files (*)")]
        onAccepted: {
            const error = Accounts.ProfileManager.exportAccounts(exportDialog.selectedFile);
            if (error.length > 0) {
                mainWindow.showPassiveNotification(KI18n.i18nc("@info %1 is the reason", "Could not save the accounts: %1", error), "long");
            } else {
                mainWindow.showPassiveNotification(KI18n.i18ncp("@info", "Saved %1 account.", "Saved %1 accounts.", Accounts.ProfileManager.rowCount()));
            }
        }
    }

    Kirigami.PromptDialog {
        id: importModePrompt

        property url file

        title: KI18n.i18nc("@title:window", "Load Account Settings")
        subtitle: KI18n.i18nc("@info", "Add the accounts in this file to the existing ones, or replace the existing accounts with them?")
        standardButtons: Kirigami.Dialog.Cancel
        showCloseButton: false

        customFooterActions: [
            Kirigami.Action {
                text: KI18n.i18nc("@action:button", "Add")
                icon.name: "list-add"
                onTriggered: mainWindow.importAccounts(importModePrompt.file, false)
            },
            Kirigami.Action {
                text: KI18n.i18nc("@action:button", "Replace")
                icon.name: "document-replace"
                onTriggered: mainWindow.importAccounts(importModePrompt.file, true)
            }
        ]
    }

    function importAccounts(file: url, replace: bool) {
        importModePrompt.close();
        const previousCount = replace ? 0 : Accounts.ProfileManager.rowCount();
        const error = Accounts.ProfileManager.importAccounts(file, replace);
        if (error.length > 0) {
            mainWindow.showPassiveNotification(KI18n.i18nc("@info %1 is the reason", "Could not load the accounts: %1", error), "long");
            return;
        }
        mainWindow.showPassiveNotification(KI18n.i18ncp("@info", "Loaded %1 account.", "Loaded %1 accounts.", Accounts.ProfileManager.rowCount() - previousCount));
    }
}
