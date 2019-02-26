TEMPLATE = lib

CONFIG += plugin
CONFIG += c++11

QT     += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

INCLUDEPATH += ../../explorer
INCLUDEPATH += ../../SlippyMap
INCLUDEPATH += ../../LocationServices

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../../SlippyMap/release/ -lSlippyMap
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../../SlippyMap/debug/ -lSlippyMap
else:unix: LIBS += -L$$OUT_PWD/../../SlippyMap/ -lSlippyMap

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../../LocationServices/release/ -lLocationServices
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../../LocationServices/debug/ -lLocationServices
else:unix: LIBS += -L$$OUT_PWD/../../LocationServices/ -lLocationServices

HEADERS += aprsfiexplorerplugin.h \
    aprsfilocationdataprovider.h \
    aprsdotficonfigurationdialog.h \
    aprsstationlistform.h \
    aprsfimarkerprovider.h

SOURCES += aprsfiexplorerplugin.cpp \
    aprsfilocationdataprovider.cpp \
    aprsdotficonfigurationdialog.cpp \
    aprsstationlistform.cpp \
    aprsfimarkerprovider.cpp

FORMS += aprsdotficonfigurationdialog.ui \
    aprsstationlistform.ui

win32 {
    INCLUDEPATH += "C:\Program Files\OpenSSL-1.0.2q-Win64\include"
    LIBS += -L"C:\Program Files\OpenSSL-1.0.2q-Win64\lib" -lssleay32 -llibeay32
}

TARGET = $$qtLibraryTarget(aprsfi)
DESTDIR = ../../plugins

EXAMPLE_FILES = aprsfiplugin.json
