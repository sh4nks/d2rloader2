add_executable(d2rloader)

ecm_add_qml_module(d2rloader
    URI
    org.someblocks.d2rloader
)

target_sources(d2rloader
    PRIVATE
    main.cpp
    d2rloader.h d2rloader.cpp

    profile.h
    profilemanager.h profilemanager.cpp

    gamemanager.h gamemanager.cpp
    settingsmanager.h settingsmanager.cpp

    book.h book.cpp
    booktablemodel.h booktablemodel.cpp
)

ecm_target_qml_sources(d2rloader
    SOURCES
    qml/Main.qml
    qml/Heading.qml
    qml/AboutDialog.qml
    qml/AccountTable.qml
)

target_link_libraries(d2rloader
    PRIVATE
    Qt6::Quick
    Qt6::Qml
    Qt6::Gui
    Qt6::QuickControls2
    Qt6::Widgets
    KF6::I18n
    KF6::CoreAddons
    KF6::IconThemes
)

target_compile_definitions(d2rloader PUBLIC APP_NAME=\"${APP_NAME}\" APP_VERSION=\"${APP_VERSION}\")

install(TARGETS d2rloader ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
