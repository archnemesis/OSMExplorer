//
// Created by robin on 10/30/2024.
//

#ifndef EXPLORERAPPLICATION_H
#define EXPLORERAPPLICATION_H

#include <QApplication>

class PluginManager;


class ExplorerApplication : public QApplication {
    Q_OBJECT
public:
    ExplorerApplication(int &argc, char **argv);

    static ExplorerApplication *instance();
    static PluginManager *pluginManager();

private:
    PluginManager *m_pluginManager;
};



#endif //EXPLORERAPPLICATION_H
