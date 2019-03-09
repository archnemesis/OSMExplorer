#-------------------------------------------------
#
# Project created by QtCreator 2019-02-10T13:27:28
#
#-------------------------------------------------

QT       += core gui network serialport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = OSMExplorer
TEMPLATE = app

QMAKE_PROJECT_NAME = OSMExplorer

DEPENDPATH += \
    ../SlippyMap \
    ../LocationServices
INCLUDEPATH += ../SlippyMap
INCLUDEPATH += ../LocationServices

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../SlippyMap/release/ -lSlippyMap
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../SlippyMap/debug/ -lSlippyMap
else:unix: LIBS += -L$$OUT_PWD/../SlippyMap/ -lSlippyMap

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../LocationServices/release/ -lLocationServices
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../LocationServices/debug/ -lLocationServices
else:unix: LIBS += -L$$OUT_PWD/../LocationServices/ -lLocationServices

win32 {
    INCLUDEPATH += "C:\Program Files\OpenSSL-1.0.2q-Win64\include"
    LIBS += -L"C:\Program Files\OpenSSL-1.0.2q-Win64\lib" -lssleay32 -llibeay32
}

macx {
    QMAKE_CXXFLAGS += -stdlib=libc++
    LIBS += -framework CoreFoundation
}

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++11

SOURCES += \
        main.cpp \
        mainwindow.cpp \
    markerdialog.cpp \
    markerlistitemwidget.cpp \
    directionlistitemwidget.cpp \
    settingsdialog.cpp \
    layerpropertiesdialog.cpp \
    gpssourcedialog.cpp \
    nmeaseriallocationdataprovider.cpp \
    textlogviewerform.cpp \
    polygonshapepropertiesform.cpp

HEADERS += \
        mainwindow.h \
    markerdialog.h \
    markerlistitemwidget.h \
    directionlistitemwidget.h \
    settingsdialog.h \
    defaults.h \
    layerpropertiesdialog.h \
    gpssourcedialog.h \
    nmeaseriallocationdataprovider.h \
    textlogviewerform.h \
    explorerplugininterface.h \
    polygonshapepropertiesform.h

FORMS += \
        mainwindow.ui \
    markerdialog.ui \
    markerlistitemwidget.ui \
    directionlistitemwidget.ui \
    settingsdialog.ui \
    layerpropertiesdialog.ui \
    gpssourcedialog.ui \
    textlogviewerform.ui \
    polygonshapepropertiesform.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    resources.qrc
