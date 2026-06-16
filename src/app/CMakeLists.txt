add_library(d2rloader_static STATIC
  d2rloader.h d2rloader.cpp

  profilemanager.h profilemanager.cpp
)

ecm_add_qml_module(d2rloader_static URI com.someblocks.d2rloader
    OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/src/com/someblocks/d2rloader
    GENERATE_PLUGIN_SOURCE
    QML_FILES
        qml/Main.qml
        qml/Heading.qml
        qml/AboutDialog.qml
        qml/TerrorZones.qml
        qml/DiabloClone.qml
        qml/ApplicationLog.qml
    SOURCES
        gamemanager.h
        gamemanager.cpp
        settingsmanager.h
        settingsmanager.cpp
    DEPENDENCIES
        QtCore
        QtQml
        QtQuick
        org.kde.kirigamiaddons.statefulapp
    IMPORTS
        com.someblocks.d2rloader.settings
        com.someblocks.d2rloader.accounts
        ${EXTRA_IMPORTS}
)

target_include_directories(d2rloader_static PRIVATE ${CMAKE_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/models)
target_link_libraries(d2rloader_static PRIVATE d2rloader_staticplugin Accountsplugin Settingsplugin)
target_link_libraries(d2rloader_static PUBLIC
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
  KirigamiAddonsStatefulApp
)

add_executable(d2rloader
    main.cpp
)
target_include_directories(d2rloader PRIVATE ${CMAKE_BINARY_DIR})
target_link_libraries(d2rloader PRIVATE
    d2rloader_static
    KF6::IconThemes
)

#ecm_add_app_icon(NEOCHAT_ICON ICONS ${CMAKE_SOURCE_DIR}/128-logo.png)
#target_sources(neochat-app PRIVATE ${NEOCHAT_ICON})

#target_compile_definitions(d2rloader PUBLIC -DHAVE_WINDOWSYSTEM)
#target_compile_definitions(d2rloader PUBLIC -DHAVE_X11=0)
#

target_compile_definitions(d2rloader_static PUBLIC APP_NAME=\"${APP_NAME}\" APP_VERSION=\"${APP_VERSION}\")
set_target_properties(d2rloader PROPERTIES OUTPUT_NAME "d2rloader")
install(TARGETS d2rloader ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
