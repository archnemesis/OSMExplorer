//
// Created by robin on 10/30/2024.
//

#ifndef PLUGINMANAGER_H
#define PLUGINMANAGER_H

#include <QObject>
#include <SlippyMap/SlippyMapLayer.h>

class ExplorerPluginInterface;
class SlippyMapLayerObjectPropertyPage;

namespace SlippyMap {
    class SlippyMapWidgetLayer;
}

class PluginManager : public QObject {
    Q_OBJECT
public:
    explicit PluginManager(QObject *parent = nullptr);
    void loadPlugins();
    const QList<ExplorerPluginInterface*> getPlugins() const;
    QList<SlippyMap::SlippyMapWidgetLayer*> getTileLayers() const;
    QList<SlippyMap::SlippyMapLayer::Ptr> getLayers() const;
    QList<SlippyMapLayerObjectPropertyPage*> getPropertyPages() const;

protected:
    QList<ExplorerPluginInterface*> m_plugins;
};



#endif //PLUGINMANAGER_H
