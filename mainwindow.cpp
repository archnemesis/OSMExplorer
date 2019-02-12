#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "slippymapwidget.h"

#include <QComboBox>
#include <QMessageBox>
#include <QLabel>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    connect(ui->slippyMap, &SlippyMapWidget::centerChanged, this, &MainWindow::onSlippyMapCenterChanged);
    connect(ui->slippyMap, &SlippyMapWidget::zoomLevelChanged, this, &MainWindow::onSlippyMapZoomLevelChanged);
    connect(ui->slippyMap, &SlippyMapWidget::tileRequestInitiated, this, &MainWindow::onSlippyMapTileRequestStarted);
    connect(ui->slippyMap, &SlippyMapWidget::tileRequestFinished, this, &MainWindow::onSlippyMapTileRequestFinished);
    connect(ui->slippyMap, &SlippyMapWidget::cursorPositionChanged, this, &MainWindow::onSlippyMapCursorPositionChanged);
    connect(ui->slippyMap, &SlippyMapWidget::cursorEntered, this, &MainWindow::onSlippyMapCursorEntered);
    connect(ui->slippyMap, &SlippyMapWidget::cursorLeft, this, &MainWindow::onSlippyMapCursorLeft);

    m_statusBarPositionLabel = new QLabel(this);
    m_statusBarStatusLabel = new QLabel(this);

    statusBar()->addPermanentWidget(m_statusBarPositionLabel);
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

}

void MainWindow::onSlippyMapTileRequestStarted()
{
    m_requestCount++;

    if (m_requestCount > 0) {
        ui->statusBar->showMessage(QString("Loading %1 tiles...").arg(m_requestCount));
    }
    else {
        ui->statusBar->clearMessage();
    }
}

void MainWindow::onSlippyMapTileRequestFinished()
{
    m_requestCount--;

    if (m_requestCount > 0) {
        ui->statusBar->showMessage(QString("Loading %1 tiles...").arg(m_requestCount));
    }
    else {
        ui->statusBar->clearMessage();
    }
}

void MainWindow::onSlippyMapCursorPositionChanged(double latitude, double longitude)
{
    QString tpl("%1 %2");
    QString lon;
    QString lat;

    if (latitude > 0) {
        lat = tpl.arg(abs(latitude), 8, 'f', 4, '0').arg("N");
    }
    else {
        lat = tpl.arg(abs(latitude), 8, 'f', 4, '0').arg("S");
    }

    if (longitude < 0) {
        lon = tpl.arg(abs(longitude), 8, 'f', 4, '0').arg("W");
    }
    else {
        lon = tpl.arg(abs(longitude), 8, 'f', 4, '0').arg("E");
    }

    m_statusBarPositionLabel->setText(
                QString("%1, %2")
                    .arg(lat).arg(lon));
}

void MainWindow::onSlippyMapCursorEntered()
{

}

void MainWindow::onSlippyMapCursorLeft()
{
    m_statusBarPositionLabel->setText("");
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
