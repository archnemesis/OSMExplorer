#ifndef APRSFIEXPLORERPLUGIN_H
#define APRSFIEXPLORERPLUGIN_H

#include <QObject>
#include <QtPlugin>
#include <QList>
#include <QString>
#include "mapmarkermodel.h"
#include "slippymapwidgetmarker.h"
#include "explorerplugininterface.h"

class AprsFiExplorerPlugin : public QObject, public ExplorerPluginInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.robingingras.osmexplorer.ExplorerPluginInterface" FILE "aprsfiplugin.json")
    Q_INTERFACES(ExplorerPluginInterface)
public:
    AprsFiExplorerPlugin();

    QString name() const;
    QString description() const;
    QList<QAction*> mapContextMenuActionList() const;
    QList<MapMarkerModel::MarkerGroup*> markerGroupList() const;
};

#endif // APRSFIEXPLORERPLUGIN_H
