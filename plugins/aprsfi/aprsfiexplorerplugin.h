#ifndef APRSFIEXPLORERPLUGIN_H
#define APRSFIEXPLORERPLUGIN_H

#include <QObject>
#include <QtPlugin>
#include <QList>
#include <QString>
#include "slippymapwidgetmarkermodel.h"
#include "slippymapwidgetmarker.h"
#include "explorerplugininterface.h"

class AprsFiLocationDataProvider;

class AprsFiExplorerPlugin : public QObject, public ExplorerPluginInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.robingingras.osmexplorer.ExplorerPluginInterface")
    Q_INTERFACES(ExplorerPluginInterface)
public:
    QString name() const;
    QString description() const;
    QString authorName() const;
    QString homepage() const;
    QList<QAction*> mapContextMenuActionList();
    QList<SlippyMapWidgetMarkerGroup *> markerGroupList();
    QDialog *configurationDialog(QWidget *parent = nullptr);
    void loadConfiguration();
protected slots:
    void dataProviderPositionUpdated(QString identifier, QPointF position, QHash<QString, QVariant> metadata);

private:
    AprsFiLocationDataProvider *m_dataProvider = nullptr;
    SlippyMapWidgetMarkerGroup *m_markerGroup = nullptr;
};

#endif // APRSFIEXPLORERPLUGIN_H
