add_library(Settings STATIC)

set_source_files_properties(
    SettingsWindow.qml
    PROPERTIES
        QT_QML_SINGLETON_TYPE TRUE
)

ecm_add_qml_module(Settings GENERATE_PLUGIN_SOURCE
    URI com.someblocks.d2rloader.settings
    OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/src/com/someblocks/d2rloader/settings
    QML_FILES
        SettingsWindow.qml
        ApplicationSettingsPage.qml
        AccountSettingsPage.qml
        DiabloCloneSettingsPage.qml
        AccountEditorPage.qml
        GameSettingsPage.qml
        LootFilterPage.qml
        LaunchSequencePage.qml
        LaunchSequenceEditorPage.qml
    DEPENDENCIES
        QtQuick
        QtQuick.Controls
        org.kde.kirigami
        org.kde.ki18n
        org.kde.kirigamiaddons.formcard
        org.kde.kirigamiaddons.settings
        com.someblocks.d2rloader.accounts
        com.someblocks.d2rloader.core
    SOURCES
        about.h
)

target_include_directories(Settings PRIVATE ${CMAKE_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/models)
target_link_libraries(Settings PRIVATE
    KF6::CoreAddons
    KF6::ColorScheme
    KF6::I18n
    KF6::ConfigCore
)
