//
// Created by robin on 10/30/2024.
//

#include "ExplorerApplication.h"
#include "PluginManager.h"

ExplorerApplication::ExplorerApplication(int &argc, char **argv) :
    QApplication(argc, argv){
    m_pluginManager = new PluginManager();
    m_pluginManager->loadPlugins();
}

ExplorerApplication * ExplorerApplication::instance() {
    return static_cast<ExplorerApplication*>(QApplication::instance());
}

PluginManager * ExplorerApplication::pluginManager() {
    auto *app = static_cast<ExplorerApplication*>(QApplication::instance());
    return app->m_pluginManager;
}
