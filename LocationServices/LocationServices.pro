#-------------------------------------------------
#
# Project created by QtCreator 2019-02-25T16:38:42
#
#-------------------------------------------------

TEMPLATE = lib

CONFIG += plugin
CONFIG += c++11

QT       += core network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = $$qtLibraryTarget(LocationServices)

DEFINES += LOCATIONSERVICES_LIBRARY
DEFINES += QT_DEPRECATED_WARNINGS
DEFINES += EXPORT_SYMBOLS

SOURCES += \
        locationdataprovider.cpp

HEADERS += \
        locationdataprovider.h \
        locationservices_global.h 

unix {
    target.path = /usr/lib
    INSTALLS += target
}
