cmake_minimum_required(VERSION 3.28.3)
project(LocationServices VERSION 1.0.1 DESCRIPTION "Location services library")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

if(CMAKE_VERSION VERSION_LESS "3.7.0")
    set(CMAKE_INCLUDE_CURRENT_DIR ON)
endif()

find_package(Qt5 COMPONENTS Widgets REQUIRED)
find_package(Qt5 COMPONENTS Network REQUIRED)

add_library(LocationServices
        locationdataprovider.cpp
        locationdataprovider.h
        locationservices_global.h
)

target_include_directories(LocationServices PRIVATE ${CMAKE_SOURCE_DIR})
target_link_libraries(LocationServices Qt5::Widgets)
target_link_libraries(LocationServices Qt5::Network)

set_target_properties(LocationServices PROPERTIES VERSION ${PROJECT_VERSION})
set_target_properties(LocationServices PROPERTIES SOVERSION ${PROJECT_VERSION_MAJOR})

set(LOCATIONSERVICES_PUBLIC_HEADERS
        locationdataprovider.h
)

set_target_properties(LocationServices PROPERTIES PUBLIC_HEADER "${LOCATIONSERVICES_PUBLIC_HEADERS}")