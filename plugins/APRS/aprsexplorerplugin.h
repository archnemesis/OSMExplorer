#ifndef APRSEXPLORERPLUGIN_H
#define APRSEXPLORERPLUGIN_H

#include <QObject>
#include "explorerplugininterface.h"

class QAction;
class QMenu;
class QDockWidget;
class QDialog;

namespace SlippyMap
{
    class SlippyMapLayer;
    class SlippyMapWidgetLayer;
}

using namespace SlippyMap;

class APRSExplorerPlugin : public QObject, public ExplorerPluginInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.robingingras.osmexplorer.ExplorerPluginInterface")
    Q_INTERFACES(ExplorerPluginInterface)
public:
    APRSExplorerPlugin();

    QString name() const;
    QString description() const;
    QString authorName() const;
    QString homepage() const;
    QList<QAction*> mapContextMenuActionList();
    QList<QMenu*> mainMenuList();
    QList<QDockWidget*> dockWidgetList();
    QDialog *configurationDialog(QWidget *parent = nullptr);
    QList<SlippyMapLayer*> layers();
    QList<SlippyMapWidgetLayer*> tileLayers();
    void loadConfiguration();
};

#endif // APRSEXPLORERPLUGIN_H
