#include "mainwindow.h"
#include "Application/ExplorerApplication.h"
#include <SlippyMap/SlippyMapWidgetMarker.h>
#include <QStyleFactory>

int main(int argc, char *argv[])
{
    QApplication::setOrganizationName("Robin Gingras");
    QApplication::setOrganizationDomain("robingingras.com");
    QApplication::setApplicationName("OSMExplorer");

    qRegisterMetaType<SlippyMap::SlippyMapWidgetMarker*>();

    ExplorerApplication a(argc, argv);
    a.setStyle(QStyleFactory::create("Fusion"));
    MainWindow w;
    w.show();

    return a.exec();
}
