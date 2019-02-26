#ifndef EXPLORERPLUGININTERFACE_H
#define EXPLORERPLUGININTERFACE_H

#include <QList>
#include <QAction>
#include <QString>
#include <QDockWidget>

#include "slippymapwidgetmarker.h"
#include "slippymapwidgetmarkermodel.h"
#include "slippymapwidgetmarkergroup.h"
#include "locationdataprovider.h"

class ExplorerPluginInterface
{
public:
    virtual ~ExplorerPluginInterface() {}

    /**
     * @brief Return the plugin display name
     * @return
     */
    virtual QString name() const = 0;

    /**
     * @brief Return the plugin description for help
     * @return
     */
    virtual QString description() const = 0;

    /**
     * @brief authorName
     * @return
     */
    virtual QString authorName() const = 0;

    /**
     * @brief homepage
     * @return
     */
    virtual QString homepage() const = 0;

    /**
     * @brief Actions to be added to the SlippyMapWidget
     * context menu.
     * @return
     */
    virtual QList<QAction*> mapContextMenuActionList() = 0;

    /**
     * @brief Dock widgets added to the main wnidow.
     * @return
     */
    virtual QList<QDockWidget*> dockWidgetList() = 0;
    virtual QList<SlippyMapWidgetMarker*> markerList() = 0;
    virtual QDialog *configurationDialog(QWidget *parent = nullptr) = 0;
    virtual void loadConfiguration() = 0;
};

#define ExplorerPluginInterface_iid "com.robingingras.osmexplorer.ExplorerPluginInterface"

Q_DECLARE_INTERFACE(ExplorerPluginInterface, ExplorerPluginInterface_iid)

#endif // EXPLORERPLUGININTERFACE_H
