add_executable(d2rloader)

ecm_add_qml_module(d2rloader
    URI
    com.someblocks.d2rloader
)

target_sources(d2rloader
    PRIVATE
    main.cpp
    d2rloader.h d2rloader.cpp

    profile.h profile.cpp
    profiletablemodel.h profiletablemodel.cpp
    profilemanager.h profilemanager.cpp

    gamemanager.h gamemanager.cpp
    settingsmanager.h settingsmanager.cpp
)

ecm_target_qml_sources(d2rloader
    SOURCES
    qml/Main.qml
    qml/Heading.qml
    qml/AboutDialog.qml
    qml/AccountTable.qml
    qml/TerrorZones.qml
    qml/DiabloClone.qml
    qml/ApplicationLog.qml
    qml/SettingsWindow.qml
    qml/ApplicationSettingsPage.qml
    qml/AccountSettingsPage.qml
    qml/DiabloCloneSettingsPage.qml
    qml/AccountEditorPage.qml
    qml/GameSettingsPage.qml
    qml/LaunchSequencePage.qml
    qml/LaunchSequenceEditorPage.qml
)

target_link_libraries(d2rloader
    PRIVATE
    Qt6::Quick
    Qt6::Qml
    Qt6::Gui
    Qt6::QuickControls2
    Qt6::Widgets
    KF6::I18n
    KF6::Kirigami
    KF6::CoreAddons
    KF6::IconThemes
    KirigamiApp
    KirigamiAddonsComponents
)

target_compile_definitions(d2rloader PUBLIC APP_NAME=\"${APP_NAME}\" APP_VERSION=\"${APP_VERSION}\")

install(TARGETS d2rloader ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
