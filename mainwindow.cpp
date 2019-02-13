#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "slippymapwidget.h"
#include "markerdialog.h"
#include "markerlistitemwidget.h"

#include <math.h>
#include <QComboBox>
#include <QMessageBox>
#include <QLabel>
#include <QListWidgetItem>
#include <QFileDialog>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

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
    connect(ui->slippyMap, &SlippyMapWidget::markerAdded, this, &MainWindow::onSlippyMapMarkerAdded);
    connect(ui->slippyMap, &SlippyMapWidget::markerDeleted, this, &MainWindow::onSlippyMapMarkerDeleted);
    connect(ui->slippyMap, &SlippyMapWidget::markerUpdated, this, &MainWindow::onSlippyMapMarkerUpdated);

    m_statusBarPositionLabel = new QLabel(this);
    m_statusBarStatusLabel = new QLabel(this);

    statusBar()->addPermanentWidget(m_statusBarPositionLabel);
    ui->toolBox->hide();
    ui->slippyMap->setFocus(Qt::OtherFocusReason);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onSlippyMapCenterChanged(double latitude, double longitude)
{

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
        lat = tpl.arg(fabs(latitude), 8, 'f', 4, '0').arg("N");
    }
    else {
        lat = tpl.arg(fabs(latitude), 8, 'f', 4, '0').arg("S");
    }

    if (longitude < 0) {
        lon = tpl.arg(fabs(longitude), 8, 'f', 4, '0').arg("W");
    }
    else {
        lon = tpl.arg(fabs(longitude), 8, 'f', 4, '0').arg("E");
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

void MainWindow::onSlippyMapMarkerAdded(SlippyMapWidget::Marker *marker)
{
    MarkerListItemWidget *itemWidget = new MarkerListItemWidget();
    itemWidget->setName(marker->label());
    itemWidget->setLatitude(marker->latitude());
    itemWidget->setLongitude(marker->longitude());
    QListWidgetItem *item = new QListWidgetItem();
    item->setSizeHint(itemWidget->sizeHint());
    ui->lstMarkers->addItem(item);
    ui->lstMarkers->setItemWidget(item, itemWidget);
    m_markerListItemMap[marker] = item;
    connect(itemWidget, &MarkerListItemWidget::markerMapButtonPressed, ui->slippyMap, &SlippyMapWidget::setCenter);
}

void MainWindow::onSlippyMapMarkerDeleted(SlippyMapWidget::Marker *marker)
{
    if (m_markerListItemMap.contains(marker)) {
        delete m_markerListItemMap[marker];
        m_markerListItemMap.remove(marker);
    }
}

void MainWindow::onSlippyMapMarkerUpdated(SlippyMapWidget::Marker *marker)
{
    if (m_markerListItemMap.contains(marker)) {
        QListWidgetItem *item = m_markerListItemMap[marker];
        MarkerListItemWidget *itemWidget = qobject_cast<MarkerListItemWidget*>(ui->lstMarkers->itemWidget(item));
        itemWidget->setName(marker->label());
        itemWidget->setLatitude(marker->latitude());
        itemWidget->setLongitude(marker->longitude());
    }
}

void MainWindow::on_actionNewMarker_triggered()
{
    SlippyMapWidget::Marker *marker = MarkerDialog::getNewMarker(this, tr("New Marker"));
    if (marker != nullptr) {
        ui->slippyMap->addMarker(marker);
    }
}

void MainWindow::on_actionViewSidebar_toggled(bool arg1)
{
    ui->toolBox->setVisible(arg1);
}

void MainWindow::on_actionDebugOpenDirectionsFile_triggered()
{
    QString fileName = QFileDialog::getOpenFileName(
                this,
                tr("Open Directions JSON"),
                tr("Select a file in GeoJSON format."),
                tr("JSON Files (*.json)"));

    if (fileName.length() > 0) {
        QFile fp(fileName);
        if (fp.exists() && fp.isReadable() && fp.open(QIODevice::ReadWrite)) {
            QByteArray data = fp.readAll();
            QJsonDocument doc = QJsonDocument::fromJson(data);
            QJsonObject obj = doc.object();

            if (obj.contains("routes") && obj["routes"].isObject()) {
                QJsonObject routes = obj["routes"].toObject();
                if (routes.contains("geometry") && routes["geometry"].isObject()) {
                    QJsonObject geometry = obj["geometry"].toObject();
                    if (geometry.contains("coordinates") && geometry["coordinates"].isArray()) {
                        QJsonArray coordinates = geometry["coordinates"].toArray();
                    }
                }
            }

        }
    }
}
