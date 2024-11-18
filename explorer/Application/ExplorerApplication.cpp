//
// Created by robin on 10/30/2024.
//

#include "ExplorerApplication.h"
#include "PluginManager.h"
#include "HistoryManager.h"
#include "DatabaseManager.h"
#include <SlippyMap/SlippyMapLayerManager.h>

ExplorerApplication::ExplorerApplication(int &argc, char **argv) :
    QApplication(argc, argv){
    m_pluginManager = new PluginManager(this);
    m_pluginManager->loadPlugins();
    m_layerManager = new SlippyMap::SlippyMapLayerManager(this);
    m_historyManager = new HistoryManager(this);
    m_databaseManager = new DatabaseManager(this);
    m_serverInterface = new ServerInterface(this);
}

ExplorerApplication * ExplorerApplication::instance() {
    return dynamic_cast<ExplorerApplication*>(QApplication::instance());
}

PluginManager * ExplorerApplication::pluginManager() {
    auto *app = dynamic_cast<ExplorerApplication*>(QApplication::instance());
    return app->m_pluginManager;
}

SlippyMap::SlippyMapLayerManager *ExplorerApplication::layerManager()
{
    auto *app = dynamic_cast<ExplorerApplication*>(QApplication::instance());
    return app->m_layerManager;
}

ServerInterface* ExplorerApplication::serverInterface()
{
    auto *app = dynamic_cast<ExplorerApplication*>(QApplication::instance());
    return app->m_serverInterface;
}

HistoryManager *ExplorerApplication::historyManager()
{
    auto *app = dynamic_cast<ExplorerApplication*>(QApplication::instance());
    return app->m_historyManager;
}

DatabaseManager *ExplorerApplication::databaseManager()
{
    auto *app = dynamic_cast<ExplorerApplication*>(QApplication::instance());
    return app->m_databaseManager;
}
