#include "rainviewerplugin.h"

#include <QMenu>
#include <QAction>
#include <QDialog>
#include <QList>
#include <QDockWidget>
#include <QTimer>

#include <SlippyMap/SlippyMapLayer.h>
#include <SlippyMap/SlippyMapWidgetLayer.h>

#include "RainViewerTileLayer.h"

using namespace SlippyMap;

RainViewerExplorerPlugin::RainViewerExplorerPlugin()
    : m_radarLayer(nullptr)
{

}

QString RainViewerExplorerPlugin::name() const
{
    return tr("RainViewer");
}

QString RainViewerExplorerPlugin::description() const
{
    return tr("View live radar imagery");
}

QString RainViewerExplorerPlugin::authorName() const
{
    return tr("Robin Gingras");
}

QString RainViewerExplorerPlugin::homepage() const
{
    return tr("https://www.robingingras.com");
}

QList<QAction *> RainViewerExplorerPlugin::mapContextMenuActionList()
{
    return {};
}

QList<QMenu *> RainViewerExplorerPlugin::mainMenuList()
{
    return {};
}

QList<QDockWidget *> RainViewerExplorerPlugin::dockWidgetList()
{
    return {};
}

QDialog *RainViewerExplorerPlugin::configurationDialog(QWidget *parent)
{
    auto *dialog = new QDialog();
    dialog->setWindowTitle(tr("RainViewer Plugin Settings"));
    return dialog;
}

QList<SlippyMapLayer *> RainViewerExplorerPlugin::layers()
{
    return {};
}

QList<SlippyMapWidgetLayer*> RainViewerExplorerPlugin::tileLayers() {
    if (m_radarLayer == nullptr) {
        m_radarLayer = new RainViewerSlippyMapTileLayer();
        m_radarLayer->update();

        m_radarFrameTimer = new QTimer();
        connect(m_radarFrameTimer,
            &QTimer::timeout,
            [this]() {
                m_radarLayer->nextFrame();
        });
        m_radarFrameTimer->start(1000);
    }

    QList<SlippyMapWidgetLayer*> list;
    list << m_radarLayer;
    return list;
}

void RainViewerExplorerPlugin::loadConfiguration()
{

}

QList<SlippyMapLayerObjectPropertyPage *> RainViewerExplorerPlugin::propertyPages()
{
    return {};
}


