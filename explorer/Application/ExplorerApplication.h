//
// Created by robin on 10/30/2024.
//

#ifndef EXPLORERAPPLICATION_H
#define EXPLORERAPPLICATION_H

#include <QApplication>

class PluginManager;

namespace SlippyMap {
    class SlippyMapLayerManager;
}

class ExplorerApplication : public QApplication {
    Q_OBJECT
public:
    ExplorerApplication(int &argc, char **argv);

    static ExplorerApplication *instance();
    static PluginManager *pluginManager();
    static SlippyMap::SlippyMapLayerManager *layerManager();

private:
    PluginManager *m_pluginManager;
    SlippyMap::SlippyMapLayerManager *m_layerManager;
};



#endif //EXPLORERAPPLICATION_H
