#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "slippymapwidget.h"

#include <QComboBox>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    connect(ui->slippyMap, &SlippyMapWidget::centerChanged, this, &MainWindow::onSlippyMapCenterChanged);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onSlippyMapCenterChanged(double latitude, double longitude)
{
    ui->lneLatitude->setText(QString("%1").arg(latitude, 7, 'f', 4, QLatin1Char('0')));
    ui->lneLongitude->setText(QString("%1").arg(longitude, 7, 'f', 4, QLatin1Char('0')));
}
