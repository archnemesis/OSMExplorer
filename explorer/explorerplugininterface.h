#ifndef EXPLORERPLUGININTERFACE_H
#define EXPLORERPLUGININTERFACE_H

#include <QObject>
#include <QList>
#include <QAction>
#include <QString>

#include "slippymapwidgetmarker.h"
#include "slippymapwidgetmarkermodel.h"
#include "slippymapwidgetmarkergroup.h"
#include "locationdataprovider.h"

class ExplorerPluginInterface : public QObject
{
    Q_OBJECT
public:
    ExplorerPluginInterface(QObject *parent = nullptr);
    ~ExplorerPluginInterface();

    virtual QString name() const = 0;
    virtual QString description() const = 0;
    virtual QString authorName() const = 0;
    virtual QString homepage() const = 0;
    virtual QList<QAction*> mapContextMenuActionList() = 0;
    virtual QList<SlippyMapWidgetMarkerGroup*> markerGroupList() = 0;
    virtual QDialog *configurationDialog(QWidget *parent = nullptr) = 0;
};

#define ExplorerPluginInterface_iid "com.robingingras.osmexplorer.ExplorerPluginInterface"

Q_DECLARE_INTERFACE(ExplorerPluginInterface, ExplorerPluginInterface_iid)

#endif // EXPLORERPLUGININTERFACE_H
