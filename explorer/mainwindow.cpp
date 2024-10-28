#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "slippymapwidget.h"
#include "slippymapwidgetlayer.h"
#include "slippymaplayerpolygon.h"
#include "slippymaplayermarker.h"
#include "slippymaplayertrack.h"
#include "slippymaplayerobjectpropertypage.h"
#include "slippymaplayerobjectprovider.h"
#include "markerdialog.h"
#include "markerlistitemwidget.h"
#include "directionlistitemwidget.h"
#include "settingsdialog.h"
#include "defaults.h"
#include "nmeaseriallocationdataprovider.h"
#include "gpssourcedialog.h"
#include "textlogviewerform.h"
#include "explorerplugininterface.h"
#include "polygonshapepropertiesform.h"
#include "gpxparser.h"
#include "gpxtrack.h"
#include "gpxtracksegment.h"
#include "gpxmetadata.h"
#include "gpxwaypoint.h"
#include "mapdataimportdialog.h"

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
#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QAction>
#include <QSettings>
#include <QTimer>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QSslConfiguration>
#include <QUrl>
#include <QStyleFactory>
#include <QStandardPaths>
#include <QPluginLoader>
#include <QMenu>
#include <QAction>
#include <QVector>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    m_layerManager = new SlippyMapLayerManager();
    ui->slippyMap->setLayerManager(m_layerManager);

    loadPlugins();

    ui->tvwMarkers->setModel(m_layerManager);
    ui->tvwMarkers->setContextMenuPolicy(Qt::CustomContextMenu);

    ui->tabShapeEditor->setVisible(false);
    m_defaultPalette = qApp->palette();

    m_net = new QNetworkAccessManager();
    connect(m_net, &QNetworkAccessManager::finished, this, &MainWindow::onNetworkRequestFinished);

    connect(ui->slippyMap, &SlippyMapWidget::centerChanged, this, &MainWindow::onSlippyMapCenterChanged);
    connect(ui->slippyMap, &SlippyMapWidget::zoomLevelChanged, this, &MainWindow::onSlippyMapZoomLevelChanged);
    connect(ui->slippyMap, &SlippyMapWidget::tileRequestInitiated, this, &MainWindow::onSlippyMapTileRequestStarted);
    connect(ui->slippyMap, &SlippyMapWidget::tileRequestFinished, this, &MainWindow::onSlippyMapTileRequestFinished);
    connect(ui->slippyMap, &SlippyMapWidget::cursorPositionChanged, this, &MainWindow::onSlippyMapCursorPositionChanged);
    connect(ui->slippyMap, &SlippyMapWidget::cursorEntered, this, &MainWindow::onSlippyMapCursorEntered);
    connect(ui->slippyMap, &SlippyMapWidget::cursorLeft, this, &MainWindow::onSlippyMapCursorLeft);
    connect(ui->slippyMap, &SlippyMapWidget::searchTextChanged, this, &MainWindow::onSlippyMapSearchTextChanged);
    connect(ui->slippyMap, &SlippyMapWidget::contextMenuRequested, this, &MainWindow::onSlippyMapContextMenuRequested);
    connect(ui->slippyMap, &SlippyMapWidget::drawModeChanged, this, &MainWindow::onSlippyMapDrawModeChanged);
    connect(ui->slippyMap, &SlippyMapWidget::rectSelected, this, &MainWindow::onSlippyMapRectSelected);

    m_defaultMarkerLayer = new SlippyMapLayer();
    m_defaultMarkerLayer->setName(tr("Markers"));
    m_layerManager->addLayer(m_defaultMarkerLayer);
    m_layerManager->setDefaultLayer(m_defaultMarkerLayer);

    loadStartupSettings();
    setupContextMenus();
    refreshSettings();
    setupToolbar();

    connect(ui->tvwMarkers, &QTreeView::customContextMenuRequested, this, &MainWindow::onTvwMarkersContextMenuRequested);

    m_statusBarPositionLabel = new QLabel();
    m_statusBarPositionLabel->setFrameStyle(QFrame::Sunken);
    m_statusBarStatusLabel = new QLabel();

    m_statusBarGpsStatusLabel = new QLabel();
    m_statusBarGpsStatusLabel->setFrameStyle(QFrame::Sunken);
    m_statusBarGpsStatusLabel->setText("GPS Position: 0.000 N 0.000 E");
    statusBar()->addPermanentWidget(m_statusBarGpsStatusLabel);

    statusBar()->addPermanentWidget(m_statusBarPositionLabel);
    ui->slippyMap->setFocus(Qt::OtherFocusReason);

    QPalette systemPalette = QGuiApplication::palette();
    m_directionLineColor = systemPalette.highlight().color();

    m_saveSplitterPosTimer = new QTimer();
    m_saveSplitterPosTimer->setSingleShot(true);
    m_saveSplitterPosTimer->setInterval(100);
    connect(m_saveSplitterPosTimer, &QTimer::timeout, this, &MainWindow::onSplitterPosTimerTimeout);

    m_saveWindowSizeTimer = new QTimer();
    m_saveWindowSizeTimer->setSingleShot(true);
    m_saveWindowSizeTimer->setInterval(100);
    connect(m_saveWindowSizeTimer, &QTimer::timeout, this, &MainWindow::onWindowSizeTimerTimeout);

    m_markerMenu = new QMenu();
    m_markerMenu->setTitle(tr("Marker"));

    m_markerPropertiesAction = new QAction();
    m_markerPropertiesAction->setText(tr("Properties..."));
    m_markerMenu->addAction(m_markerPropertiesAction);
    connect(m_markerPropertiesAction, &QAction::triggered, this, &MainWindow::onMarkerMenuPropertiesActionTriggered);

    qDebug() << "Loading plugin marker groups...";

    for (ExplorerPluginInterface *plugin : m_plugins) {
        QList<QDockWidget *> dockWidgets = plugin->dockWidgetList();
        for (QDockWidget *dockWidget : dockWidgets) {
            dockWidget->setParent(this);
            addDockWidget(Qt::LeftDockWidgetArea, dockWidget);
        }

    }

    loadMarkers();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    if (m_saveWindowSizeTimer != nullptr) {
        m_saveWindowSizeTimer->stop();
        m_saveWindowSizeTimer->start();
    }
}

void MainWindow::loadPlugins()
{
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
    pluginsDir.cd("plugins");

    foreach (QString fileName, pluginsDir.entryList(QDir::Files)) {
        QPluginLoader pluginLoader(pluginsDir.absoluteFilePath(fileName));
        QObject *plugin = pluginLoader.instance();
        if (plugin) {
            ExplorerPluginInterface *interface =
                    qobject_cast<ExplorerPluginInterface*>(plugin);
            interface->loadConfiguration();
            m_plugins.append(interface);

            QList<SlippyMapLayer*> pluginLayers = interface->layers();
            for (SlippyMapLayer *layer : pluginLayers) {
                m_layerManager->addLayer(layer);
            }
        }
    }
}

void MainWindow::loadMarkers()
{
//    QSettings settings;

//    for (SlippyMapWidgetMarker *marker : m_loadedMarkers) {
//        ui->slippyMap->deleteMarker(marker);
//        delete marker;
//    }
//    m_loadedMarkers.clear();

//    int count = settings.beginReadArray("places/my-places");
//    for (int i = 0; i < count; i++) {
//        settings.setArrayIndex(i);
//        double lat = settings.value("latitude").toDouble();
//        double lon = settings.value("longitude").toDouble();
//        QPointF pos(lon, lat);
//        QString label = settings.value("name").toString();
//        SlippyMapWidgetMarker *marker =
//                new SlippyMapWidgetMarker(pos, label);
//        marker->setInformation(settings.value("information").toString());
//        ui->slippyMap->addMarker(marker);
//        m_loadedMarkers.append(marker);
//        connect(marker, &SlippyMapWidgetMarker::changed, this, &MainWindow::saveMarkers);
//    }
//    settings.endArray();
}

void MainWindow::setupContextMenus()
{
    m_coordAction = new QAction();
    m_coordAction->setEnabled(false);

    m_addMarkerAction = new QAction();
    m_addMarkerAction->setText(tr("Add Marker"));
    connect(m_addMarkerAction, &QAction::triggered, this, &MainWindow::onAddMarkerActionTriggered);

    m_deleteMarkerAction = new QAction();
    m_deleteMarkerAction->setText(tr("Delete Marker"));

    m_setMarkerLabelAction = new QAction();
    m_setMarkerLabelAction->setText(tr("Properties..."));
    connect(m_setMarkerLabelAction, &QAction::triggered, this, &MainWindow::onEditMarkerActionTriggered);

    m_centerMapAction = new QAction();
    m_centerMapAction->setText(tr("Center Here"));
    connect(m_centerMapAction, &QAction::triggered, this, &MainWindow::onCenterMapActionTriggered);

    m_zoomInHereMapAction = new QAction();
    m_zoomInHereMapAction->setText(tr("Zoom In"));
    m_zoomOutHereMapAction = new QAction();
    m_zoomOutHereMapAction->setText(tr("Zoom Out"));
    m_copyCoordinatesAction = new QAction();
    m_copyCoordinatesAction->setText(tr("Copy Coordinates"));
    m_copyLatitudeAction = new QAction();
    m_copyLatitudeAction->setText(tr("Copy Latitude"));
    m_copyLongitudeAction = new QAction();
    m_copyLongitudeAction->setText(tr("Copy Longitude"));

    m_directionsFromHereAction = new QAction();
    m_directionsFromHereAction->setText("Directions From Here");
    m_directionsToHereAction = new QAction();
    m_directionsToHereAction->setText("Directions To Here");

    m_editShapeAction = new QAction();
    m_editShapeAction->setText(tr("Properties..."));
    m_editShapeAction->setVisible(false);

    m_deleteShapeAction = new QAction();
    m_deleteShapeAction->setText(tr("Delete"));
    m_deleteShapeAction->setVisible(false);

    connect(m_directionsFromHereAction, &QAction::triggered, this, &MainWindow::onDirectionsFromHereTriggered);
    connect(m_directionsToHereAction, &QAction::triggered, this, &MainWindow::onDirectionsToHereTriggered);

    m_contextMenu = new QMenu();
    m_contextMenu->addAction(m_coordAction);
    m_contextMenu->addSeparator();
    m_contextMenu->addAction(m_addMarkerAction);
    m_contextMenu->addAction(m_deleteMarkerAction);
    m_contextMenu->addAction(m_setMarkerLabelAction);
    m_contextMenu->addAction(m_editShapeAction);
    m_contextMenu->addAction(m_deleteShapeAction);
    m_contextMenu->addSeparator();
    m_contextMenu->addAction(m_centerMapAction);
    m_contextMenu->addAction(m_zoomInHereMapAction);
    m_contextMenu->addAction(m_zoomOutHereMapAction);
    m_contextMenu->addSeparator();
    m_contextMenu->addAction(m_copyCoordinatesAction);
    m_contextMenu->addAction(m_copyLatitudeAction);
    m_contextMenu->addAction(m_copyLongitudeAction);
    m_contextMenu->addSeparator();
    m_contextMenu->addAction(m_directionsFromHereAction);
    m_contextMenu->addAction(m_directionsToHereAction);
}

void MainWindow::loadStartupSettings()
{
    QSettings settings;

    if (settings.contains("view/windowWidth") && settings.contains("view/windowHeight")) {
        int width = settings.value("view/windowWidth").toInt();
        int height = settings.value("view/windowHeight").toInt();
        resize(width, height);
    }

    int layerCount = settings.beginReadArray("layers");
    for (int i = 0; i < layerCount; i++) {
        settings.setArrayIndex(i);
        QString name = settings.value("name").toString();
        QString description = settings.value("description").toString();
        QString tileUrl = settings.value("tileServer").toString();
        int zOrder = settings.value("zOrder").toInt();
        bool visible = settings.value("visible", true).toBool();
        SlippyMapWidgetLayer *layer = new SlippyMapWidgetLayer(tileUrl);
        layer->setName(name);
        layer->setDescription(description);
        layer->setZOrder(zOrder);
        layer->setVisible(visible);
        ui->slippyMap->addLayer(layer);
        m_layers.append(layer);

        QAction *layerShowHide = new QAction();
        layerShowHide->setCheckable(true);
        layerShowHide->setChecked(visible);
        layerShowHide->setText(name);
        connect(layerShowHide, &QAction::triggered, [=]() {
            layer->setVisible(layerShowHide->isChecked());
            ui->slippyMap->update();
            saveLayers();
        });
        ui->menuFileLayers->addAction(layerShowHide);
    }
    settings.endArray();

    if (layerCount == 0) {
        QMessageBox::information(
                    this,
                    tr("Layers"),
                    tr("There are no layers configured. To get started, add one or more layers in Settings."),
                    QMessageBox::Ok);
        on_actionFileSettings_triggered();
    }

    double defLat = settings.value("map/defaults/latitude", DEFAULT_LATITUDE).toDouble();
    double defLon = settings.value("map/defaults/longitude", DEFAULT_LONGITUDE).toDouble();
    int defZoom = settings.value("map/defaults/zoomLevel", DEFAULT_ZOOM).toInt();
    ui->slippyMap->setCenter(defLat, defLon);
    ui->slippyMap->setZoomLevel(defZoom);
}

void MainWindow::saveLayers()
{
    QSettings settings;

    settings.beginWriteArray("layers");
    for (int i = 0; i < m_layers.length(); i++) {
        settings.setArrayIndex(i);
        settings.setValue("name", m_layers.at(i)->name());
        settings.setValue("description", m_layers.at(i)->description());
        settings.setValue("tileServer", m_layers.at(i)->tileUrl());
        settings.setValue("zOrder", m_layers.at(i)->zOrder());
        settings.setValue("visible", m_layers.at(i)->isVisible());
    }
    settings.endArray();
}

void MainWindow::setupToolbar()
{
    ui->toolBar->addAction(ui->actionDrawRectangle);
    ui->toolBar->addAction(ui->actionDrawEllipse);
    ui->toolBar->addAction(ui->actionDrawPolygon);
}

void MainWindow::onSlippyMapCenterChanged(double latitude, double longitude)
{
    (void)latitude;
    (void)longitude;
}

void MainWindow::onSlippyMapZoomLevelChanged(int zoom)
{
    (void)zoom;
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

//void MainWindow::onSlippyMapMarkerAdded(SlippyMapWidgetMarker *marker)
//{
////    m_markerModelGroup_myMarkers->addMarker(marker);
////    connect(marker, &SlippyMapWidgetMarker::changed, this, &MainWindow::saveMarkers);
////    ui->tvwMarkers->update();
//}

//void MainWindow::onSlippyMapMarkerDeleted(SlippyMapWidgetMarker *marker)
//{
////    m_markerModelGroup_myMarkers->removeMarker(marker);
////    ui->tvwMarkers->update();
//}

//void MainWindow::onSlippyMapMarkerUpdated(SlippyMapWidgetMarker *marker)
//{

//}

//void MainWindow::onSlippyMapMarkerEditRequested(SlippyMapWidgetMarker *marker)
//{
//    if (MarkerDialog::getEditMarker(this, "Marker Properties", marker)) {
//        // ok...
//    }
//}

void MainWindow::onSlippyMapContextMenuActivated(double latitude, double longitude)
{
    m_slippyContextMenuLocation.setX(longitude);
    m_slippyContextMenuLocation.setY(latitude);
}

void MainWindow::onSlippyMapSearchTextChanged(const QString &text)
{
    qDebug() << "Search Text:" << text;
}

void MainWindow::onSlippyMapContextMenuRequested(const QPoint &point)
{
    m_contextMenuLocation = point;
    m_coordAction->setText(SlippyMapWidget::latLonToString(
                               ui->slippyMap->widgetY2lat(point.y()),
                               ui->slippyMap->widgetX2long(point.x())));

    m_addMarkerAction->setVisible(true);
    m_deleteMarkerAction->setVisible(false);
    m_setMarkerLabelAction->setVisible(false);
    m_editShapeAction->setVisible(false);
    m_deleteShapeAction->setVisible(false);

    QRectF viewport = ui->slippyMap->boundingBoxLatLon();
    for (SlippyMapLayer *layer : m_layerManager->layers()) {
        for (SlippyMapLayerObject *object : layer->objects()) {
            if (viewport.contains(object->position())) {
                int markerX = ui->slippyMap->long2widgetX(object->position().x());
                int markerY = ui->slippyMap->lat2widgety(object->position().y());

                QRect clickbox(
                    markerX - 5,
                    markerY - 5,
                    10, 10);

                if (clickbox.contains(m_contextMenuLocation)) {
                    m_addMarkerAction->setVisible(false);
                    m_setMarkerLabelAction->setVisible(true);

                    break;
                }
            }
        }
    }

    QPointF geoPoint;
    geoPoint.setX(ui->slippyMap->widgetX2long(point.x()));
    geoPoint.setY(ui->slippyMap->widgetY2lat(point.y()));

    m_contextMenu->exec(ui->slippyMap->mapToGlobal(point));
}

void MainWindow::onSlippyMapRectSelected(QRect rect)
{
    QBrush br;
    br.setStyle(Qt::SolidPattern);
    br.setColor(QColor(0,0,0,128));

    QPen pn;
    pn.setStyle(Qt::NoPen);
    pn.setColor(Qt::darkCyan);

    QPointF topleft = ui->slippyMap->widgetCoordsToGeoCoords(rect.topLeft());
    QPointF bottomRight = ui->slippyMap->widgetCoordsToGeoCoords(rect.bottomRight());

    QVector<QPointF> points(4);
    points[0] = QPointF(topleft);
    points[1] = QPointF(bottomRight.x(), topleft.y());
    points[2] = QPointF(bottomRight);
    points[3] = QPointF(topleft.x(), bottomRight.y());
    SlippyMapLayerPolygon *poly = new SlippyMapLayerPolygon(points);
    poly->setName(tr("New Rect"));
    poly->setDescription(tr("New rectangle"));
//    poly->setBrush(br);
//    poly->setPen(pn);
    m_layerManager->addLayerObject(poly);
}

void MainWindow::onSlippyMapDrawModeChanged(SlippyMapWidget::DrawMode mode)
{
    switch (mode) {
    case SlippyMapWidget::NoDrawing:
        ui->actionDrawLine->setChecked(false);
        ui->actionDrawRectangle->setChecked(false);
        ui->actionDrawPolygon->setChecked(false);
        ui->actionDrawEllipse->setChecked(false);
        break;
    default:
        break;
    }
}

void MainWindow::onSlippyMapLayerObjectActivated(SlippyMapLayerObject *object)
{
    if (m_selectedObject != nullptr && m_selectedObject == object) {
        return;
    }

    SlippyMapLayerObjectPropertyPage *ppage = object->propertyPage(this);
    ui->tabShapeEditor->insertTab(1, ppage, ppage->tabTitle());
    ui->tabShapeEditor->setVisible(true);
    ui->lblNoShapeSelected->setVisible(false);
    m_selectedObject = object;
}

void MainWindow::onSlippyMapLayerObjectDeactivated(SlippyMapLayerObject *object)
{
    (void)object;

    if (m_selectedObject != nullptr) {
        ui->tabShapeEditor->removeTab(1);
        ui->tabShapeEditor->setVisible(false);
        ui->lblNoShapeSelected->setVisible(true);
        m_selectedObject = nullptr;
    }
}

void MainWindow::saveMarkers()
{
    QSettings settings;


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

void MainWindow::onNetworkRequestFinished(QNetworkReply *reply)
{
    m_loadingDialog->hide();

    if (reply->error() != QNetworkReply::NoError) {
        QMessageBox::critical(
                    this,
                    tr("Network Error"),
                    tr("Failed to get directions from server."));
        reply->deleteLater();
        return;
    }

    QByteArray data = reply->readAll();

    qDebug() << QString::fromUtf8(data);

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
                //ui->slippyMap->addLineSet(lineSet);
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

void MainWindow::onGpsDataProviderPositionUpdated(QString identifier, QPointF position, QHash<QString, QVariant> metadata)
{
    SlippyMapLayerMarker *marker;

    if (m_gpsMarkers.contains(identifier)) {
        marker = m_gpsMarkers[identifier];
        marker->setLabel(metadata["gps_label"].toString());
        marker->setPosition(position);
    }
    else {
        marker = new SlippyMapLayerMarker(position);
        marker->setLabel(metadata["gps_label"].toString());
        marker->setColor(Qt::green);
        marker->setEditable(false);
        marker->setMovable(false);
        m_gpsMarkers[identifier] = marker;
        m_gpsMarkerLayer->addObject(marker);
    }
}

void MainWindow::onTvwMarkersContextMenuRequested(const QPoint &point)
{
    QModelIndex index = ui->tvwMarkers->indexAt(point);
    if (index.isValid()) {
        m_markerMenu->exec(ui->tvwMarkers->viewport()->mapToGlobal(point));
    }
}

void MainWindow::onMarkerMenuPropertiesActionTriggered()
{
    QModelIndex index = ui->tvwMarkers->currentIndex();
    SlippyMapLayerObject *obj = static_cast<SlippyMapLayerObject*>(index.internalPointer());
    if (m_layerManager->contains(obj)) {
        SlippyMapLayerObjectPropertyPage *page = obj->propertyPage();
        if (page != nullptr) {
            page->show();
        }
    }
}

void MainWindow::onPluginLayerObjectProviderMarkerAdded(SlippyMapLayerObject *object)
{
    //ui->slippyMap->addMarker(marker);

}

void MainWindow::onAddMarkerActionTriggered()
{
    double lon = ui->slippyMap->widgetX2long(m_contextMenuLocation.x());
    double lat = ui->slippyMap->widgetY2lat(m_contextMenuLocation.y());
    QPointF markerPoint(lon, lat);
    SlippyMapLayerMarker *marker = new SlippyMapLayerMarker(markerPoint);
    marker->setLabel(SlippyMapWidget::latLonToString(lat, lon));
    marker->setDescription(tr("Test Label"));
    m_layerManager->defaultLayer()->addObject(marker);
}

void MainWindow::onDeleteMarkerActionTriggered()
{

}

void MainWindow::onEditMarkerActionTriggered()
{
//    QRectF viewport = ui->slippyMap->boundingBoxLatLon();
//    for (SlippyMapWidgetMarker *marker : ui->slippyMap->markerList()) {
//        if (viewport.contains(marker->position())) {
//            int markerX = ui->slippyMap->long2widgetX(marker->longitude());
//            int markerY = ui->slippyMap->lat2widgety(marker->latitude());
//            QRect clickbox(
//                markerX - 5,
//                markerY - 5,
//                10, 10);
//            if (clickbox.contains(m_contextMenuLocation)) {
//                MarkerDialog::getEditMarker(
//                    this,
//                    tr("Marker Properties"),
//                    marker);
//            }
//        }
//    }
}

void MainWindow::onCenterMapActionTriggered()
{
    QPointF coords = ui->slippyMap->widgetCoordsToGeoCoords(m_contextMenuLocation);
    ui->slippyMap->setCenter(coords);
}

void MainWindow::setDarkModeEnabled(bool enabled)
{
    // increase font size for better reading
    //QFont defaultFont = QApplication::font();
    //defaultFont.setPointSize(defaultFont.pointSize()+2);
    //qApp->setFont(defaultFont);
    // modify palette to dark

    if (enabled) {
        QPalette darkPalette;
        darkPalette.setColor(QPalette::Window,QColor(53,53,53));
        darkPalette.setColor(QPalette::WindowText,Qt::white);
        darkPalette.setColor(QPalette::Disabled,QPalette::WindowText,QColor(127,127,127));
        darkPalette.setColor(QPalette::Base,QColor(42,42,42));
        darkPalette.setColor(QPalette::AlternateBase,QColor(66,66,66));
        darkPalette.setColor(QPalette::ToolTipBase,Qt::white);
        darkPalette.setColor(QPalette::ToolTipText,Qt::white);
        darkPalette.setColor(QPalette::Text,Qt::white);
        darkPalette.setColor(QPalette::Disabled,QPalette::Text,QColor(127,127,127));
        darkPalette.setColor(QPalette::Dark,QColor(35,35,35));
        darkPalette.setColor(QPalette::Shadow,QColor(20,20,20));
        darkPalette.setColor(QPalette::Button,QColor(53,53,53));
        darkPalette.setColor(QPalette::ButtonText,Qt::white);
        darkPalette.setColor(QPalette::Disabled,QPalette::ButtonText,QColor(127,127,127));
        darkPalette.setColor(QPalette::BrightText,Qt::red);
        darkPalette.setColor(QPalette::Link,QColor(42,130,218));
        darkPalette.setColor(QPalette::Highlight,QColor(42,130,218));
        darkPalette.setColor(QPalette::Disabled,QPalette::Highlight,QColor(80,80,80));
        darkPalette.setColor(QPalette::HighlightedText,Qt::white);
        darkPalette.setColor(QPalette::Disabled,QPalette::HighlightedText,QColor(127,127,127));

        qApp->setPalette(darkPalette);
    }
    else {
        qApp->setPalette(m_defaultPalette);
    }
}

void MainWindow::onEditShapeActionTriggered()
{
    QRectF viewport = ui->slippyMap->boundingBoxLatLon();
    QPointF geoPoint;

    geoPoint.setX(ui->slippyMap->widgetX2long(m_contextMenuLocation.x()));
    geoPoint.setY(ui->slippyMap->widgetY2lat(m_contextMenuLocation.y()));

    for (SlippyMapLayer *layer : m_layerManager->layers()) {
        for (SlippyMapLayerObject *object : layer->objects()) {
            if (object->isIntersectedBy(viewport)) {

            }
        }
    }
}

void MainWindow::onSplitterPosTimerTimeout()
{
//    QSettings settings;
//    QList<int> widths = ui->splitter->sizes();
//    double ratio = static_cast<double>(widths[0]) / static_cast<double>(width());
//    settings.setValue("view/sidebarWidth", ratio);
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
    ui->slippyMap->setTileCacheDir(settings.value("map/cache/tiledir", QStandardPaths::writableLocation(QStandardPaths::CacheLocation)).toString());
    ui->slippyMap->setTileCachingEnabled(settings.value("map/cache/enable", true).toBool());
}

void MainWindow::on_actionNewMarker_triggered()
{
//    SlippyMapWidgetMarker *marker = MarkerDialog::getNewMarker(this, tr("New Marker"));
//    if (marker != nullptr) {
//        ui->slippyMap->addMarker(marker);
//    }
}

void MainWindow::on_actionViewSidebar_toggled(bool arg1)
{
    //ui->toolBox->setVisible(arg1);
    QSettings settings;
    settings.setValue("view/sidebarVisible", arg1);
}

void MainWindow::on_actionViewClearRoute_triggered()
{
    if (m_currentRouteListItem != nullptr) {
//        ui->slippyMap->removeLineSet(m_currentRouteLineSet);
//        ui->lstDirections->removeItemWidget(m_currentRouteListItem);
//        ui->lstDirections->clear();
//        m_currentRouteLineSet = nullptr;
//        m_currentRouteListItem = nullptr;
//        m_currentRouteListItemWidget = nullptr;
    }
}

void MainWindow::on_actionFileSettings_triggered()
{
    if (m_settingsDialog == nullptr) {
        m_settingsDialog = new SettingsDialog(m_plugins);
        connect(m_settingsDialog, &SettingsDialog::accepted, this, &MainWindow::refreshSettings);
    }

    m_settingsDialog->setModal(true);
    m_settingsDialog->show();
}

void MainWindow::on_btnDirectionsGo_clicked()
{
    QSettings settings;

    on_actionViewClearRoute_triggered();

    if (settings.contains("wayfinding/service")) {
        QString service = settings.value("wayfinding/service").toString();
        if (service == "openrouteservice.org") {
            QString startLocation = ui->lneDirectionsStart->text();
            QStringList startLocationParts = startLocation.split(",");
            double startLongitude = startLocationParts[0].toDouble();
            double startLatitude = startLocationParts[1].toDouble();
            QPointF startPoint(startLongitude, startLatitude);

            QString finishLocation = ui->lneDirectionsFinish->text();
            QStringList finishLocationParts = finishLocation.split(",");
            double finishLongitude = finishLocationParts[0].toDouble();
            double finishLatitude = finishLocationParts[1].toDouble();
            QPointF finishPoint(finishLongitude, finishLatitude);



            QString urlBase = settings.value("wayfinding/openrouteservice/url").toString();
            QString apiKey = settings.value("wayfinding/openrouteservice/apikey").toString();
            QString req = QString("%1?api_key=%2&coordinates=%3|%4&profile=driving-car&geometry_format=geojson")
                    .arg(urlBase)
                    .arg(apiKey)
                    .arg(ui->lneDirectionsStart->text())
                    .arg(ui->lneDirectionsFinish->text());
            qDebug() << "Requesting from" << req;

            //QSslConfiguration config = QSslConfiguration::defaultConfiguration();
            //config.setProtocol(QSsl::TlsV1_2);
            QNetworkRequest request(req);
            //request.setSslConfiguration(config);
            m_net->get(request);

            if (m_loadingDialog == nullptr) {
                m_loadingDialog = new QMessageBox();
            }
            m_loadingDialog->setWindowTitle(tr("Loading"));
            m_loadingDialog->setText(tr("Loading directions..."));
            m_loadingDialog->setInformativeText(tr("Click Cancel to abort."));
            m_loadingDialog->setStandardButtons(QMessageBox::Cancel);
            m_loadingDialog->setDefaultButton(QMessageBox::Cancel);
            m_loadingDialog->setModal(true);
            m_loadingDialog->show();
        }
    }
}

void MainWindow::on_actionMapGpsAddSource_triggered()
{
    GpsSourceDialog::GpsSourceInfo source =
            GpsSourceDialog::getNewSource(
                this,
                tr("Add GPS Source"));

    if (source.isValid) {
        if (source.sourceType == GpsSourceDialog::NmeaSource) {
            NmeaSerialLocationDataProvider *provider =
                    new NmeaSerialLocationDataProvider();
            provider->setLabelText(source.label);
            provider->setPortName(source.portName);
            provider->setBaudRate(source.baudRate);
            provider->setDataBits(source.dataBits);
            provider->setStopBits(source.stopBits);
            provider->setParity(source.parity);
            provider->setFlowControl(source.flowControl);
            connect(provider, &LocationDataProvider::positionUpdated, this, &MainWindow::onGpsDataProviderPositionUpdated);
            provider->start();
            m_gpsProviders.append(provider);

            QAction *configAction = new QAction(this);
            configAction->setText(QString("%1 (%2)").arg(source.label).arg(source.portName));
            configAction->setCheckable(true);
            configAction->setChecked(true);
            ui->menuFileGps->addAction(configAction);
            connect(configAction, &QAction::toggled, [=](bool value){
                if (value) {
                    provider->start();
                }
                else {
                    provider->stop();
                }
            });
        }
    }
}

void MainWindow::on_actionViewGpsLog_triggered()
{
    if (m_nmeaLog == nullptr) {
        m_nmeaLog = new TextLogViewerForm();
        m_nmeaLog->setWindowTitle(tr("NMEA Log"));
        for (LocationDataProvider *provider : m_gpsProviders) {
            NmeaSerialLocationDataProvider *gpsProvider
                    = qobject_cast<NmeaSerialLocationDataProvider*>(provider);
            connect(
                        gpsProvider,
                        &NmeaSerialLocationDataProvider::lineReceived,
                        m_nmeaLog,
                        &TextLogViewerForm::addLine);
        }
    }

    m_nmeaLog->show();
}

void MainWindow::on_tvwMarkers_activated(const QModelIndex &index)
{
//    SlippyMapWidgetMarker *marker =
//            static_cast<SlippyMapWidgetMarker *>(index.internalPointer());
//    if (m_markerModel->contains(marker)) {
//        ui->slippyMap->setCenter(marker->position());
//    }
    //SlippyMapLayerObject *obj = static_cast<SlippyMapLayerObject*>(index.internalPointer());

}

void MainWindow::on_tvwMarkers_clicked(const QModelIndex &index)
{
//    SlippyMapWidgetMarker *marker =
//            static_cast<SlippyMapWidgetMarker *>(index.internalPointer());
//    if (m_markerModel->contains(marker)) {
//        ui->lblMarkerName->setText(marker->label());
//        ui->lblMarkerInformation->setText(marker->information());
//    }
}

void MainWindow::on_actionDrawRectangle_triggered()
{
    ui->slippyMap->setDrawMode(SlippyMapWidget::RectDrawing);
}

void MainWindow::on_actionDrawEllipse_triggered()
{
    ui->slippyMap->setDrawMode(SlippyMapWidget::EllipseDrawing);
}

void MainWindow::on_actionImport_triggered()
{
    QString fileName = QFileDialog::getOpenFileName(
                this,
                tr("Choose Import File"),
                QDir::homePath(),
                tr("GPX Files (*.gpx)"));

    QFile file(fileName);

    qDebug() << "Processing file" << fileName;

    if (file.open(QFile::ReadOnly)) {
        GPXParser parser;
        parser.read(&file);

        qDebug() << "Found" << parser.tracks().count() << "tracks";
        qDebug() << "Found" << parser.waypoints().count() << "waypoints";

        for (GPXTrack *track : parser.tracks()) {
            for (GPXTrackSegment *segment : track->segments()) {
                QList<QPointF> points;
                for (GPXWaypoint *waypoint : segment->points()) {
                    points.append(QPointF(waypoint->longitude(), waypoint->latitude()));
                    qDebug() << "Added point" << points.last();
                }
                SlippyMapLayerTrack *track = new SlippyMapLayerTrack(QVector<QPointF>::fromList(points));
                QString trackName = parser.metadata()->name();
                if (trackName.isEmpty()) {
                    QFileInfo trackFileInfo(fileName);
                    trackName = trackFileInfo.baseName();
                }
                track->setName(trackName);
                track->setDescription(parser.metadata()->description());
                m_layerManager->addLayerObject(m_defaultMarkerLayer, track);
            }
        }
    }
}

void MainWindow::on_actionMarkerImport_triggered()
{
}

void MainWindow::on_actionToolsOSMImport_triggered()
{
    if (m_importDialog == nullptr) {
        m_importDialog = new MapDataImportDialog(this);
    }
    m_importDialog->show();
    m_importDialog->raise();
}
