#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "slippymapwidget.h"
#include "markerdialog.h"
#include "markerlistitemwidget.h"
#include "directionlistitemwidget.h"
#include "settingsdialog.h"
#include "defaults.h"

#include <math.h>
#include <QGuiApplication>
#include <QPalette>
#include <QDebug>
#include <QComboBox>
#include <QMessageBox>
#include <QLabel>
#include <QListWidgetItem>
#include <QFileDialog>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QAction>
#include <QSettings>
#include <QTimer>
#include <QNetworkAccessManager>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    connect(ui->splitter, &QSplitter::splitterMoved, this, &MainWindow::onSplitterMoved);

    m_net = new QNetworkAccessManager();

    QSettings settings;

    if (settings.contains("view/sidebarWidth")) {
        double ratio = settings.value("view/sidebarWidth").toDouble();
        int sidebar_width = (int)((double)width() * ratio);
        int map_width = width() - sidebar_width;
        QList<int> widths;
        widths.append(sidebar_width);
        widths.append(map_width);
        ui->splitter->setSizes(widths);
    }

    if (settings.contains("view/sidebarVisible")) {
        ui->toolBox->setVisible(settings.value("view/sidebarVisible").toBool());
        ui->actionViewSidebar->setChecked(true);
    }

    if (settings.contains("view/windowWidth") && settings.contains("view/windowHeight")) {
        int width = settings.value("view/windowWidth").toInt();
        int height = settings.value("view/windowHeight").toInt();
        resize(width, height);
    }

    double defLat = settings.value("map/defaults/latitude", DEFAULT_LATITUDE).toDouble();
    double defLon = settings.value("map/defaults/longitude", DEFAULT_LONGITUDE).toDouble();
    int defZoom = settings.value("map/defaults/zoomLevel", DEFAULT_ZOOM).toInt();
    ui->slippyMap->setCenter(defLat, defLon);
    ui->slippyMap->setZoomLevel(defZoom);

    refreshSettings();

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
    connect(ui->slippyMap, &SlippyMapWidget::contextMenuActivated, this, &MainWindow::onSlippyMapContextMenuActivated);

    m_statusBarPositionLabel = new QLabel();
    m_statusBarStatusLabel = new QLabel();

    statusBar()->addPermanentWidget(m_statusBarPositionLabel);
    ui->slippyMap->setFocus(Qt::OtherFocusReason);

    QPalette systemPalette = QGuiApplication::palette();
    m_directionLineColor = systemPalette.highlight().color();

    m_directionsFromHereAction = new QAction();
    m_directionsFromHereAction->setText("Directions From Here");
    m_directionsToHereAction = new QAction();
    m_directionsToHereAction->setText("Directions To Here");

    ui->slippyMap->addContextMenuAction(m_directionsFromHereAction);
    ui->slippyMap->addContextMenuAction(m_directionsToHereAction);
    connect(m_directionsFromHereAction, &QAction::triggered, this, &MainWindow::onDirectionsFromHereTriggered);
    connect(m_directionsToHereAction, &QAction::triggered, this, &MainWindow::onDirectionsToHereTriggered);

    m_saveSplitterPosTimer = new QTimer();
    m_saveSplitterPosTimer->setSingleShot(true);
    m_saveSplitterPosTimer->setInterval(100);
    connect(m_saveSplitterPosTimer, &QTimer::timeout, this, &MainWindow::onSplitterPosTimerTimeout);

    m_saveWindowSizeTimer = new QTimer();
    m_saveWindowSizeTimer->setSingleShot(true);
    m_saveWindowSizeTimer->setInterval(100);
    connect(m_saveWindowSizeTimer, &QTimer::timeout, this, &MainWindow::onWindowSizeTimerTimeout);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    m_saveWindowSizeTimer->stop();
    m_saveWindowSizeTimer->start();
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

void MainWindow::onSlippyMapContextMenuActivated(double latitude, double longitude)
{
    m_slippyContextMenuLocation.setX(longitude);
    m_slippyContextMenuLocation.setY(latitude);
}

void MainWindow::onDirectionsToHereTriggered()
{
    QString latlon = QString("%1,%2")
            .arg(m_slippyContextMenuLocation.x())
            .arg(m_slippyContextMenuLocation.y());
    ui->lneDirectionsFinish->setText(latlon);
    ui->lneDirectionsStart->setFocus();
}

void MainWindow::onDirectionsFromHereTriggered()
{
    QString latlon = QString("%1,%2")
            .arg(m_slippyContextMenuLocation.x())
            .arg(m_slippyContextMenuLocation.y());
    ui->lneDirectionsStart->setText(latlon);
    ui->lneDirectionsFinish->setFocus();
}

void MainWindow::onSplitterMoved(int pos, int index)
{
    m_saveSplitterPosTimer->stop();
    m_saveSplitterPosTimer->start();
}

void MainWindow::onSplitterPosTimerTimeout()
{
    QSettings settings;
    QList<int> widths = ui->splitter->sizes();
    double ratio = (double)widths[0] / (double)width();
    settings.setValue("view/sidebarWidth", ratio);
}

void MainWindow::onWindowSizeTimerTimeout()
{
    QSettings settings;
    settings.setValue("view/windowWidth", width());
    settings.setValue("view/windowHeight", height());
}

void MainWindow::refreshSettings()
{
    QSettings settings;
    bool enable = settings.value("map/zoom/centerOnCursor", DEFAULT_CENTER_ON_CURSOR_ZOOM).toBool();
    ui->slippyMap->setCenterOnCursorWhileZooming(enable);
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
    QSettings settings;
    settings.setValue("view/sidebarVisible", arg1);
}

void MainWindow::on_actionDebugOpenDirectionsFile_triggered()
{
    QString fileName = QFileDialog::getOpenFileName(
                this,
                tr("Open Directions JSON"),
                "",
                tr("JSON Files (*.json)"));

    if (fileName.length() > 0) {
        QFile fp(fileName);
        qDebug() << "Trying file" << fileName;
        if (fp.open(QIODevice::ReadOnly)) {
            QByteArray data = fp.readAll();
            QJsonDocument doc = QJsonDocument::fromJson(data);
            QJsonObject obj = doc.object();

            if (obj.contains("routes") && obj["routes"].isArray()) {
                QJsonArray routesArray = obj["routes"].toArray();
                QJsonObject routes = routesArray[0].toObject();
                if (routes.contains("geometry") && routes["geometry"].isObject()) {
                    QJsonObject geometry = routes["geometry"].toObject();
                    if (geometry.contains("coordinates") && geometry["coordinates"].isArray()) {
                        QJsonArray coordinates = geometry["coordinates"].toArray();
                        QVector<QPointF> *points = new QVector<QPointF>();
                        for (int i = 0; i < coordinates.count(); i++) {
                            QJsonArray tuple = coordinates[i].toArray();
                            points->append(QPointF(tuple[0].toDouble(), tuple[1].toDouble()));
                        }

                        SlippyMapWidget::LineSet *lineSet = new SlippyMapWidget::LineSet(points, 3, m_directionLineColor);
                        ui->slippyMap->addLineSet(lineSet);
                        m_currentRouteLineSet = lineSet;
                    }
                    else {
                        qDebug() << "Could not find coordinates object";
                    }
                }
                else {
                    qDebug() << "Could not find geometry object";
                }

                if (routes.contains("segments") && routes["segments"].isArray()) {
                    QJsonArray segments = routes["segments"].toArray();
                    QJsonObject segment = segments[0].toObject();
                    if (segment.contains("steps") && segment["steps"].isArray()) {
                        QJsonArray steps = segment["steps"].toArray();

                        for (int i = 0; i < steps.count(); i++) {
                            QJsonObject step = steps[i].toObject();
                            double distance = step["distance"].toDouble();
                            double duration = step["duration"].toDouble();
                            QString instruction = step["instruction"].toString();
                            DirectionListItemWidget *itemWidget = new DirectionListItemWidget();
                            itemWidget->setInstruction(instruction);
                            itemWidget->setDistance(distance);
                            itemWidget->setDuration(duration);
                            QListWidgetItem *item = new QListWidgetItem();
                            item->setSizeHint(itemWidget->sizeHint());
                            ui->lstDirections->addItem(item);
                            ui->lstDirections->setItemWidget(item, itemWidget);
                            m_currentRouteListItem = item;
                            m_currentRouteListItemWidget = itemWidget;
                        }
                    }
                    else {
                        qDebug() << "Could not find steps array";
                    }
                }
                else {
                    qDebug() << "Could not find segments object";
                }
            }
            else {
                qDebug() << "Could not find routes object!";
            }
        }
        else {
            qDebug() << "Unable to open file!";
        }
    }
}

void MainWindow::on_actionViewClearRoute_triggered()
{
    if (m_currentRouteListItem != nullptr) {
        ui->slippyMap->removeLineSet(m_currentRouteLineSet);
        ui->lstDirections->removeItemWidget(m_currentRouteListItem);
        ui->lstDirections->clear();
        m_currentRouteLineSet = nullptr;
        m_currentRouteListItem = nullptr;
        m_currentRouteListItemWidget = nullptr;
    }
}

void MainWindow::on_actionFileSettings_triggered()
{
    if (m_settingsDialog == nullptr) {
        m_settingsDialog = new SettingsDialog();
        connect(m_settingsDialog, &SettingsDialog::accepted, this, &MainWindow::refreshSettings);
    }

    m_settingsDialog->setModal(true);
    m_settingsDialog->show();
}

void MainWindow::on_btnDirectionsGo_clicked()
{
    QSettings settings;

    if (settings.contains("wayfinding/service")) {
        QString service = settings.value("wayfinding/service").toString();
        if (service == "openrouteservice") {
            QString urlBase = settings.value("wayfinding/openrouteservice/url").toString();
            QString apiKey = settings.value("wayfinding/openrouteservice/apikey").toString();
            QString req = QString("%1?api_key=%2&coordinates=%3|%4&profile=driving-car")
                    .arg(urlBase)
                    .arg(apiKey)
                    .arg(ui->lneDirectionsStart->text())
                    .arg(ui->lneDirectionsFinish->text());
        }
    }
}
