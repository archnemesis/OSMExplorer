#include "mainwindow.h"
#include "Window/TabbedMainWindow.h"
#include "Application/ExplorerApplication.h"
#include <SlippyMap/SlippyMapWidgetMarker.h>
#include <SlippyMap/SlippyMapLayerPolygon.h>
#include <SlippyMap/SlippyMapLayerPath.h>
#include <SlippyMap/SlippyMapAnimatedLayer.h>
#include <QStyleFactory>

#include "Map/SlippyMapLayerTrack.h"
#include "Network/ServerConnectionDialog.h"

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

    // todo: check if an IPC is already open for the app, that means we
    //  already have a process running and we should instead connect and
    //  send the link info


    ServerConnectionDialog dlg;
    int result = dlg.exec();

    if (result == QDialog::Accepted) {
        MainWindow w;
        w.show();
        return a.exec();
    }
}
