cmake_minimum_required(VERSION 3.5)

project(osmexplorer VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

cmake_policy(SET CMP0074 NEW)

find_package(QT NAMES Qt5 REQUIRED COMPONENTS Widgets Network Multimedia Core SerialPort Charts)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Network Multimedia Core SerialPort Charts)

add_subdirectory(SlippyMap)
add_subdirectory(LocationServices)
add_subdirectory(plugins)

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
        explorer/gpx/gpxcopyright.cpp
        explorer/gpx/gpxcopyright.h
        explorer/gpx/gpxmetadata.cpp
        explorer/gpx/gpxmetadata.h
        explorer/gpx/gpxparser.cpp
        explorer/gpx/gpxparser.h
        explorer/gpx/gpxperson.cpp
        explorer/gpx/gpxperson.h
        explorer/gpx/gpxtrack.cpp
        explorer/gpx/gpxtrack.h
        explorer/gpx/gpxtracklink.cpp
        explorer/gpx/gpxtracklink.h
        explorer/gpx/gpxtracksegment.cpp
        explorer/gpx/gpxtracksegment.h
        explorer/gpx/gpxwaypoint.cpp
        explorer/gpx/gpxwaypoint.h
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
        explorer/SlippyMapLayerTrack.cpp
        explorer/SlippyMapLayerTrack.h
        explorer/PropertyPage/SlippyMapLayerObjectPropertyPage.cpp
        explorer/PropertyPage/SlippyMapLayerObjectPropertyPage.h
        explorer/slippymaplayerobjectprovider.cpp
        explorer/slippymaplayerobjectprovider.h
        explorer/PropertyPage/SlippyMapLayerPolygonPropertyPage.cpp
        explorer/PropertyPage/SlippyMapLayerPolygonPropertyPage.h
        explorer/PropertyPage/SlippyMapLayerTrackPropertyPage.cpp
        explorer/PropertyPage/SlippyMapLayerTrackPropertyPage.h
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
        explorer/PropertyPage/SlippyMapLayerMarkerPropertyPage.cpp
        explorer/PropertyPage/SlippyMapLayerMarkerPropertyPage.h
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

target_include_directories(osmexplorer PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/explorer)
target_include_directories(osmexplorer PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/SlippyMap/include)
target_include_directories(osmexplorer PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/LocationServices)

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
