#include "aprsexplorerplugin.h"

#include <QMenu>
#include <QAction>
#include <QList>

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
    (void)parent;
    return nullptr;
}

QList<SlippyMapLayer *> APRSExplorerPlugin::layers()
{
    return QList<SlippyMapLayer *>();
}

void APRSExplorerPlugin::loadConfiguration()
{

}


