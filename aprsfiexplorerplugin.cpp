#include "aprsfiexplorerplugin.h"

AprsFiExplorerPlugin::AprsFiExplorerPlugin()
{

}

QString AprsFiExplorerPlugin::name() const
{
    return "aprs.fi";
}

QString AprsFiExplorerPlugin::description() const
{
    return "aprs.fi integration (http://aprs.fi)";
}

QList<QAction *> AprsFiExplorerPlugin::mapContextMenuActionList() const
{
    return QList<QAction *>();
}

QList<MapMarkerModel::MarkerGroup *> AprsFiExplorerPlugin::markerGroupList() const
{
    MapMarkerModel::MarkerGroup *group = new MapMarkerModel::MarkerGroup("aprs.fi");
    QList<MapMarkerModel::MarkerGroup *> list;
    list.append(group);
    return list;
}
