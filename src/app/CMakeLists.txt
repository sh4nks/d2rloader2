add_library(d2rloader_static STATIC)

ecm_add_qml_module(d2rloader_static URI com.someblocks.d2rloader
    OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/src/com/someblocks/d2rloader
    GENERATE_PLUGIN_SOURCE
    QML_FILES
        qml/Main.qml
        qml/AboutDialog.qml
        qml/ZoneEntry.qml
        qml/RealmProgress.qml
        qml/RealmProgressBar.qml
        qml/TerrorZones.qml
        qml/DiabloClone.qml
        qml/ApplicationLog.qml
    SOURCES
        gamemanager.h
        gamemanager.cpp
        gamelauncher.h
        gamelauncher.cpp
        gameinfo.h
        gameinfo.cpp
    DEPENDENCIES
        QtCore
        QtQml
        QtQuick
        org.kde.kirigami
        org.kde.ki18n
        org.kde.kirigamiaddons.statefulapp
        com.someblocks.d2rloader.settings
        com.someblocks.d2rloader.accounts
        com.someblocks.d2rloader.core
    IMPORTS
        ${EXTRA_IMPORTS}
)

if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    target_sources(d2rloader_static PRIVATE gamelauncher_linux.h gamelauncher_linux.cpp kwinwindowpositions.h kwinwindowpositions.cpp)
    qt_add_resources(d2rloader_static "kwin_scripts" PREFIX "/kwin" BASE kwin FILES kwin/windowpositions.js)
    target_link_libraries(d2rloader_static PRIVATE Qt6::DBus)
endif()

target_include_directories(d2rloader_static PRIVATE ${CMAKE_BINARY_DIR})
target_link_libraries(d2rloader_static PRIVATE d2rloader_staticplugin Accountsplugin Settingsplugin Coreplugin)
target_link_libraries(d2rloader_static PUBLIC
  Core
  Settings
  Qt6::Network
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
  KF6::ConfigCore
  KF6::Notifications
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

file(GLOB D2RLOADER_ICONS ${CMAKE_CURRENT_SOURCE_DIR}/icons/*-apps-com.someblocks.d2rloader.png)
ecm_add_app_icon(D2RLOADER_ICON ICONS ${D2RLOADER_ICONS})
target_sources(d2rloader PRIVATE ${D2RLOADER_ICON})

#target_compile_definitions(d2rloader PUBLIC -DHAVE_WINDOWSYSTEM)
#target_compile_definitions(d2rloader PUBLIC -DHAVE_X11=0)
#

target_compile_definitions(d2rloader_static PUBLIC APP_VERSION=\"${APP_VERSION}\")
set_target_properties(d2rloader PROPERTIES OUTPUT_NAME "d2rloader")
install(TARGETS d2rloader ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES d2rloader.notifyrc DESTINATION ${KDE_INSTALL_KNOTIFYRCDIR})
install(FILES com.someblocks.d2rloader.desktop DESTINATION ${KDE_INSTALL_APPDIR})
install(FILES com.someblocks.d2rloader.metainfo.xml DESTINATION ${KDE_INSTALL_METAINFODIR})
ecm_install_icons(ICONS ${D2RLOADER_ICONS} icons/sc-apps-com.someblocks.d2rloader.svg DESTINATION ${KDE_INSTALL_ICONDIR})
