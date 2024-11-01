#include "mainwindow.h"
#include "Application/ExplorerApplication.h"
#include <SlippyMap/SlippyMapWidgetMarker.h>
#include <SlippyMap/SlippyMapLayerPolygon.h>
#include <SlippyMap/SlippyMapAnimatedLayer.h>
#include <QStyleFactory>

int main(int argc, char *argv[])
{
    QApplication::setOrganizationName("Robin Gingras");
    QApplication::setOrganizationDomain("robingingras.com");
    QApplication::setApplicationName("OSMExplorer");

    qRegisterMetaType<SlippyMap::SlippyMapWidgetMarker*>();
    qRegisterMetaType<SlippyMap::SlippyMapLayerPolygon*>();
    qRegisterMetaType<SlippyMap::SlippyMapAnimatedLayer*>();

    ExplorerApplication a(argc, argv);
    a.setStyle(QStyleFactory::create("Fusion"));
    MainWindow w;
    w.show();

    return a.exec();
}
