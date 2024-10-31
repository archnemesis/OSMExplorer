//
// Created by robin on 10/30/2024.
//

#include "PluginManager.h"

#include <QApplication>
#include <QDir>
#include <QPluginLoader>

#include <SlippyMap/SlippyMapWidgetLayer.h>
#include <SlippyMap/SlippyMapLayer.h>

#include "explorerplugininterface.h"

using namespace SlippyMap;

PluginManager::PluginManager(QObject *parent) :
    QObject(parent) {

}

void PluginManager::loadPlugins() {
    QDir pluginsDir(qApp->applicationDirPath());

#if defined(Q_OS_WIN)
    if (pluginsDir.dirName().toLower() == "debug" ||  pluginsDir.dirName().toLower() == "release") {
        pluginsDir.cdUp();
        pluginsDir.cdUp();
    }
#elif defined(Q_OS_MAC)
    if (pluginsDir.dirName() == "MacOS") {
        pluginsDir.cdUp();
        pluginsDir.cdUp();
        pluginsDir.cdUp();
    }
#endif

#ifdef QT_DEBUG
    QStringList pluginPaths;
    pluginPaths << "plugins/APRS";
    pluginPaths << "plugins/RainViewer";

    for (const auto& pluginPath : pluginPaths) {
#else
    QString pluginPath = "/usr/lib";
#endif
    pluginsDir.cd(pluginPath);

    foreach (QString fileName, pluginsDir.entryList(QDir::Files)) {
        qDebug() << "Loading plugin file:" << fileName;
        QPluginLoader pluginLoader(pluginsDir.absoluteFilePath(fileName));
        QObject *plugin = pluginLoader.instance();
        if (plugin) {
            ExplorerPluginInterface *interface =
                    qobject_cast<ExplorerPluginInterface*>(plugin);
            interface->loadConfiguration();
            m_plugins.append(interface);
        }
    }
#ifdef QT_DEBUG
    pluginsDir.cdUp();
    pluginsDir.cdUp();
    }
#endif
}

const QList<ExplorerPluginInterface *> PluginManager::getPlugins() const {
    return m_plugins;
}

QList<SlippyMap::SlippyMapWidgetLayer *> PluginManager::getTileLayers() const {
    QList<SlippyMapWidgetLayer*> layers;
    for (auto *plugin : m_plugins) {
        for (auto *pluginLayer : plugin->tileLayers()) {
            layers.append(pluginLayer);
        }
    }

    return layers;
}

QList<SlippyMap::SlippyMapLayer *> PluginManager::getLayers() const {
    QList<SlippyMap::SlippyMapLayer *> layers;
    for (auto *plugin : m_plugins) {
        for (auto *pluginLayer : plugin->layers()) {
            layers.append(pluginLayer);
        }
    }

    return layers;
}

QList<SlippyMapLayerObjectPropertyPage *> PluginManager::getPropertyPages() const
{
    QList<SlippyMapLayerObjectPropertyPage*> propertyPages;
    for (auto *plugin : m_plugins) {
        for (auto *propertyPage : plugin->propertyPages()) {
            propertyPages.append(propertyPage);
        }
    }

    return propertyPages;
}
