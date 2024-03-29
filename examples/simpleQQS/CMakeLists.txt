cmake_minimum_required(VERSION 3.1.0)

# Require C++14
set(CMAKE_AUTOMOC ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


# Configure Qt
find_package(QT NAMES Qt6 Qt5 COMPONENTS Core Gui QuickControls2 REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core Gui QuickControls2 REQUIRED)
message(${CMAKE_BINARY_DIR}/qml/QtQuickStream/resources/View)
set (QML_IMPORT_PATH ${CMAKE_BINARY_DIR}/qml/QtQuickStream/resources/View)
set(QT_QML_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/qml)

# Creates and finalizes an application target of a platform-specific type
qt_add_executable(simpleQQS main.cpp)

# This command defines a QML module that can consist of C++ sources, .qml files, or both.
qt_add_qml_module(simpleQQS
    URI "simpleQQS"
    VERSION 1.0

   QML_FILES
    main.qml

   SOURCES

)

target_include_directories(simpleQQS PUBLIC
    Qt${QT_VERSION_MAJOR}::QuickControls2)

target_compile_definitions(simpleQQS
    PRIVATE $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:QT_QML_DEBUG>)

target_link_libraries(simpleQQS PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::QuickControls2
    QtQuickStreamplugin
)


#set(QML_IMPORT_PATH ${QT_QML_OUTPUT_DIRECTORY} CACHE STRING "QtCreator QML Modules Lookup")
