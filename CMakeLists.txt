cmake_minimum_required(VERSION 3.20)

project(D2RLoader VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

string(TIMESTAMP CURRENT_TIMESTAMP "%s" UTC)
set(APP_NAME ${CMAKE_PROJECT_NAME})
set(APP_VERSION ${CMAKE_PROJECT_VERSION})
set(APP_VERSION_CODE ${TIMESTAMP})
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(QT_QML_GENERATE_QMLLS_INI ON)

################################################################################

message(STATUS "[CMAKE]   CMAKE_VERSION             = ${CMAKE_VERSION}")
message(STATUS "[CMAKE]   CMAKE_PREFIX_PATH         = ${CMAKE_PREFIX_PATH}")
message(STATUS "[SYSTEM]  (HOST) SYSTEM NAME        = ${CMAKE_HOST_SYSTEM_NAME}")
message(STATUS "[SYSTEM]  (HOST) SYSTEM VERSION     = ${CMAKE_HOST_SYSTEM_VERSION}")
message(STATUS "[SYSTEM]  (TARGET) SYSTEM NAME      = ${CMAKE_SYSTEM_NAME}")
message(STATUS "[SYSTEM]  (TARGET) SYSTEM VERSION   = ${CMAKE_SYSTEM_VERSION}")
message(STATUS "[PROJECT] NAME                      = ${CMAKE_PROJECT_NAME}")
message(STATUS "[PROJECT] VERSION                   = ${CMAKE_PROJECT_VERSION}")
message(STATUS "[PROJECT] SOURCE_DIR                = ${CMAKE_SOURCE_DIR}")
message(STATUS "[PROJECT] BUILD_TYPE                = ${CMAKE_BUILD_TYPE}")
message(STATUS "[PROJECT] TOOLCHAIN_FILE            = ${CMAKE_TOOLCHAIN_FILE}")
message(STATUS "[PROJECT] QT_HOST_PATH              = ${QT_HOST_PATH}")

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Svg Qml Quick QuickControls2)
# find_package(KF6 REQUIRED COMPONENTS
#     CoreAddons # For KAboutData
#     KirigamiAddons # For KAboutData
# )
qt_standard_project_setup(REQUIRES 6.9)

qt_add_executable(${CMAKE_PROJECT_NAME}
    src/main.cpp
    src/d2rloader.h src/d2rloader.cpp

    src/profile.h
    src/profilemanager.h src/profilemanager.cpp

    src/gamemanager.h src/gamemanager.cpp
    src/settingsmanager.h src/settingsmanager.cpp

    src/book.h src/book.cpp
    src/booklistmodel.h src/booklistmodel.cpp
    src/booktablemodel.h src/booktablemodel.cpp

    qml/qml.qrc
)

qt_add_qml_module(${CMAKE_PROJECT_NAME}
    URI ${CMAKE_PROJECT_NAME}
    NO_PLUGIN
    QML_FILES
        qml/Main.qml
        qml/Heading.qml
        qml/AboutDialog.qml
        qml/AccountTable.qml

        qml/ktableview/private/TableCellDelegate.qml
        qml/ktableview/private/HeaderDelegate.qml
        qml/ktableview/private/AbstractTable.qml
        qml/ktableview/private/AbstractHeaderComponent.qml
        qml/ktableview/MyTableView.qml
        qml/ktableview/HeaderComponent.qml
)

target_link_libraries(${CMAKE_PROJECT_NAME}
    PRIVATE Qt6::Core
            Qt6::Widgets
            Qt6::Quick
            Qt6::QuickControls2
)

target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE src/)
target_compile_definitions(D2RLoader PUBLIC APP_NAME=\"${APP_NAME}\" APP_VERSION=\"${APP_VERSION}\")

set_target_properties(${CMAKE_PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin"
)

# Output?
install(TARGETS ${CMAKE_PROJECT_NAME}
    BUNDLE DESTINATION "${CMAKE_SOURCE_DIR}/bin"
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

# include(GNUInstallDirs)
install(TARGETS ${CMAKE_PROJECT_NAME}
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# Output?
qt_generate_deploy_qml_app_script(
    TARGET ${CMAKE_PROJECT_NAME}
    OUTPUT_SCRIPT deploy_script
    MACOS_BUNDLE_POST_BUILD
    NO_UNSUPPORTED_PLATFORM_ERROR
    DEPLOY_USER_QML_MODULES_ON_UNSUPPORTED_PLATFORM
)
install(SCRIPT ${deploy_script})

include(FeatureSummary)
feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)
