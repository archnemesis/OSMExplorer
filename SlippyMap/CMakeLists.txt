cmake_minimum_required(VERSION 3.28.3)
project(SlippyMap VERSION 1.0.1 DESCRIPTION "SlippyMap Tiled Map Widget Library")

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Network Multimedia SerialPort)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Network Multimedia SerialPort)

export(PACKAGE SlippyMap)

add_library(SlippyMap
        defaults.h
        slippymap_global.h
        slippymapellipse.cpp
        slippymapellipse.h
        slippymaplayer.cpp
        slippymaplayer.h
        slippymaplayermanager.cpp
        slippymaplayermanager.h
        slippymaplayermarker.cpp
        slippymaplayermarker.h
        slippymaplayermarkerpropertypage.cpp
        slippymaplayermarkerpropertypage.h
        slippymaplayerobject.cpp
        slippymaplayerobject.h
        slippymaplayerobjectpropertypage.cpp
        slippymaplayerobjectpropertypage.h
        slippymaplayerobjectprovider.cpp
        slippymaplayerobjectprovider.h
        slippymaplayerpolygon.cpp
        slippymaplayerpolygon.h
        slippymaplayerpolygonpropertypage.cpp
        slippymaplayerpolygonpropertypage.h
        slippymaprenderer.cpp
        slippymaprenderer.h
        slippymapwidget.cpp
        slippymapwidget.h
        slippymapwidgetlayer.cpp
        slippymapwidgetlayer.h
        slippymapwidgetmarker.cpp
        slippymapwidgetmarker.h
        slippymapwidgetmarkergroup.cpp
        slippymapwidgetmarkergroup.h
        slippymapwidgetmarkermodel.cpp
        slippymapwidgetmarkermodel.h
        slippymapwidgetmarkerprovider.cpp
        slippymapwidgetmarkerprovider.h
)

include(GenerateExportHeader)
generate_export_header(SlippyMap)

target_include_directories(SlippyMap PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/SlippyMap)
target_link_libraries(SlippyMap Qt5::Widgets)
target_link_libraries(SlippyMap Qt5::Network)

set_target_properties(SlippyMap PROPERTIES VERSION ${PROJECT_VERSION})
set_target_properties(SlippyMap PROPERTIES SOVERSION ${PROJECT_VERSION_MAJOR})

set(SLIPPYMAP_PUBLIC_HEADERS
        slippymap_global.h
        slippymapellipse.h
        slippymaplayer.h
        slippymaplayermanager.h
        slippymaplayermarker.h
        slippymaplayermarkerpropertypage.h
        slippymaplayerobject.h
        slippymaplayerobjectpropertypage.h
        slippymaplayerobjectprovider.h
        slippymaplayerpolygon.h
        slippymaplayerpolygonpropertypage.h
        slippymaprenderer.h
        slippymapwidget.h
        slippymapwidgetlayer.h
        slippymapwidgetmarker.h
        slippymapwidgetmarkergroup.h
        slippymapwidgetmarkermodel.h
        slippymapwidgetmarkerprovider.h
)

set_target_properties(SlippyMap PROPERTIES PUBLIC_HEADER "${SLIPPYMAP_PUBLIC_HEADERS}")