cmake_minimum_required(VERSION 3.5)

project(rainviewerplugin VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt5 REQUIRED COMPONENTS Widgets Network Multimedia Core SerialPort)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Network Multimedia Core SerialPort)

set(PROJECT_SOURCES
        rainviewer_global.h
        rainviewerplugin.h
        rainviewerplugin.cpp
        RainViewerTileLayer.h
        RainViewerTileLayer.cpp
)

add_library(rainviewerplugin MODULE ${PROJECT_SOURCES})

target_include_directories(rainviewerplugin PRIVATE ${CMAKE_SOURCE_DIR}/explorer)
target_include_directories(rainviewerplugin PRIVATE ${CMAKE_SOURCE_DIR}/SlippyMap/include)
target_include_directories(rainviewerplugin PRIVATE ${CMAKE_SOURCE_DIR}/LocationServices)
target_link_libraries(rainviewerplugin Qt5::Widgets)
target_link_libraries(rainviewerplugin Qt5::Network)
target_link_libraries(rainviewerplugin SlippyMap)
