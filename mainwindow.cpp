#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "slippymapwidget.h"

#include <QComboBox>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    connect(ui->slippyMap, &SlippyMapWidget::centerChanged, this, &MainWindow::onSlippyMapCenterChanged);
    connect(ui->slippyMap, &SlippyMapWidget::zoomLevelChanged, this, &MainWindow::onSlippyMapZoomLevelChanged);
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

void MainWindow::onSlippyMapZoomLevelChanged(int zoom)
{
    ui->cboZoomLevel->setCurrentIndex(zoom);
}

void MainWindow::on_btnGo_clicked()
{
    bool oklat = false;
    bool oklon = false;
    double lat = ui->lneLatitude->text().toDouble(&oklat);
    double lon = ui->lneLongitude->text().toDouble(&oklon);

    if (!(oklat && oklon)) {
        QMessageBox::critical(
                    this,
                    tr("Invalid Input"),
                    tr("Please input latitude and longitude in degree decimal format."));
        return;
    }

    ui->slippyMap->setCenter(lat, lon);
}
