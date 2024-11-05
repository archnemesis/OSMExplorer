#ifndef RAINVIEWERPLUGIN_H
#define RAINVIEWERPLUGIN_H

#include <QObject>
#include "explorerplugininterface.h"

namespace SlippyMap
{
    class SlippyMapWidgetLayer;
    class SlippyMapLayer;
}
using namespace SlippyMap;

class QDialog;
class QTimer;
class RainViewerSlippyMapTileLayer;
class SlippyMapLayerObjectPropertyPage;

class RainViewerExplorerPlugin : public QObject, public ExplorerPluginInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.robingingras.osmexplorer.RainViewerExplorerPlugin")
    Q_INTERFACES(ExplorerPluginInterface)
public:
    RainViewerExplorerPlugin();

    QString name() const;
    QString description() const;
    QString authorName() const;
    QString homepage() const;
    QList<QAction*> mapContextMenuActionList();
    QList<QMenu*> mainMenuList();
    QList<QDockWidget*> dockWidgetList();
    QDialog *configurationDialog(QWidget *parent = nullptr);
    QList<SlippyMapLayer::Ptr> layers();
    QList<SlippyMapWidgetLayer*> tileLayers();
    QList<SlippyMapLayerObjectPropertyPage*> propertyPages();
    void loadConfiguration();

private:
    RainViewerSlippyMapTileLayer *m_radarLayer;
    QTimer *m_radarFrameTimer;
};

#endif // RAINVIEWERPLUGIN_H
