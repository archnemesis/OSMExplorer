#include "aprsexplorerplugin.h"

#include <QMenu>
#include <QAction>
#include <QDialog>
#include <QList>
#include <QDockWidget>

#include <SlippyMap/SlippyMapLayer.h>
#include <SlippyMap/SlippyMapWidgetLayer.h>

APRSExplorerPlugin::APRSExplorerPlugin()
{

}

QString APRSExplorerPlugin::name() const
{
    return tr("APRS");
}

QString APRSExplorerPlugin::description() const
{
    return tr("Stream APRS data from a serial-enabled receiver");
}

QString APRSExplorerPlugin::authorName() const
{
    return tr("Robin Gingras");
}

QString APRSExplorerPlugin::homepage() const
{
    return tr("https://www.robingingras.com");
}

QList<QAction *> APRSExplorerPlugin::mapContextMenuActionList()
{
    return QList<QAction *>();
}

QList<QMenu *> APRSExplorerPlugin::mainMenuList()
{
    return QList<QMenu *>();
}

QList<QDockWidget *> APRSExplorerPlugin::dockWidgetList()
{
    return QList<QDockWidget *>();
}

QDialog *APRSExplorerPlugin::configurationDialog(QWidget *parent)
{
    auto *dialog = new QDialog();
    dialog->setWindowTitle(tr("APRS Plugin Settings"));
    return dialog;
}

QList<SlippyMapLayer*> APRSExplorerPlugin::layers()
{
    return QList<SlippyMapLayer *>();
}

QList<SlippyMapWidgetLayer*> APRSExplorerPlugin::tileLayers() {
    return QList<SlippyMapWidgetLayer*>();
}

void APRSExplorerPlugin::loadConfiguration()
{

}


