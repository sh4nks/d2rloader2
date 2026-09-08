add_library(Accounts STATIC)

ecm_add_qml_module(Accounts GENERATE_PLUGIN_SOURCE
    URI com.someblocks.d2rloader.accounts
    OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/src/com/someblocks/d2rloader/accounts
    QML_FILES
        AccountTable.qml
        CellDelegate.qml
        CellDelegateAction.qml
        CellDelegateComboBox.qml
        CellDelegateLabel.qml
        CellDelegateStatus.qml
    DEPENDENCIES
        QtQuick
        QtQuick.Controls
        org.kde.kirigami
    SOURCES
        profilemanager.h
        profilemanager.cpp
        profile.h
        profile.cpp
        region.h
        region.cpp
        authmethod.h
        authmethod.cpp
)

target_include_directories(Accounts PRIVATE ${CMAKE_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/models)
target_link_libraries(Accounts PRIVATE
    Qt::Core
    Qt::Quick
    KF6::I18n
    KF6::Kirigami
    KF6::ConfigCore
)
