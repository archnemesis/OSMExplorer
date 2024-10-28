cmake_minimum_required(VERSION 3.5)

project(osmexplorer VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_PREFIX_PATH "C:\\Qt-Binary\\5.15.2\\mingw81_64")

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Network Multimedia SerialPort)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Network Multimedia SerialPort)

add_subdirectory(SlippyMap)
add_subdirectory(LocationServices)

set(PROJECT_SOURCES
        explorer/main.cpp
        explorer/mainwindow.cpp
        explorer/mainwindow.h
        explorer/mainwindow.ui
        explorer/directionlistitemwidget.cpp
        explorer/directionlistitemwidget.h
        explorer/directionlistitemwidget.ui
        explorer/explorerplugininterface.h
        explorer/gpssourcedialog.cpp
        explorer/gpssourcedialog.h
        explorer/gpssourcedialog.ui
        explorer/gpxcopyright.cpp
        explorer/gpxcopyright.h
        explorer/gpxmetadata.cpp
        explorer/gpxmetadata.h
        explorer/gpxparser.cpp
        explorer/gpxparser.h
        explorer/gpxperson.cpp
        explorer/gpxperson.h
        explorer/gpxtrack.cpp
        explorer/gpxtrack.h
        explorer/gpxtracklink.cpp
        explorer/gpxtracklink.h
        explorer/gpxtracksegment.cpp
        explorer/gpxtracksegment.h
        explorer/gpxwaypoint.cpp
        explorer/gpxwaypoint.h
        explorer/layerpropertiesdialog.cpp
        explorer/layerpropertiesdialog.h
        explorer/layerpropertiesdialog.ui
        explorer/mapdataimportdialog.cpp
        explorer/mapdataimportdialog.h
        explorer/mapdataimportdialog.ui
        explorer/markerdialog.cpp
        explorer/markerdialog.h
        explorer/markerdialog.ui
        explorer/markerlistitemwidget.cpp
        explorer/markerlistitemwidget.h
        explorer/markerlistitemwidget.ui
        explorer/nmeaseriallocationdataprovider.cpp
        explorer/nmeaseriallocationdataprovider.h
        explorer/polygonpropertypageform.cpp
        explorer/polygonpropertypageform.h
        explorer/polygonpropertypageform.ui
        explorer/polygonpropertypagewidget.cpp
        explorer/polygonpropertypagewidget.h
        explorer/polygonshapepropertiesform.cpp
        explorer/polygonshapepropertiesform.h
        explorer/polygonshapepropertiesform.ui
        explorer/resources.qrc
        explorer/settingsdialog.cpp
        explorer/settingsdialog.h
        explorer/settingsdialog.ui
        explorer/shapepropertypagewidget.cpp
        explorer/shapepropertypagewidget.h
        explorer/slippymaplayertrack.cpp
        explorer/slippymaplayertrack.h
        explorer/slippymaplayertrackpropertypageform.cpp
        explorer/slippymaplayertrackpropertypageform.h
        explorer/slippymaplayertrackpropertypageform.ui
        explorer/textlogviewerform.cpp
        explorer/textlogviewerform.h
        explorer/textlogviewerform.ui
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(osmexplorer
            MANUAL_FINALIZATION
            ${PROJECT_SOURCES}
    )
    # Define target properties for Android with Qt 6 as:
    #    set_property(TARGET IntercomDesktop APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
    #                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
    # For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(osmexplorer SHARED
                ${PROJECT_SOURCES}
        )
        # Define properties for Android with Qt 5 after find_package() calls as:
        #    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(osmexplorer
                ${PROJECT_SOURCES}
        )
    endif()
endif()

target_link_libraries(osmexplorer PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)
target_link_libraries(osmexplorer PRIVATE Qt${QT_VERSION_MAJOR}::Network)
target_link_libraries(osmexplorer PRIVATE Qt${QT_VERSION_MAJOR}::Multimedia)
target_link_libraries(osmexplorer PRIVATE Qt${QT_VERSION_MAJOR}::SerialPort)
target_link_libraries(osmexplorer PRIVATE SlippyMap)
target_link_libraries(osmexplorer PRIVATE LocationServices)

target_include_directories(osmexplorer PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/explorer)
target_include_directories(osmexplorer PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/SlippyMap)
target_include_directories(osmexplorer PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/LocationServices)

set_target_properties(osmexplorer PROPERTIES
        MACOSX_BUNDLE_GUI_IDENTIFIER osmexplorer.robingingras.com
        MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
        MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
        MACOSX_BUNDLE TRUE
        WIN32_EXECUTABLE TRUE
)

install(TARGETS osmexplorer
        BUNDLE DESTINATION .
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(osmexplorer)
endif()
