#include "mainwindow.h"
#include "Window/TabbedMainWindow.h"
#include "Application/ExplorerApplication.h"
#include <SlippyMap/SlippyMapWidgetMarker.h>
#include <SlippyMap/SlippyMapLayerPolygon.h>
#include <SlippyMap/SlippyMapLayerPath.h>
#include <SlippyMap/SlippyMapAnimatedLayer.h>
#include <QStyleFactory>

#include "Map/SlippyMapLayerTrack.h"

int main(int argc, char *argv[])
{
    QApplication::setOrganizationName("Robin Gingras");
    QApplication::setOrganizationDomain("robingingras.com");
    QApplication::setApplicationName("OSMExplorer");
//    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
//    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);

    qRegisterMetaType<SlippyMap::SlippyMapWidgetMarker*>();
    qRegisterMetaType<SlippyMap::SlippyMapLayerPolygon*>();
    qRegisterMetaType<SlippyMap::SlippyMapLayerPath*>();
    qRegisterMetaType<SlippyMapLayerTrack*>();
    qRegisterMetaType<SlippyMap::SlippyMapAnimatedLayer*>();

    ExplorerApplication a(argc, argv);
    a.setStyle(QStyleFactory::create("Fusion"));
    MainWindow w;
//    Explorer::TabbedMainWindow w;
    w.show();

    return a.exec();
}
