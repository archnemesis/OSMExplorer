#ifndef APRSFIEXPLORERPLUGIN_H
#define APRSFIEXPLORERPLUGIN_H

#include <QObject>
#include <QtPlugin>
#include <QList>
#include <QMap>
#include <QString>
#include <QStringList>
#include <QTimer>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QDockWidget>

#include "explorerplugininterface.h"
#include "slippymaplayer.h"
#include "slippymaplayerobject.h"

class AprsFiLocationDataProvider;

class AprsFiExplorerPlugin : public QObject, public ExplorerPluginInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.robingingras.osmexplorer.ExplorerPluginInterface")
    Q_INTERFACES(ExplorerPluginInterface)
public:
    AprsFiExplorerPlugin(QObject *parent = nullptr);

    QString name() const;
    QString description() const;
    QString authorName() const;
    QString homepage() const;
    QList<QAction*> mapContextMenuActionList();
    QList<QMenu*> mainMenuList();
    QList<QDockWidget*> dockWidgetList();
    QDialog *configurationDialog(QWidget *parent = nullptr);
    QList<SlippyMapLayer*> layers();
    void loadConfiguration();

private:
    SlippyMapLayer *m_layer = nullptr;
    int m_updateInterval;
    QString m_apiUrl;
    QString m_apiKey;
    QStringList m_callsigns;
    QNetworkAccessManager *m_net;
    QNetworkReply *m_reply;
    QTimer m_requestTimer;
    QMap<QString,SlippyMapLayerObject*> m_objects;

protected slots:
    void onNetworkRequestFinished(QNetworkReply *reply);
    void onRequestTimerUpdate();

signals:
    void objectCreated(SlippyMapLayerObject *object);
    void objectRemoved(SlippyMapLayerObject *object);
};

#endif // APRSFIEXPLORERPLUGIN_H
