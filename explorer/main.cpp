#include "mainwindow.h"
#include <QApplication>
#include <QStyleFactory>
#include <QSslSocket>
#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication::setOrganizationName("Robin Gingras");
    QApplication::setOrganizationDomain("robingingras.com");
    QApplication::setApplicationName("OSMExplorer");

    qDebug() << "SSL Version:" << QSslSocket::sslLibraryBuildVersionString();

    QApplication a(argc, argv);
    a.setStyle(QStyleFactory::create("Fusion"));
    MainWindow w;
    w.show();

    return a.exec();
}