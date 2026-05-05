qt_add_library(d2rloader STATIC
    d2rloader.h d2rloader.cpp

    profile.h profile.cpp
    profiletablemodel.h profiletablemodel.cpp
    profilemanager.h profilemanager.cpp

    gamemanager.h gamemanager.cpp
    settingsmanager.h settingsmanager.cpp
)

ecm_add_qml_module(d2rloader URI com.someblocks.d2rloader
    OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/src/com/someblocks/d2rloader
    GENERATE_PLUGIN_SOURCE
    QML_FILES
        qml/Main.qml
        qml/Heading.qml
        qml/AboutDialog.qml
        qml/AccountTable.qml
        qml/TerrorZones.qml
        qml/DiabloClone.qml
        qml/ApplicationLog.qml
    DEPENDENCIES
        QtCore
        QtQuick
    IMPORTS
        com.someblocks.d2rloader.settings
        ${EXTRA_IMPORTS}
)

qt_add_executable(d2rloader-app
    main.cpp
)
target_include_directories(d2rloader-app PRIVATE ${CMAKE_BINARY_DIR})
target_link_libraries(d2rloader-app PRIVATE
    d2rloader
    KF6::IconThemes
)

#ecm_add_app_icon(NEOCHAT_ICON ICONS ${CMAKE_SOURCE_DIR}/128-logo.png)
#target_sources(neochat-app PRIVATE ${NEOCHAT_ICON})

#target_compile_definitions(d2rloader PUBLIC -DHAVE_WINDOWSYSTEM)
#target_compile_definitions(d2rloader PUBLIC -DHAVE_X11=0)

target_include_directories(d2rloader PRIVATE ${CMAKE_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/models)
target_link_libraries(d2rloader PRIVATE d2rloaderplugin Settingsplugin)
target_link_libraries(d2rloader PUBLIC
  Settings
  Qt6::Quick
  Qt6::Qml
  Qt6::Gui
  Qt6::QuickControls2
  Qt6::Widgets
  KF6::I18n
  KF6::Kirigami
  KF6::CoreAddons
  KF6::IconThemes
  KF6::I18nQml
  KirigamiApp
  KirigamiAddonsComponents
)
target_compile_definitions(d2rloader PUBLIC APP_NAME=\"${APP_NAME}\" APP_VERSION=\"${APP_VERSION}\")
set_target_properties(d2rloader-app PROPERTIES OUTPUT_NAME "d2rloader")
install(TARGETS d2rloader-app ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
