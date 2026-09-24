add_library(Core STATIC)

set_source_files_properties(
    Log.qml
    PROPERTIES
        QT_QML_SINGLETON_TYPE TRUE
)

ecm_add_qml_module(Core GENERATE_PLUGIN_SOURCE
    URI com.someblocks.d2rloader.core
    OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/src/com/someblocks/d2rloader/core
    QML_FILES
        Log.qml
        CardHeader.qml
    DEPENDENCIES
        QtCore
        QtQml
        QtQuick
        org.kde.kirigami
    SOURCES
        logbuffer.h
        logbuffer.cpp
)

target_sources(Core PRIVATE
    logging.h
    logging.cpp
)

kconfig_target_kcfg_file(Core
    FILE d2rloaderconfig.kcfg
    CLASS_NAME D2RLoaderConfig
    MUTATORS
    DEFAULT_VALUE_GETTERS
    NOTIFIERS
    GENERATE_PROPERTIES
    SINGLETON
    QML_REGISTRATION
    GENERATE_MOC
)

target_include_directories(Core PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(Core PUBLIC
    Qt::Core
    Qt::Qml
    Qt::Quick
    KF6::ConfigCore
    KF6::ConfigGui
)
