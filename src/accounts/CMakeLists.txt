add_library(Accounts STATIC)

ecm_add_qml_module(Accounts GENERATE_PLUGIN_SOURCE
    URI com.someblocks.d2rloader.accounts
    OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/src/com/someblocks/d2rloader/accounts
    QML_FILES
        AccountTable.qml
    SOURCES
        models/profiletablemodel.h
        models/profiletablemodel.cpp
        models/profile.h
        models/profile.cpp
        models/region.h
        models/region.cpp
        models/authmethod.h
        models/authmethod.cpp
)

target_include_directories(Accounts PRIVATE ${CMAKE_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/models)
target_link_libraries(Accounts PRIVATE
    Qt::Core
    Qt::Quick
    KF6::Kirigami
)
