cmake_minimum_required(VERSION 3.5)

project(aprsplugin VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt5 REQUIRED COMPONENTS Widgets Network Multimedia Core SerialPort)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Network Multimedia Core SerialPort)

set(PROJECT_SOURCES
    aprs_global.h
    aprsexplorerplugin.h
    aprsexplorerplugin.cpp
    aprsmessage.h
    aprsmessage.cpp
    aprsparser.h
    aprsparser.cpp)

add_library(aprsplugin MODULE ${PROJECT_SOURCES})

target_include_directories(aprsplugin PRIVATE ${CMAKE_SOURCE_DIR}/explorer)
target_include_directories(aprsplugin PRIVATE ${CMAKE_SOURCE_DIR}/SlippyMap/include)
target_include_directories(aprsplugin PRIVATE ${CMAKE_SOURCE_DIR}/LocationServices)
target_link_libraries(aprsplugin Qt5::Widgets)
target_link_libraries(aprsplugin Qt5::Network)
