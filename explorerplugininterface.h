#ifndef EXPLORERPLUGININTERFACE_H
#define EXPLORERPLUGININTERFACE_H

#include <QList>
#include <QAction>
#include <QString>

#include "slippymapwidgetmarker.h"
#include "mapmarkermodel.h"

class ExplorerPluginInterface
{
public:
    virtual ~ExplorerPluginInterface() {}

    virtual QString name() const = 0;
    virtual QString description() const = 0;
    virtual QList<QAction*> mapContextMenuActionList() const = 0;
    virtual QList<MapMarkerModel::MarkerGroup*> markerGroupList() const = 0;
};

#define ExplorerPluginInterface_iid "com.robingingras.osmexplorer.ExplorerPluginInterface"

Q_DECLARE_INTERFACE(ExplorerPluginInterface, ExplorerPluginInterface_iid)

#endif // EXPLORERPLUGININTERFACE_H
