//
// Created by robin on 10/30/2024.
//

#ifndef EXPLORERAPPLICATION_H
#define EXPLORERAPPLICATION_H

#include <QApplication>

class PluginManager;
class HistoryManager;
class DatabaseManager;

namespace SlippyMap {
    class SlippyMapLayerManager;
}

class ExplorerApplication : public QApplication {
    Q_OBJECT
public:
    ExplorerApplication(int &argc, char **argv);

    static ExplorerApplication *instance();
    static PluginManager *pluginManager();
    static HistoryManager *historyManager();
    static DatabaseManager *databaseManager();
    static SlippyMap::SlippyMapLayerManager *layerManager();

private:
    PluginManager *m_pluginManager;
    HistoryManager *m_historyManager;
    DatabaseManager *m_databaseManager;
    SlippyMap::SlippyMapLayerManager *m_layerManager;
};



#endif //EXPLORERAPPLICATION_H
