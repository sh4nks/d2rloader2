cmake_minimum_required(VERSION 3.20)

set(RELEASE_SERVICE_VERSION_MAJOR "0")
set(RELEASE_SERVICE_VERSION_MINOR "1")
set(RELEASE_SERVICE_VERSION_MICRO "0")
set(RELEASE_SERVICE_VERSION "${RELEASE_SERVICE_VERSION_MAJOR}.${RELEASE_SERVICE_VERSION_MINOR}.${RELEASE_SERVICE_VERSION_MICRO}")

project(D2RLoader VERSION ${RELEASE_SERVICE_VERSION})

find_package(ECM 6.0.0 REQUIRED NO_MODULE)

set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

include(FeatureSummary)
include(ECMSetupVersion)
include(KDEInstallDirs)
include(ECMFindQmlModule)
include(KDECMakeSettings)
include(ECMAddTests)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(ECMAddAppIcon)
include(ECMInstallIcons)
include(KDEGitCommitHooks)
include(ECMCheckOutboundLicense)
include(ECMQtDeclareLoggingCategory)
include(ECMAddAndroidApk)
include(ECMQmlModule)
include(ECMDeprecationSettings)
include(GenerateExportHeader)
include(ECMGenerateHeaders)
include(KDEClangFormat)


set(APP_VERSION ${CMAKE_PROJECT_VERSION})
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(QT_QML_GENERATE_QMLLS_INI ON)

ecm_setup_version(${PROJECT_VERSION}
    VARIABLE_PREFIX D2RLOADER
    VERSION_HEADER ${CMAKE_CURRENT_BINARY_DIR}/d2rloader-version.h
)

message(STATUS "[ECM]     ECM_MODULE_PATH    = ${ECM_MODULE_PATH}")
message(STATUS "[KDE]     KDE_INSTALL_BINDIR = ${KDE_INSTALL_BINDIR}")
message(STATUS "[CMAKE]   CMAKE_VERSION      = ${CMAKE_VERSION}")
message(STATUS "[SYSTEM]  SYSTEM NAME        = ${CMAKE_SYSTEM_NAME}")
message(STATUS "[SYSTEM]  SYSTEM VERSION     = ${CMAKE_SYSTEM_VERSION}")
message(STATUS "[PROJECT] NAME               = ${CMAKE_PROJECT_NAME}")
message(STATUS "[PROJECT] VERSION            = ${CMAKE_PROJECT_VERSION}")
message(STATUS "[PROJECT] SOURCE_DIR         = ${CMAKE_SOURCE_DIR}")
message(STATUS "[PROJECT] BUILD_TYPE         = ${CMAKE_BUILD_TYPE}")

find_package(Qt6 REQUIRED COMPONENTS
    Core
    Network
    Quick
    Gui
    QuickControls2
    Widgets
)

# keeps the game windows in place through a KWin script on Plasma
if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    find_package(Qt6 REQUIRED COMPONENTS DBus)
endif()

if (QT_KNOWN_POLICY_QTP0004)
  qt_policy(SET QTP0004 NEW)
endif ()

find_package(KF6 REQUIRED COMPONENTS
    Kirigami
    KirigamiAddons
    I18n
    Notifications
    ColorScheme
    IconThemes
    CoreAddons
    Config
    QQC2DesktopStyle
)

add_definitions(-DQT_NO_FOREACH)

add_subdirectory(src)

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)

file(GLOB_RECURSE ALL_CLANG_FORMAT_SOURCE_FILES src/*.cpp src/*.h)
kde_clang_format(${ALL_CLANG_FORMAT_SOURCE_FILES})
kde_configure_git_pre_commit_hook(CHECKS CLANG_FORMAT)
