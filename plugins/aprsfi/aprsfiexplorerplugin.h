#ifndef APRSFIEXPLORERPLUGIN_H
#define APRSFIEXPLORERPLUGIN_H

#include <QObject>
#include <QtPlugin>
#include <QList>
#include <QString>
#include "mapmarkermodel.h"
#include "slippymapwidgetmarker.h"
#include "explorerplugininterface.h"

class AprsFiLocationDataProvider;

class AprsFiExplorerPlugin : public QObject, public ExplorerPluginInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.robingingras.osmexplorer.ExplorerPluginInterface")
    Q_INTERFACES(ExplorerPluginInterface)
public:
    AprsFiExplorerPlugin();

    QString name() const;
    QString description() const;
    QList<QAction*> mapContextMenuActionList();
    QList<MapMarkerModel::MarkerGroup*> markerGroupList();
    QDialog *configurationDialog(QWidget *parent = nullptr);
    QList<LocationDataProvider*> locationDataProviderList();
private:
    AprsFiLocationDataProvider *m_dataProvider = nullptr;
    MapMarkerModel::MarkerGroup *m_markerGroup = nullptr;
};

#endif // APRSFIEXPLORERPLUGIN_H
