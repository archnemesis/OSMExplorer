TEMPLATE = lib

CONFIG += plugin

QT     += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

INCLUDEPATH += ../../explorer

HEADERS += aprsfiexplorerplugin.h \
    aprsfilocationdataprovider.h \
    aprsdotficonfigurationdialog.h

SOURCES += aprsfiexplorerplugin.cpp \
    aprsfilocationdataprovider.cpp \
    aprsdotficonfigurationdialog.cpp

FORMS += aprsdotficonfigurationdialog.ui

win32 {
    INCLUDEPATH += "C:\Program Files\OpenSSL-1.0.2q-Win64\include"
    LIBS += -L"C:\Program Files\OpenSSL-1.0.2q-Win64\lib" -lssleay32 -llibeay32
}

macx {
    QMAKE_CXXFLAGS += -stdlib=libc++
    LIBS += -framework CoreFoundation
}

TARGET = $$qtLibraryTarget(aprsfi)
DESTDIR = ../../plugins

EXAMPLE_FILES = aprsfiplugin.json
