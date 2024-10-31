#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QAction>
#include <QInputDialog>
#include <QFile>
#include <QFileDialog>
#include <QGuiApplication>
#include <QJsonArray>
#include <QJsonDocument>
#include <QLabel>
#include <QListWidgetItem>
#include <QMenu>
#include <QMessageBox>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QPalette>
#include <QPluginLoader>
#include <QSettings>
#include <QSpinBox>
#include <QStandardPaths>
#include <QTimer>
#include <QUrl>
#include <QVector>

#include <QtColorWidgets/ColorSelector>

#include <SlippyMap/SlippyMapWidget.h>
#include <SlippyMap/SlippyMapLayer.h>
#include <SlippyMap/SlippyMapWidgetLayer.h>
#include <SlippyMap/SlippyMapLayerPolygon.h>
#include <SlippyMap/SlippyMapWidgetMarker.h>
#include <SlippyMap/SlippyMapLayerManager.h>
#include <SlippyMap/SlippyMapLayerObjectPropertyPage.h>
#include <SlippyMap/SlippyMapLayerMarkerPropertyPage.h>
#include <SlippyMap/SlippyMapLayerPolygonPropertyPage.h>
#include "Map/SlippyMapLayerTrackPropertyPage.h"

#include "Map/SlippyMapLayerTrack.h"
#include "Weather/WeatherForecastWindow.h"
#include "Application/ExplorerApplication.h"

#include "defaults.h"
#include "directionlistitemwidget.h"
#include "explorerplugininterface.h"
#include "gpssourcedialog.h"
#include "Map/gpx/gpxmetadata.h"
#include "Map/gpx/gpxparser.h"
#include "Map/gpx/gpxtrack.h"
#include "Map/gpx/gpxtracksegment.h"
#include "mapdataimportdialog.h"
#include "nmeaseriallocationdataprovider.h"
#include "settingsdialog.h"
#include "textlogviewerform.h"

#include <math.h>
#include <QSpinBox>

#include "Application/PluginManager.h"
#include "SlippyMap/SlippyMapLayerObjectCommonPropertyPage.h"
#include "Weather/WeatherStationMarker.h"
#include "Weather/WeatherStationPropertyPage.h"

#ifdef QT_DEBUG
#include <QDebug>
#endif

using namespace SlippyMap;
using namespace color_widgets;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_forecastZonePolygon(nullptr),
    m_weatherStationMarker(nullptr)
{
    ui->setupUi(this);
    setWindowTitle(tr("OSMExplorer") + " - Untitled.osm");
    loadPluginLayers();

    ui->tabShapeEditor->setVisible(false);
    m_defaultPalette = qApp->palette();

    /*
     * Layer manager and tree view model
    */
    //m_layerManager = new SlippyMapLayerManager();
    m_layerManager = ExplorerApplication::layerManager();
    ui->slippyMap->setLayerManager(m_layerManager);
    ui->tvwMarkers->setModel(m_layerManager);
    ui->tvwMarkers->setContextMenuPolicy(Qt::CustomContextMenu);

    /*
     * Default layer setup
     */
    m_defaultMarkerLayer = new SlippyMapLayer();
    m_defaultMarkerLayer->setName(tr("Markers"));
    m_layerManager->addLayer(m_defaultMarkerLayer);
    m_layerManager->setDefaultLayer(m_defaultMarkerLayer);

    /*
     * GPS Marker Layer
     */
    m_gpsMarkerLayer = new SlippyMapLayer();
    m_gpsMarkerLayer->setName(tr("GPS"));
    m_gpsMarkerLayer->setEditable(false);
    m_layerManager->addLayer(m_gpsMarkerLayer);

    /*
     * Weather Service Integration
     */
    m_weatherNetworkAccessManager = new QNetworkAccessManager();
    connect(m_weatherNetworkAccessManager,
        &QNetworkAccessManager::finished,
        this,
        &MainWindow::weatherNetworkAccessManager_onRequestFinished);

    m_weatherService = new NationalWeatherServiceInterface(this);
    connect(m_weatherService,
        &NationalWeatherServiceInterface::stationListReady,
        this,
            &MainWindow::onWeatherService_stationListReady);

    m_weatherLayer = new SlippyMapLayer();
    m_weatherLayer->setName(tr("Weather"));
    m_weatherLayer->setVisible(true);
    m_weatherLayer->setEditable(false);
    m_layerManager->addLayer(m_weatherLayer);

    /*
     * SlippyMap Signals and Slots
     */
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
    connect(ui->slippyMap, &SlippyMapWidget::objectActivated, this, &MainWindow::onSlippyMapLayerObjectActivated);
    connect(ui->slippyMap, &SlippyMapWidget::objectDeactivated, this, &MainWindow::onSlippyMapLayerObjectDeactivated);
    connect(ui->slippyMap, &SlippyMapWidget::objectDoubleClicked, this, &MainWindow::onSlippyMapLayerObjectDoubleClicked);
    connect(ui->slippyMap, &SlippyMapWidget::dragFinished, this, &MainWindow::onSlippyMapDragFinished);

    connect(ui->zoomInButton,
            &QPushButton::clicked,
            ui->slippyMap,
            &SlippyMapWidget::increaseZoomLevel);
    connect(ui->zoomOutButton,
            &QPushButton::clicked,
            ui->slippyMap,
            &SlippyMapWidget::decreaseZoomLevel);

    /**
     * Color and stroke width selectors for drawing
     */
    m_strokeColorSelector = new ColorSelector();
    m_strokeColorSelector->setColor(Qt::black);
    m_strokeColorSelector->setToolTip(tr("Stroke color"));
    connect(m_strokeColorSelector,
        &ColorSelector::colorSelected,
        [this](const QColor& color) {
            ui->slippyMap->setDrawingStrokeColor(color.lighter());
    });

    m_fillColorSelector = new ColorSelector();
    m_fillColorSelector->setColor(Qt::white);
    m_fillColorSelector->setToolTip(tr("Fill color"));
    connect(m_fillColorSelector,
        &ColorSelector::colorSelected,
        [this](const QColor& color) {
            ui->slippyMap->setDrawingFillColor(color.lighter());
    });

    m_strokeWidth = new QSpinBox();
    m_strokeWidth->setMinimum(0);
    m_strokeWidth->setValue(2);
    m_strokeWidth->setToolTip(tr("Stroke width"));
    connect(m_strokeWidth,
        QOverload<int>::of(&QSpinBox::valueChanged),
        [this](int value) {
            ui->slippyMap->setDrawingStrokeWidth(value);
    });

    ui->slippyMap->setDrawingFillColor(Qt::white);
    ui->slippyMap->setDrawingStrokeColor(QColor(Qt::black).lighter());
    ui->slippyMap->setDrawingStrokeWidth(2);

    /*
     * Lat/Lon inputs in the toolbar
     */
    m_toolBarLatitudeInput = new QLineEdit();
    m_toolBarLatitudeInput->setPlaceholderText(tr("Latitude"));
    m_toolBarLatitudeInput->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    m_toolBarLongitudeInput = new QLineEdit();
    m_toolBarLongitudeInput->setPlaceholderText(tr("Longitude"));
    m_toolBarLongitudeInput->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    m_toolBarLatLonButton = new QPushButton();
    m_toolBarLatLonButton->setText(tr("Go"));

    /*
     * Text inputs for label and description that let you edit the marker label.
     */
    connect(ui->selectedObjectName,
        &QLineEdit::textEdited,
        [this](const QString& text) {
        if (m_selectedObject) {
            m_selectedObject->setLabel(text);
        }
    });

    connect(ui->selectedObjectDescription, &QPlainTextEdit::textChanged, [this]() {
        if (m_selectedObject) {
            m_selectedObject->setDescription(ui->selectedObjectDescription->toPlainText());
        }
    });

    /*
     * Save workspace action
     */
    connect(ui->actionFile_SaveWorkspace,
        &QAction::triggered,
        this,
        &MainWindow::onActionFileSaveWorkspaceTriggered);

    connect(ui->actionFile_OpenWorkspace,
            &QAction::triggered,
            this,
            &MainWindow::onActionFileOpenWorkspaceTriggered);
    connect(ui->actionFile_CloseWorkspace,
            &QAction::triggered,
            this,
            &MainWindow::onActionFileCloseWorkspaceTriggered);

    /*
     * Enable showing forecast data as the map moves
     */
    connect(ui->actionWeather_ShowWFOGrid,
        &QAction::toggled,
        [this](bool state){
            if (state) {
                m_weatherService->getWeatherStationList(QPointF(
                        ui->slippyMap->longitude(),
                        ui->slippyMap->latitude()));
            }
            else {
                // TODO: remove the weather markers
            }
        });

    loadStartupSettings();
    setupContextMenus();
    refreshSettings();
    setupToolbar();

    connect(ui->tvwMarkers,
            &QTreeView::customContextMenuRequested,
            this,
            &MainWindow::onTvwMarkersContextMenuRequested);

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
    m_saveWindowSizeTimer->setInterval(1000);
    connect(m_saveWindowSizeTimer,
            &QTimer::timeout,
            this,
            &MainWindow::onWindowSizeTimerTimeout);

    m_markerMenu = new QMenu(this);
    m_markerMenu->setTitle(tr("Marker"));

    m_newLayerAction = new QAction();
    m_newLayerAction->setText(tr("New Layer"));
    m_markerMenu->addAction(m_newLayerAction);
    connect(m_newLayerAction,
            &QAction::triggered,
            [this]() {

        bool ok;
        QString layerName = QInputDialog::getText(this,
                                                  tr("New Layer"),
                                                  tr("Name"),
                                                  QLineEdit::Normal,
                                                  "New Layer",
                                                  &ok);

        if (ok) {
            auto *newLayer = new SlippyMapLayer();
            newLayer->setName(layerName);
            m_layerManager->addLayer(newLayer);
        }
    });

    m_markerVisibilityAction = new QAction();
    m_markerVisibilityAction->setText(tr("Visible"));
    m_markerVisibilityAction->setCheckable(true);
    m_markerMenu->addAction(m_markerVisibilityAction);
    connect(ui->tvwMarkers, &QTreeView::clicked, [this](const QModelIndex& index) {
        if (index.parent().isValid()) return;
        Q_ASSERT(index.row() < m_layerManager->layers().count());
        SlippyMapLayer *layer = m_layerManager->layers().at(index.row());
        m_markerVisibilityAction->setChecked(layer->isVisible());
    });
    connect(ui->tvwMarkers, &QTreeView::activated, [this](const QModelIndex& index) {
        if (index.parent().isValid()) return;
        Q_ASSERT(index.row() < m_layerManager->layers().count());
        SlippyMapLayer *layer = m_layerManager->layers().at(index.row());
        m_markerVisibilityAction->setChecked(layer->isVisible());
    });
    connect(m_markerVisibilityAction, &QAction::toggled, [this](bool checked) {
        QModelIndex index = ui->tvwMarkers->currentIndex();
        Q_ASSERT(index.row() < m_layerManager->layers().count());
        SlippyMapLayer *layer = m_layerManager->layers().at(index.row());
        layer->setVisible(checked);
    });

    qDebug() << "Loading plugin marker groups...";

    for (ExplorerPluginInterface *plugin : ExplorerApplication::pluginManager()->getPlugins()) {
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

void MainWindow::loadPluginLayers()
{
    auto *pluginManager = ExplorerApplication::pluginManager();

    for (auto *tileLayer : pluginManager->getTileLayers()) {
        ui->slippyMap->addLayer(tileLayer);
        auto *action = new QAction();
        action->setText(tileLayer->name());
        action->setCheckable(true);
        action->setChecked(true);
        connect(action,
            &QAction::triggered,
            [this, tileLayer](bool checked) {
            tileLayer->setVisible(checked);
        });
        ui->menuLayer_TileLayers->addAction(action);
    }

    for (auto *layer : pluginManager->getLayers()) {
        m_layerManager->addLayer(layer);
    }
}

void MainWindow::loadMarkers()
{

}

void MainWindow::setupContextMenus()
{
    m_coordAction = new QAction();
    m_coordAction->setEnabled(false);

    m_addMarkerAction = new QAction();
    m_addMarkerAction->setText(tr("Add Marker"));
    connect(m_addMarkerAction,
            &QAction::triggered,
            this,
            &MainWindow::onAddMarkerActionTriggered);

    m_deleteMarkerAction = new QAction();
    m_deleteMarkerAction->setText(tr("Delete Marker"));

    m_markerPropertiesAction = new QAction();
    m_markerPropertiesAction->setText(tr("Properties..."));
    connect(m_markerPropertiesAction,
            &QAction::triggered,
            this,
            &MainWindow::onMarkerMenuPropertiesActionTriggered);

    m_centerMapAction = new QAction();
    m_centerMapAction->setText(tr("Center Here"));
    connect(m_centerMapAction,
            &QAction::triggered,
            this,
            &MainWindow::onCenterMapActionTriggered);

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

    m_editShapeAction = new QAction();
    m_editShapeAction->setText(tr("Properties..."));
    m_editShapeAction->setVisible(false);

    m_deleteShapeAction = new QAction();
    m_deleteShapeAction->setText(tr("Delete"));
    m_deleteShapeAction->setVisible(false);

    m_getForecastHereAction = new QAction();
    m_getForecastHereAction->setText(tr("Get Forecast Here"));
    connect(m_getForecastHereAction,
            &QAction::triggered,
            [this]() {
        if (m_weatherForecastWindow != nullptr) {
            m_weatherForecastWindow->close();
            m_weatherForecastWindow->deleteLater();
        }

        m_weatherForecastWindow = new WeatherForecastWindow(m_contextMenuPoint);
        m_weatherForecastWindow->show();
    });

    m_contextMenu = new QMenu();
    m_contextMenu->addAction(m_coordAction);
    m_contextMenu->addSeparator();
    m_contextMenu->addAction(m_addMarkerAction);
    m_contextMenu->addAction(m_deleteMarkerAction);
    m_contextMenu->addAction(m_markerPropertiesAction);
    m_contextMenu->addAction(m_editShapeAction);
    m_contextMenu->addAction(m_deleteShapeAction);
    m_contextMenu->addSeparator();
    m_contextMenu->addAction(m_getForecastHereAction);
    m_contextMenu->addSeparator();
    m_contextMenu->addAction(m_centerMapAction);
    m_contextMenu->addAction(m_zoomInHereMapAction);
    m_contextMenu->addAction(m_zoomOutHereMapAction);
    m_contextMenu->addSeparator();
    m_contextMenu->addAction(m_copyCoordinatesAction);
    m_contextMenu->addAction(m_copyLatitudeAction);
    m_contextMenu->addAction(m_copyLongitudeAction);
}

void MainWindow::loadStartupSettings()
{
    QSettings settings;

    if (settings.contains("view/windowWidth") && settings.contains("view/windowHeight")) {
        if (settings.value("view/maximized", false).toBool()) {
            qDebug() << "Saved maximized state";
            showMaximized();
        }
        else {
            int width = settings.value("view/windowWidth").toInt();
            int height = settings.value("view/windowHeight").toInt();
            resize(width, height);
        }
    }

    int recentFileCount = settings.beginReadArray("recentFiles");
    for (int i = 0; i < recentFileCount; i++) {
        settings.setArrayIndex(i);
        QString fileName = settings.value("filename").toString();
        m_recentFileList.append(fileName);
    }
    settings.endArray();

    int layerCount = settings.beginReadArray("layers");
    for (int i = 0; i < layerCount; i++) {
        settings.setArrayIndex(i);
        QString name = settings.value("name").toString();
        QString description = settings.value("description").toString();
        QString tileUrl = settings.value("tileServer").toString();

        qDebug() << "Loading layer" << name << ":" << tileUrl;

        int zOrder = settings.value("zOrder").toInt();
        bool visible = settings.value("visible", true).toBool();
        SlippyMapWidgetLayer *layer = new SlippyMapWidgetLayer(tileUrl);
        layer->setName(name);
        layer->setDescription(description);
        layer->setZOrder(zOrder);
        layer->setVisible(visible);
        layer->setBaseLayer(true);
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
        ui->menuLayer_TileLayers->addAction(layerShowHide);
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
    ui->toolBar->addSeparator();
    ui->toolBar->addWidget(m_toolBarLatitudeInput);
    ui->toolBar->addWidget(m_toolBarLongitudeInput);
    ui->toolBar->addWidget(m_toolBarLatLonButton);
    ui->toolBar->addSeparator();
    ui->toolBar->addWidget(m_strokeColorSelector);
    ui->toolBar->addWidget(m_fillColorSelector);
    ui->toolBar->addWidget(m_strokeWidth);
}

void MainWindow::onSlippyMapCenterChanged(double latitude, double longitude)
{
    QString cardinal_lat;
    QString cardinal_lon;

    if (latitude >= 0) cardinal_lat = "N";
    else cardinal_lat = "S";

    if (longitude >= 0) cardinal_lon = "E";
    else cardinal_lon = "W";

    m_toolBarLatitudeInput->setText(QString("%1 %2")
        .arg(latitude, 0, 'f', 6)
        .arg(cardinal_lat));
    m_toolBarLongitudeInput->setText(QString("%1 %2")
        .arg(longitude, 0, 'f', 6)
        .arg(cardinal_lon));
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
////    connect(marker, &SlippyMapWidgetMarker::changed, this, &MainWindow::onActionFileSaveWorkspaceTriggered);
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
    m_markerPropertiesAction->setVisible(false);
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
                    m_coordAction->setText(object->label());
                    m_addMarkerAction->setVisible(false);
                    m_markerPropertiesAction->setVisible(true);
                    connect(m_markerPropertiesAction,
                            &QAction::triggered,
                            [this, object]() {
                        showPropertyPage(object);
                    });
                    m_deleteMarkerAction->setVisible(true);

                    break;
                }
            }
        }
    }

    QPointF geoPoint;
    geoPoint.setX(ui->slippyMap->widgetX2long(point.x()));
    geoPoint.setY(ui->slippyMap->widgetY2lat(point.y()));

    m_contextMenuPoint = geoPoint;
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
    poly->setLabel(tr("New Rect"));
    poly->setDescription(tr("New rectangle"));
    poly->setFillColor(m_fillColorSelector->color());
    poly->setStrokeColor(m_strokeColorSelector->color());
    poly->setStrokeWidth(m_strokeWidth->value());
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
        ui->slippyMap->setCursor(Qt::ArrowCursor);
        break;
    default:
        ui->slippyMap->setCursor(Qt::CrossCursor);
        break;
    }
}

void MainWindow::onSlippyMapLayerObjectActivated(SlippyMapLayerObject *object)
{
    auto *commonPropertyPage = new SlippyMapLayerObjectCommonPropertyPage(
        object, m_layerManager);

    connect(object,
        &SlippyMapLayerObject::updated,
        commonPropertyPage,
        &SlippyMapLayerObjectCommonPropertyPage::updateUi);

    QList<SlippyMapLayerObjectPropertyPage*> propertyPages;
    propertyPages.append(commonPropertyPage);
    propertyPages.append(object->propertyPage());

    for (auto *propertyPage : ExplorerApplication::pluginManager()->getPropertyPages()) {
        propertyPages.append(propertyPage);
    }

    ui->tabShapeEditor->clear();
    for (auto *propertyPage : propertyPages) {
        propertyPage->setupUi();
        ui->tabShapeEditor->addTab(propertyPage, propertyPage->tabTitle());
    }
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

void MainWindow::showPropertyPage(SlippyMapLayerObject *object)
{
    QList<SlippyMapLayerObjectPropertyPage*> propertyPages;
    propertyPages.append(new SlippyMapLayerObjectCommonPropertyPage(object, m_layerManager));
    propertyPages.append(object->propertyPage());

    for (auto *propertyPage : ExplorerApplication::pluginManager()->getPropertyPages()) {
        propertyPages.append(propertyPage);
    }

    auto *tabWidget = new QTabWidget();

    for (auto *propertyPage : propertyPages) {
        propertyPage->setupUi();
        tabWidget->addTab(propertyPage, propertyPage->tabTitle());
    }

    auto *dialog = new QDialog(this);

    auto *saveButton = new QPushButton(tr("Save"));
    connect(saveButton,
            &QPushButton::clicked,
            [this, object, propertyPages, dialog]() {
                if (object->isEditable()) {
                    for (auto *propertyPage: propertyPages)
                        propertyPage->save();
                    setWorkspaceDirty(true);
                }
                dialog->accept();
            });

    if (!object->isEditable())
        saveButton->setEnabled(false);

    auto *cancelButton = new QPushButton(tr("Cancel"));
    connect(cancelButton,
            &QPushButton::clicked,
            dialog,
            &QDialog::reject);

    auto *buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    buttonLayout->addWidget(saveButton);
    buttonLayout->addWidget(cancelButton);

    auto *layout = new QVBoxLayout();
    layout->setSizeConstraint(QVBoxLayout::SetFixedSize);
    layout->addWidget(tabWidget);
    layout->addLayout(buttonLayout);

    dialog->setWindowTitle(object->label());
    dialog->setLayout(layout);
    dialog->setSizeGripEnabled(false);

    int result = dialog->exec();
    delete dialog;
}

void MainWindow::onSlippyMapLayerObjectDoubleClicked(SlippyMapLayerObject* object) {
    if (m_selectedObject == nullptr) return;
    showPropertyPage(object);
}

void MainWindow::onSlippyMapDragFinished()
{
    if (ui->actionWeather_ShowWFOGrid->isChecked()) {
        m_weatherService->getWeatherStationList(
                QPointF(
                        ui->slippyMap->longitude(),
                        ui->slippyMap->latitude()));
    }
}

void MainWindow::onWeatherService_stationListReady(
        const QList<NationalWeatherServiceInterface::WeatherStation>& stations
        )
{
    for (auto *marker : m_weatherStationMarkers) {
        m_layerManager->removeLayerObject(m_weatherLayer, marker);
        delete marker;
    }

    m_weatherStationMarkers.clear();

    for (const auto& station : m_weatherService->stations()) {
        auto *marker = new WeatherStationMarker(station.stationId);
        marker->setEditable(false);
        marker->setLabel(station.stationId);
        marker->setMovable(false);
        marker->setPosition(QPointF(station.longitude, station.latitude));
        m_layerManager->addLayerObject(m_weatherLayer, marker);
        m_weatherStationMarkers.append(marker);
    }
}

void MainWindow::onWeatherService_forecastReady(
        const NationalWeatherServiceInterface::Forecast12Hr& forecast
        )
{

}

void MainWindow::saveWorkspace(QString fileName)
{
    QFile file(fileName);

    if (!file.open(QIODevice::WriteOnly)) {
        qCritical() << "Unable to open file" << fileName;
        QMessageBox::critical(this,
                              tr("File Error"),
                              tr("Unable to open file '%1'").arg(fileName));
        return;
    }

    m_workspaceFileName = fileName;

    QDataStream out(&file);
    out.setVersion(QDataStream::Qt_5_15);

    out << QString("layers");
    out << m_layerManager->objectCount();

    for (auto *layer : m_layerManager->layers()) {
        // todo: serialize the layer
        for (auto *object : layer->objects()) {
            out << QString(object->metaObject()->className());
            object->serialize(out);
        }
    }

    setWorkspaceDirty(false);
}

void MainWindow::onActionFileSaveWorkspaceTriggered()
{
    QString fileName = m_workspaceFileName;
    if (fileName.isEmpty()) {
        fileName = QFileDialog::getSaveFileName(
                this,
                tr("Save Workspace"));
    }

    saveWorkspace(fileName);
}

void MainWindow::onActionFileOpenWorkspaceTriggered()
{
    if (!m_workspaceFileName.isEmpty()) {
        if (!closeWorkspace())
            return;
    }

    QString fileName = QFileDialog::getOpenFileName(
            this,
            tr("Open Workspace"));

    QFile file(fileName);

    if (!file.open(QIODevice::ReadOnly)) {
        qCritical() << "Unable to open file" << fileName;
        QMessageBox::critical(this,
                              tr("File Error"),
                              tr("Unable to open file '%1'").arg(fileName));
        return;
    }

    m_workspaceFileName = fileName;

    QDataStream in(&file);
    in.setVersion(QDataStream::Qt_5_15);

    QString section;
    QString className;
    int count = 0;

    in >> section;

    if (section == "layers") {
        in >> count;

        for (int i = 0; i < count; i++) {
            in >> className;
            className.append("*");

            // get the type information from qt meta
            int typeId = QMetaType::type(className.toLocal8Bit());
            const QMetaObject *metaObject = QMetaType::metaObjectForType(typeId);

            // create a new object and cast to layer object
            QObject *o = metaObject->newInstance();
            auto *object = qobject_cast<SlippyMapLayerObject*>(o);

            // unserialize and add to layer
            object->unserialize(in);
            m_layerManager->addLayerObject(m_layerManager->defaultLayer(), object);
        }
    }

    setWorkspaceDirty(false);
}

void MainWindow::weatherNetworkAccessManager_onRequestFinished(QNetworkReply* reply) {
    if (reply->error() != QNetworkReply::NoError) {
        qWarning() << "Weather request failed:" << reply->errorString();
        reply->deleteLater();
        return;
    }

    QByteArray replyData = reply->readAll();
    QJsonDocument document = QJsonDocument::fromJson(replyData);

    QJsonObject root = document.object();
    QJsonObject properties = document["properties"].toObject();
    QJsonObject relativeLocation = properties["relativeLocation"].toObject();
    QJsonObject relativeLocationGeometry = relativeLocation["geometry"].toObject();
    QJsonObject relativeLocationProperties = relativeLocation["relativeLocation"].toObject();
    QString gridId = properties["gridId"].toString();
    QString city = relativeLocationProperties["city"].toString();
    QString state = relativeLocationProperties["state"].toString();

    QJsonArray relativeLocationGeometryCoords = relativeLocationGeometry["coordinates"].toArray();
    double latitude = relativeLocationGeometryCoords[1].toDouble();
    double longitude = relativeLocationGeometryCoords[0].toDouble();

    SlippyMapWidgetMarker *marker = new SlippyMapWidgetMarker();
    marker->setLabel(gridId);
    marker->setPosition(QPointF(longitude, latitude));
    m_layerManager->addLayerObject(m_weatherLayer, marker);
    m_weatherLayer->setVisible(true);
    m_weatherLayer->setEditable(false);
    qDebug() << "Got weather station" << gridId;
    qDebug() << "Got location" << city << "," << state;
    qDebug() << "Got coordinates" << latitude << "," << longitude;

    QString forecastZoneUrl = properties["forecastZone"].toString();
}

void MainWindow::onGpsDataProviderPositionUpdated(QString identifier, QPointF position, QHash<QString, QVariant> metadata)
{
    SlippyMapWidgetMarker *marker;

    if (m_gpsMarkers.contains(identifier)) {
        marker = m_gpsMarkers[identifier];
        marker->setLabel(metadata["gps_label"].toString());
        marker->setPosition(position);
    }
    else {
        marker = new SlippyMapWidgetMarker(position);
        marker->setLabel(metadata["gps_label"].toString());
        marker->setColor(Qt::green);
        marker->setEditable(false);
        marker->setMovable(false);
        m_gpsMarkers[identifier] = marker;
        m_layerManager->addLayerObject(m_gpsMarkerLayer, marker);
    }

    QString cardinal_lat;
    QString cardinal_lon;

    if (position.y() >= 0) cardinal_lat = "N";
    else cardinal_lat = "S";

    if (position.x() >= 0) cardinal_lon = "E";
    else cardinal_lon = "W";

    m_statusBarGpsStatusLabel->setText(tr("GPS Postion: %1 %2 %3 %4")
        .arg(position.y(), 0, 'f', 7)
        .arg(cardinal_lat)
        .arg(position.x(), 0, 'f', 7)
        .arg(cardinal_lon));
}

void MainWindow::onTvwMarkersContextMenuRequested(const QPoint &point)
{
    qDebug() << "Context menu requested...";
    QModelIndex index = ui->tvwMarkers->indexAt(point);
    if (index.isValid()) {
        if (!index.parent().isValid()) {
            m_markerMenu->exec(ui->tvwMarkers->viewport()->mapToGlobal(point));
        }
    }
    else {
        qDebug() << "No selected item";
    }
}

void MainWindow::onMarkerMenuPropertiesActionTriggered()
{

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

    SlippyMapWidgetMarker *marker = new SlippyMapWidgetMarker(markerPoint);
    marker->setLabel(SlippyMapWidget::latLonToString(lat, lon));
    marker->setDescription(tr("Test Label"));

    m_layerManager->addLayerObject(
        m_layerManager->defaultLayer(),
        marker);

    setWorkspaceDirty(true);
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
    settings.setValue("view/maximized", isMaximized());

    qDebug() << "Is it max?" << (windowState() == Qt::WindowMaximized);
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
    if (index.isValid() && !index.parent().isValid()) {
        int layerIndex = index.row();
        for (int i = 0; i < m_layerManager->layers().count(); i++) {
            if (i == layerIndex) {
                qDebug() << "Activating layer" << m_layerManager->layers().at(i)->name();
                m_layerManager->setActiveLayer(m_layerManager->layers().at(i));
                break;
            }
        }
    }

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

        for (const GPXTrack& track : parser.tracks()) {
            qDebug() << "Found" << track.segments().count() << "segments";
            for (const GPXTrackSegment& segment : track.segments()) {
                qDebug() << "Found" << segment.points().count() << "points";
                auto *layerTrack = new SlippyMapLayerTrack(track);
                QString trackName = parser.metadata().name();
                if (trackName.isEmpty()) {
                    QFileInfo trackFileInfo(fileName);
                    trackName = trackFileInfo.baseName();
                }
                layerTrack->setLabel(trackName);
                layerTrack->setDescription(parser.metadata().description());
                m_layerManager->addLayerObject(m_defaultMarkerLayer, layerTrack);
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

void MainWindow::setWorkspaceDirty(bool dirty)
{
    QString fileName = m_workspaceFileName;
    if (fileName.isEmpty())
        fileName = "Untitled.osm";

    m_workspaceDirty = dirty;

    if (dirty) {
        ui->actionFile_SaveWorkspace->setEnabled(true);
        setWindowTitle(tr("OSMExplorer") + " - " + fileName + "*");
    }
    else {
        ui->actionFile_SaveWorkspace->setEnabled(false);
        setWindowTitle(tr("OSMExplorer") + " - " + fileName);
    }
}

bool MainWindow::closeWorkspace()
{
    if (m_workspaceDirty) {
        int result = QMessageBox::question(this,
                              tr("Save Workspace?"),
                              tr("You have unsaved changes. Would you like to save?"),
                              QMessageBox::StandardButtons(QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel));

        if (result == QMessageBox::Yes) {
            onActionFileSaveWorkspaceTriggered();
            return true;
        }
        else if (result == QMessageBox::Cancel)
            return false;
    }

    for (auto *layer : m_layerManager->layers()) {
        layer->removeAll();
    }
    m_workspaceFileName.clear();
    setWorkspaceDirty(true);
    return true;
}

void MainWindow::onActionFileCloseWorkspaceTriggered()
{
    closeWorkspace();
}

void MainWindow::updateRecentFileList()
{
    ui->menuRecent->clear();
}
