cmake_minimum_required(VERSION 3.5)

project(osmexplorer VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

cmake_policy(SET CMP0074 NEW)
cmake_policy(SET CMP0153 OLD)

find_package(QT NAMES Qt5 REQUIRED COMPONENTS Widgets Network Multimedia Core SerialPort Charts)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Network Multimedia Core SerialPort Charts)

add_subdirectory(SlippyMap)
add_subdirectory(LocationServices)
add_subdirectory(plugins)
add_subdirectory(externals/Qt-Color-Widgets)

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
        explorer/Map/gpx/gpxcopyright.cpp
        explorer/Map/gpx/gpxcopyright.h
        explorer/Map/gpx/gpxmetadata.cpp
        explorer/Map/gpx/gpxmetadata.h
        explorer/Map/gpx/gpxparser.cpp
        explorer/Map/gpx/gpxparser.h
        explorer/Map/gpx/gpxperson.cpp
        explorer/Map/gpx/gpxperson.h
        explorer/Map/gpx/gpxtrack.cpp
        explorer/Map/gpx/gpxtrack.h
        explorer/Map/gpx/gpxtracklink.cpp
        explorer/Map/gpx/gpxtracklink.h
        explorer/Map/gpx/gpxtracksegment.cpp
        explorer/Map/gpx/gpxtracksegment.h
        explorer/Map/gpx/gpxwaypoint.cpp
        explorer/Map/gpx/gpxwaypoint.h
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
        explorer/resources.qrc
        explorer/settingsdialog.cpp
        explorer/settingsdialog.h
        explorer/settingsdialog.ui
        explorer/Map/SlippyMapLayerTrack.cpp
        explorer/Map/SlippyMapLayerTrack.h
        explorer/Map/SlippyMapLayerTrackPropertyPage.cpp
        explorer/Map/SlippyMapLayerTrackPropertyPage.h
        explorer/slippymaplayerobjectprovider.cpp
        explorer/slippymaplayerobjectprovider.h
        explorer/textlogviewerform.cpp
        explorer/textlogviewerform.h
        explorer/textlogviewerform.ui
        explorer/Weather/NationalWeatherServiceInterface.cpp
        explorer/Weather/NationalWeatherServiceInterface.h
        explorer/Weather/WeatherStationPropertyPage.cpp
        explorer/Weather/WeatherStationPropertyPage.h
        explorer/Weather/WeatherStationMarker.cpp
        explorer/Weather/WeatherStationMarker.h
        explorer/Weather/WeatherForecastWindow.cpp
        explorer/Weather/WeatherForecastWindow.h
        explorer/Weather/ForecastPeriodWidget.cpp
        explorer/Weather/ForecastPeriodWidget.h
        explorer/Application/ExplorerApplication.cpp
        explorer/Application/ExplorerApplication.h
        explorer/Application/PluginManager.cpp
        explorer/Application/PluginManager.h
        explorer/Map/SlippyMapTreeView.cpp
        explorer/Map/SlippyMapTreeView.h
        explorer/Map/SlippyMapLayerTrackStylePropertyPage.cpp
        explorer/Map/SlippyMapLayerTrackStylePropertyPage.h
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
    add_executable(osmexplorer
            ${PROJECT_SOURCES}
    )
endif()

target_link_libraries(osmexplorer PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)
target_link_libraries(osmexplorer PRIVATE Qt${QT_VERSION_MAJOR}::Network)
target_link_libraries(osmexplorer PRIVATE Qt${QT_VERSION_MAJOR}::Multimedia)
target_link_libraries(osmexplorer PRIVATE Qt${QT_VERSION_MAJOR}::SerialPort)
target_link_libraries(osmexplorer PRIVATE Qt${QT_VERSION_MAJOR}::Charts)
target_link_libraries(osmexplorer PRIVATE SlippyMap)
target_link_libraries(osmexplorer PRIVATE LocationServices)
target_link_libraries(osmexplorer PRIVATE QtColorWidgets)

target_include_directories(osmexplorer PRIVATE ${CMAKE_SOURCE_DIR}/explorer)
target_include_directories(osmexplorer PRIVATE ${CMAKE_SOURCE_DIR}/SlippyMap/include)
target_include_directories(osmexplorer PRIVATE ${CMAKE_SOURCE_DIR}/LocationServices)
target_include_directories(osmexplorer PRIVATE ${CMAKE_SOURCE_DIR}/externals/Qt-Color-Widgets/include)

set_target_properties(osmexplorer PROPERTIES
        MACOSX_BUNDLE_GUI_IDENTIFIER osmexplorer.robingingras.com
        MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
        MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
        MACOSX_BUNDLE TRUE
        WIN32_EXECUTABLE TRUE
)

install(TARGETS osmexplorer
        BUNDLE DESTINATION .)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(osmexplorer)
endif()
