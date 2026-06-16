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
        LaunchSequencePage.qml
        LaunchSequenceEditorPage.qml
)

target_include_directories(Settings PRIVATE ${CMAKE_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/models)
target_link_libraries(Settings PRIVATE
    KF6::ColorScheme
    KF6::I18n
)
