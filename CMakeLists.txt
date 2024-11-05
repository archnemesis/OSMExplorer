cmake_minimum_required(VERSION 3.30)

set(OSMEXPLORER_VERSION_MAJOR 1)
set(OSMEXPLORER_VERSION_MINOR 0)
set(OSMEXPLORER_VERSION_REVISION 0)
project(osmexplorer VERSION "1.0.0" LANGUAGES C CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

cmake_policy(SET CMP0074 NEW)
cmake_policy(SET CMP0153 OLD)

if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    enable_language("RC")
    set(WIN32_RESOURCES ${CMAKE_CURRENT_SOURCE_DIR}/explorer/osmexplorer.rc)
endif()

find_package(QT NAMES Qt5 REQUIRED COMPONENTS Widgets Network Multimedia Core SerialPort Charts Sql)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Network Multimedia Core SerialPort Charts Sql)

add_subdirectory(SlippyMap EXCLUDE_FROM_ALL)
add_subdirectory(LocationServices EXCLUDE_FROM_ALL)
add_subdirectory(plugins)
add_subdirectory(externals/nmea)

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
        explorer/Map/SlippyMapGpsMarker.cpp
        explorer/Map/SlippyMapGpsMarker.h
        explorer/Map/SlippyMapGpsMarkerPropertyPage.cpp
        explorer/Map/SlippyMapGpsMarkerPropertyPage.h
        explorer/Weather/SlippyMapWeatherStationMarker.cpp
        explorer/Weather/SlippyMapWeatherStationMarker.h
        explorer/Application/HistoryManager.cpp
        explorer/Application/HistoryManager.h
        explorer/Dialog/databaseconnectiondialog.cpp
        explorer/Dialog/databaseconnectiondialog.h
        explorer/Dialog/databaseconnectiondialog.ui
        explorer/Application/DatabaseManager.cpp
        explorer/Application/DatabaseManager.h
        explorer/Database/DatabaseObjectPropertyPage.cpp
        explorer/Database/DatabaseObjectPropertyPage.h
        explorer/config.h
        explorer/Network/ServerInterface.cpp
        explorer/Network/ServerInterface.h
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
            ${WIN32_RESOURCES}
    )
endif()

target_link_libraries(osmexplorer PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)
target_link_libraries(osmexplorer PRIVATE Qt${QT_VERSION_MAJOR}::Network)
target_link_libraries(osmexplorer PRIVATE Qt${QT_VERSION_MAJOR}::Multimedia)
target_link_libraries(osmexplorer PRIVATE Qt${QT_VERSION_MAJOR}::SerialPort)
target_link_libraries(osmexplorer PRIVATE Qt${QT_VERSION_MAJOR}::Charts)
target_link_libraries(osmexplorer PRIVATE Qt${QT_VERSION_MAJOR}::Sql)
target_link_libraries(osmexplorer PRIVATE SlippyMap)
target_link_libraries(osmexplorer PRIVATE LocationServices)
target_link_libraries(osmexplorer PRIVATE QtColorWidgets)
target_link_libraries(osmexplorer PRIVATE nmea)

target_include_directories(osmexplorer PRIVATE ${CMAKE_SOURCE_DIR}/explorer)
target_include_directories(osmexplorer PRIVATE ${CMAKE_SOURCE_DIR}/SlippyMap/include)
target_include_directories(osmexplorer PRIVATE ${CMAKE_SOURCE_DIR}/SlippyMap/externals/Qt-Color-Widgets/include)
target_include_directories(osmexplorer PRIVATE ${CMAKE_SOURCE_DIR}/LocationServices)
target_include_directories(osmexplorer PRIVATE ${CMAKE_SOURCE_DIR}/externals/nmea/include)
target_include_directories(osmexplorer PRIVATE "C:/boost_1_86_0")

set_target_properties(osmexplorer PROPERTIES
        MACOSX_BUNDLE_GUI_IDENTIFIER osmexplorer.robingingras.com
        MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
        MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
        MACOSX_BUNDLE TRUE
        WIN32_EXECUTABLE TRUE
)

if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    install(TARGETS osmexplorer RUNTIME DESTINATION .)
    install(TARGETS rainviewerplugin LIBRARY DESTINATION plugins)
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    install(TARGETS osmexplorer RUNTIME DESTINATION bin)
    install(TARGETS rainviewerplugin LIBRARY DESTINATION lib)
endif()

install(TARGETS osmexplorer RUNTIME DESTINATION .)
install(TARGETS rainviewerplugin LIBRARY DESTINATION plugins)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(osmexplorer)
endif()

if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    set(CMAKE_INSTALL_SYSTEM_RUNTIME_DESTINATION .)
    include(InstallRequiredSystemLibraries)
    set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/License.txt")
    set(CPACK_PACKAGE_NAME "OSMExplorer")
    set(CPACK_PACKAGE_VERSION_MAJOR "${OSMEXPLORER_VERSION_MAJOR}")
    set(CPACK_PACKAGE_VERSION_MINOR "${OSMEXPLORER_VERSION_MINOR}")
    set(CPACK_WIX_VERSION "4")
    set(CPACK_GENERATOR "WIX")
    set(CPACK_SOURCE_GENERATOR "TGZ")
    set(CPACK_WIX_PRODUCT_ICON "${CMAKE_CURRENT_SOURCE_DIR}/explorer/resources/icon.ico")
    set(CPACK_WIX_UI_BANNER "${CMAKE_CURRENT_SOURCE_DIR}/explorer/resources/banner.png")
    set(CPACK_WIX_UI_DIALOG "${CMAKE_CURRENT_SOURCE_DIR}/explorer/resources/dialog.png")
    include(CPack)
endif ()