#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QAction>
#include <QFile>
#include <QFileDialog>
#include <QGraphicsDropShadowEffect>
#include <QGuiApplication>
#include <QInputDialog>
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
#include <QSqlError>
#include <QSqlQuery>
#include <QStandardPaths>
#include <QTimer>
#include <QVector>

#include <QtColorWidgets/ColorSelector>

#include <boost/geometry.hpp>
#include <boost/geometry/geometries/point_xy.hpp>
#include <boost/geometry/geometries/polygon.hpp>

#include <SlippyMap/SlippyMapWidget.h>
#include <SlippyMap/SlippyMapLayer.h>
#include <SlippyMap/SlippyMapWidgetLayer.h>
#include <SlippyMap/SlippyMapLayerPolygon.h>
#include <SlippyMap/SlippyMapWidgetMarker.h>
#include <SlippyMap/SlippyMapLayerManager.h>
#include <SlippyMap/SlippyMapLayerObjectPropertyPage.h>
#include <SlippyMap/SlippyMapLayerObjectCommonPropertyPage.h>
#include <SlippyMap/SlippyMapLayerPath.h>

#include "Map/SlippyMapLayerTrack.h"
#include "Weather/WeatherForecastWindow.h"
#include "Application/ExplorerApplication.h"
#include "Application/HistoryManager.h"
#include "Application/DatabaseManager.h"
#include "Dialog/DatabaseConnectionDialog.h"
#include "Database/DatabaseObjectPropertyPage.h"

#include "defaults.h"
#include "config.h"
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
#include "Map/SlippyMapGpsMarker.h"
#include "Weather/WeatherStationMarker.h"
#include "Weather/WeatherStationPropertyPage.h"

#ifdef QT_DEBUG
#include <QDebug>

#endif

using namespace SlippyMap;
using namespace color_widgets;
using namespace boost::geometry;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_forecastZonePolygon(nullptr),
    m_weatherStationMarker(nullptr)
{
    ui->setupUi(this);
    setWindowTitle(tr("OSMExplorer") + " - Untitled.osm");
    loadPluginLayers();

    {
        auto *dropShadow = new QGraphicsDropShadowEffect();
        dropShadow->setBlurRadius(30);
        dropShadow->setOffset(3);
        dropShadow->setColor(QColor("#66000000"));
        ui->tvwMarkers->setGraphicsEffect(dropShadow);
    }

    {
        auto *dropShadow = new QGraphicsDropShadowEffect();
        dropShadow->setBlurRadius(30);
        dropShadow->setOffset(3);
        dropShadow->setColor(QColor("#66000000"));
        ui->zoomInButton->setGraphicsEffect(dropShadow);
    }

    {
        auto *dropShadow = new QGraphicsDropShadowEffect();
        dropShadow->setBlurRadius(30);
        dropShadow->setOffset(3);
        dropShadow->setColor(QColor("#66000000"));
        ui->zoomOutButton->setGraphicsEffect(dropShadow);
    }

    {
        auto *dropShadow = new QGraphicsDropShadowEffect();
        dropShadow->setBlurRadius(30);
        dropShadow->setOffset(3);
        dropShadow->setColor(QColor("#66000000"));
        ui->currentLocationButton->setGraphicsEffect(dropShadow);
    }


    ui->tabShapeEditor->setVisible(false);
    m_defaultPalette = qApp->palette();

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

    connect(ui->actionExit,
        &QAction::triggered,
        this,
        &MainWindow::close);

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

    m_historyManager = ExplorerApplication::historyManager();

    connect(m_historyManager,
            &HistoryManager::undoEventAdded,
            this,
            &MainWindow::undoEventAdded);

    connect(m_historyManager,
            &HistoryManager::redoHistoryCleared,
            this,
            &MainWindow::redoHistoryCleared);

    ui->actionEdit_Undo->setEnabled(false);
    connect(ui->actionEdit_Undo,
            &QAction::triggered,
            this,
            &MainWindow::undo);

    ui->actionEdit_Redo->setEnabled(false);
    connect(ui->actionEdit_Redo,
            &QAction::triggered,
            this,
            &MainWindow::redo);

    connect(ui->actionEdit_Cut,
            &QAction::triggered,
            this,
            &MainWindow::cutActiveObject);

    connect(ui->actionEdit_Copy,
            &QAction::triggered,
            this,
            &MainWindow::copyActiveObject);

    connect(ui->actionEdit_Paste,
            &QAction::triggered,
            this,
            &MainWindow::pasteObject);

    connect(ui->actionEdit_Delete,
            &QAction::triggered,
            this,
            &MainWindow::deleteActiveObject);

    /*
     * Database Connection Stuff
     */
    connect(ui->actionFile_ConnectToDatabase,
            &QAction::triggered,
            this,
            &MainWindow::connectToDatabase);

    /*
     * Drawing
     */
    connect(ui->actionDrawPolygon,
            &QAction::triggered,
            this,
            &MainWindow::startPolygonSelection);

    /*
     * New layer action
     */
    connect(ui->actionLayer_New,
            &QAction::triggered,
            this,
            &MainWindow::createNewLayer);

    /*
     * Delete layer action
     */
    connect(ui->actionLayer_Delete,
            &QAction::triggered,
            this,
            &MainWindow::deleteSelectedLayer);

    /*
     * TreeView signals and slots
     */
    connect(ui->tvwMarkers,
            &QTreeView::customContextMenuRequested,
            this,
            &MainWindow::onTvwMarkersContextMenuRequested);

    connect(ui->tvwMarkers,
            &QTreeView::clicked,
            this,
            &MainWindow::onTvwMarkersClicked);

    loadStartupSettings();
    loadLayers();
    setupMap();
    setupWeather();
    setupContextMenus();
    refreshSettings();
    setupToolbar();

    m_statusBarPositionLabel = new QLabel();
    m_statusBarPositionLabel->setFrameStyle(QFrame::Sunken);
    m_statusBarStatusLabel = new QLabel();

    m_statusBarGpsStatusLabel = new QLabel();
    m_statusBarGpsStatusLabel->setFrameStyle(QFrame::Sunken);
    m_statusBarGpsStatusLabel->setText("GPS Position: 0.000 N 0.000 E");

    statusBar()->addPermanentWidget(m_statusBarStatusLabel, 1);
    statusBar()->addPermanentWidget(m_statusBarGpsStatusLabel);
    statusBar()->addPermanentWidget(m_statusBarPositionLabel);

    ui->slippyMap->setFocus(Qt::OtherFocusReason);

    m_animationTimer = new QTimer();
    m_animationTimer->setInterval(1000);
    connect(m_animationTimer,
            &QTimer::timeout,
            this,
            &MainWindow::onAnimationTimerTimeout);
    m_animationTimer->start();

    m_saveWindowSizeTimer = new QTimer();
    m_saveWindowSizeTimer->setSingleShot(true);
    m_saveWindowSizeTimer->setInterval(1000);
    connect(m_saveWindowSizeTimer,
            &QTimer::timeout,
            this,
            &MainWindow::onWindowSizeTimerTimeout);

    for (ExplorerPluginInterface *plugin : ExplorerApplication::pluginManager()->getPlugins()) {
        QList<QDockWidget *> dockWidgets = plugin->dockWidgetList();
        for (QDockWidget *dockWidget : dockWidgets) {
            dockWidget->setParent(this);
            addDockWidget(Qt::LeftDockWidgetArea, dockWidget);
        }
    }

    loadMarkers();

    /*
     * Default layer setup
     */
    m_defaultMarkerLayer = new SlippyMapLayer();
    m_defaultMarkerLayer->setName(tr("Layer1"));
    m_layerManager->addLayer(m_defaultMarkerLayer);
    m_layerManager->setDefaultLayer(m_defaultMarkerLayer);

    setWorkspaceDirty(false);
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
    QSettings settings;
    auto *pluginManager = ExplorerApplication::pluginManager();

    for (auto *plugin : pluginManager->getPlugins()) {
        for (auto *tileLayer : plugin->tileLayers()) {
            QString key = QString("plugins/%1/layers/%2/visible")
                    .arg(plugin->name())
                    .arg(tileLayer->name());
            bool visible = settings.value(key, true).toBool();

            tileLayer->setVisible(visible);
            ui->slippyMap->addLayer(tileLayer);

            auto *action = new QAction();
            action->setText(tileLayer->name());
            action->setCheckable(true);
            action->setChecked(visible);
            connect(action,
                    &QAction::triggered,
                    [this, key, tileLayer](bool checked) {
                        tileLayer->setVisible(checked);
                        QSettings settings;
                        settings.setValue(key, checked);
                    });

            ui->menuLayer_TileLayers->addAction(action);
        }
    }


    for (auto *layer : pluginManager->getLayers()) {
        m_layerManager->addLayer(layer);
    }
}

void MainWindow::loadMarkers()
{

}

void MainWindow::setupWeather()
{
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
}

void MainWindow::setupContextMenus()
{
    /*
     * Layer tree view context menu
     */

    m_treeViewMenu = new QMenu(this);
    m_treeViewMenu->setTitle(tr("Layers"));

    m_newLayerAction = new QAction();
    m_newLayerAction->setText(tr("New Layer"));
    m_treeViewMenu->addAction(m_newLayerAction);
    connect(m_newLayerAction,
            &QAction::triggered,
            this,
            &MainWindow::createNewLayer);

    m_renameLayerAction = new QAction();
    m_renameLayerAction->setText(tr("Rename..."));
    m_treeViewMenu->addAction(m_renameLayerAction);
    connect(m_renameLayerAction,
            &QAction::triggered,
            this,
            &MainWindow::renameActiveLayer);
    connect(ui->actionLayer_Rename,
            &QAction::triggered,
            this,
            &MainWindow::renameActiveLayer);

    m_deleteLayerAction = new QAction();
    m_deleteLayerAction->setText(tr("Delete"));
    m_treeViewMenu->addAction(m_deleteLayerAction);
    connect(m_deleteLayerAction,
            &QAction::triggered,
            this,
            &MainWindow::deleteSelectedLayer);
    connect(ui->actionLayer_Delete,
            &QAction::triggered,
            this,
            &MainWindow::deleteActiveLayer);

    m_clearLayerAction = new QAction();
    m_clearLayerAction->setText(tr("Clear Layer"));
    m_treeViewMenu->addAction(m_clearLayerAction);
    connect(m_clearLayerAction,
            &QAction::triggered,
            this,
            &MainWindow::clearSelectedLayer);

    m_treeViewMenu->addSeparator();

    m_markerVisibilityAction = new QAction();
    m_markerVisibilityAction->setText(tr("Visible"));
    m_markerVisibilityAction->setCheckable(true);
    m_treeViewMenu->addAction(m_markerVisibilityAction);
    connect(m_markerVisibilityAction, &QAction::toggled, [this](bool checked) {
        QModelIndex index = ui->tvwMarkers->currentIndex();
        Q_ASSERT(index.row() < m_layerManager->layers().count());
        SlippyMapLayer *layer = m_layerManager->layers().at(index.row());
        layer->setVisible(checked);
    });

    m_markerLockedAction = new QAction();
    m_markerLockedAction->setText(tr("Locked"));
    m_markerLockedAction->setCheckable(true);
    m_treeViewMenu->addAction(m_markerLockedAction);
    connect(m_markerLockedAction,
            &QAction::toggled,
            [this](bool checked) {
                QModelIndex index = ui->tvwMarkers->currentIndex();
                if (!index.isValid() || index.parent().isValid()) return;
                Q_ASSERT(index.row() < m_layerManager->layers().count());
                SlippyMapLayer *layer = m_layerManager->layers().at(index.row());
                layer->setEditable(!checked);
            });

    connect(ui->tvwMarkers, &QTreeView::clicked, [this](const QModelIndex& index) {
        if (!index.isValid() || index.parent().isValid()) return;
        Q_ASSERT(index.row() < m_layerManager->layers().count());
        SlippyMapLayer *layer = m_layerManager->layers().at(index.row());
        m_markerVisibilityAction->setChecked(layer->isVisible());
        m_markerLockedAction->setChecked(!layer->isEditable());
    });

    connect(ui->tvwMarkers, &QTreeView::activated, [this](const QModelIndex& index) {
        if (!index.isValid() || index.parent().isValid()) return;
        Q_ASSERT(index.row() < m_layerManager->layers().count());
        SlippyMapLayer *layer = m_layerManager->layers().at(index.row());
        m_markerVisibilityAction->setChecked(layer->isVisible());
        m_markerLockedAction->setChecked(!layer->isEditable());
    });

    /*
     * SlippyMapWidget context menu
     */

    m_coordAction = new QAction();
    m_coordAction->setEnabled(false);

    m_addMarkerAction = new QAction();
    m_addMarkerAction->setText(tr("Add Marker"));
    connect(m_addMarkerAction,
            &QAction::triggered,
            this,
            &MainWindow::createMarkerAtContextMenuPosition);
    connect(ui->actionObject_NewMarkerAtCurrent,
            &QAction::triggered,
            this,
            &MainWindow::createMarkerAtCurrentPosition);

    m_deleteObjectAction = new QAction();
    m_deleteObjectAction->setText(tr("Delete..."));
    connect(m_deleteObjectAction,
            &QAction::triggered,
            this,
            &MainWindow::deleteActiveObject);
    connect(ui->actionObject_Delete,
            &QAction::triggered,
            this,
            &MainWindow::deleteActiveObject);

    m_objectPropertiesAction = new QAction();
    m_objectPropertiesAction->setText(tr("Properties..."));
    connect(m_objectPropertiesAction,
            &QAction::triggered,
            this,
            &MainWindow::showActiveObjectPropertyPage);
    connect(ui->actionObject_Properties,
            &QAction::triggered,
            this,
            &MainWindow::showActiveObjectPropertyPage);

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
    m_contextMenu->addAction(m_deleteObjectAction);
    m_contextMenu->addAction(m_objectPropertiesAction);
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

void MainWindow::setupMap()
{
    m_layerManager = ExplorerApplication::layerManager();

    ui->slippyMap->setLayerManager(m_layerManager);
    ui->tvwMarkers->setModel(m_layerManager);
    ui->tvwMarkers->setContextMenuPolicy(Qt::CustomContextMenu);

    /*
     * GPS Marker Layer
     */
    m_gpsMarkerLayer = new SlippyMapLayer();
    m_gpsMarkerLayer->setName(tr("GPS"));
    m_gpsMarkerLayer->setEditable(false);
    m_layerManager->addLayer(m_gpsMarkerLayer);

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
    connect(ui->slippyMap, &SlippyMapWidget::polygonSelected, this, &MainWindow::onSlippyMapPolygonSelected);
    connect(ui->slippyMap, &SlippyMapWidget::pathSelected, this, &MainWindow::onSlippyMapPathSelected);
    connect(ui->slippyMap, &SlippyMapWidget::objectActivated, this, &MainWindow::onSlippyMapLayerObjectActivated);
    connect(ui->slippyMap, &SlippyMapWidget::objectDeactivated, this, &MainWindow::onSlippyMapLayerObjectDeactivated);
    connect(ui->slippyMap, &SlippyMapWidget::objectDoubleClicked, this, &MainWindow::onSlippyMapLayerObjectDoubleClicked);
    connect(ui->slippyMap, &SlippyMapWidget::objectWasDragged, this, &MainWindow::onSlippyMapLayerObjectWasDragged);
    connect(ui->slippyMap, &SlippyMapWidget::dragFinished, this, &MainWindow::onSlippyMapDragFinished);

    connect(ui->zoomInButton,
            &QPushButton::clicked,
            ui->slippyMap,
            &SlippyMapWidget::increaseZoomLevel);
    connect(ui->zoomOutButton,
            &QPushButton::clicked,
            ui->slippyMap,
            &SlippyMapWidget::decreaseZoomLevel);
}

void MainWindow::loadStartupSettings()
{
    QSettings settings;

    if (settings.contains("view/windowWidth") && settings.contains("view/windowHeight")) {
        if (settings.value("view/maximized", false).toBool()) {
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

    double defLat = settings.value("map/defaults/latitude", DEFAULT_LATITUDE).toDouble();
    double defLon = settings.value("map/defaults/longitude", DEFAULT_LONGITUDE).toDouble();
    int defZoom = settings.value("map/defaults/zoomLevel", DEFAULT_ZOOM).toInt();
    ui->slippyMap->setCenter(defLat, defLon);
    ui->slippyMap->setZoomLevel(defZoom);
}

void MainWindow::loadLayers()
{
    QSettings settings;

    for (SlippyMapWidgetLayer *tileLayer: m_layers) {
        ui->slippyMap->takeLayer(tileLayer);
        tileLayer->deleteLater();
    }

    for (QAction *action: m_layerShowHideActions)
        ui->menuLayer_TileLayers->removeAction(action);

    m_layers.clear();
    m_layerShowHideActions.clear();

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
        m_layerShowHideActions.append(layerShowHide);
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
    /**
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

    /**
     * Color and stroke width selectors for drawing
     */
    m_strokeColorSelector = new ColorSelector();
    m_strokeColorSelector->setColor(qApp->palette().text().color());
    m_strokeColorSelector->setToolTip(tr("Stroke color"));
    connect(m_strokeColorSelector,
            &ColorSelector::colorSelected,
            [this](const QColor& color) {
                ui->slippyMap->setDrawingStrokeColor(color.lighter());
            });

    m_fillColorSelector = new ColorSelector();
    m_fillColorSelector->setColor(qApp->palette().highlight().color());
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

    m_lineWidth = new QSpinBox();
    m_lineWidth->setMinimum(1);
    m_lineWidth->setValue(10);
    m_lineWidth->setToolTip(tr("Path line width"));
    connect(m_lineWidth,
            QOverload<int>::of(&QSpinBox::valueChanged),
            [this](int value) {
        ui->slippyMap->setDrawingLineWidth(value);
    });

    ui->slippyMap->setDrawingFillColor(m_fillColorSelector->color());
    ui->slippyMap->setDrawingLineWidth(m_lineWidth->value());
    ui->slippyMap->setDrawingStrokeColor(m_strokeColorSelector->color());
    ui->slippyMap->setDrawingStrokeWidth(m_strokeWidth->value());

    /*
     * Animation buttons
     */
    connect(ui->actionLayer_Play,
            &QAction::toggled,
            [this](bool state) {
        if (state)
            m_animationState = Forward;
        else
            m_animationState = Paused;
    });

    connect(ui->actionLayer_NextFrame,
            &QAction::triggered,
            [this]() {
        ui->slippyMap->nextFrame();
    });

    connect(ui->actionLayer_PreviousFrame,
            &QAction::triggered,
            [this]() {
        ui->slippyMap->previousFrame();
    });

    ui->toolBar->addAction(ui->actionFile_NewWorkspace);
    ui->toolBar->addAction(ui->actionFile_SaveWorkspace);
    ui->toolBar->addAction(ui->actionFile_OpenWorkspace);
    ui->toolBar->addSeparator();
    ui->toolBar->addAction(ui->actionDrawLine);
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
    ui->toolBar->addWidget(m_lineWidth);
    ui->toolBar->addWidget(m_strokeWidth);
    ui->toolBar->addSeparator();
    ui->toolBar->addAction(ui->actionLayer_PreviousFrame);
    ui->toolBar->addAction(ui->actionLayer_Play);
    ui->toolBar->addAction(ui->actionLayer_NextFrame);
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
    QPointF menuPosition = QPointF(
            ui->slippyMap->widgetX2long(point.x()),
            ui->slippyMap->widgetY2lat(point.y())
            );

    m_coordAction->setText(SlippyMapWidget::latLonToString(
                               ui->slippyMap->widgetY2lat(point.y()),
                               ui->slippyMap->widgetX2long(point.x())));

    m_addMarkerAction->setVisible(true);
    m_deleteObjectAction->setVisible(false);
    m_objectPropertiesAction->setVisible(false);
    m_editShapeAction->setVisible(false);
    m_deleteShapeAction->setVisible(false);

    QRectF viewport = ui->slippyMap->boundingBoxLatLon();
    for (SlippyMapLayer *layer : m_layerManager->layers()) {
        for (SlippyMapLayerObject *object : layer->objects()) {
            if (viewport.contains(object->position())) {
                if (object->contains(menuPosition, ui->slippyMap->zoomLevel())) {
                    ui->slippyMap->setActiveObject(object);
                    m_coordAction->setText(object->label());
                    m_addMarkerAction->setVisible(false);
                    m_objectPropertiesAction->setVisible(true);
                    m_deleteObjectAction->setVisible(true);
                    break;
                }
            }
        }
    }

    m_contextMenuPoint = menuPosition;
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

    createUndoAddObject(
            tr("New Rectangle"),
            m_layerManager->activeLayer(),
            poly);
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
    case SlippyMapWidget::PathDrawing:
        ui->actionDrawLine->setChecked(true);
        ui->actionDrawRectangle->setChecked(false);
        ui->actionDrawPolygon->setChecked(false);
        ui->actionDrawEllipse->setChecked(false);
        break;
    case SlippyMapWidget::RectDrawing:
        ui->actionDrawLine->setChecked(false);
        ui->actionDrawRectangle->setChecked(true);
        ui->actionDrawPolygon->setChecked(false);
        ui->actionDrawEllipse->setChecked(false);
        break;
    case SlippyMapWidget::EllipseDrawing:
        ui->actionDrawLine->setChecked(false);
        ui->actionDrawRectangle->setChecked(false);
        ui->actionDrawPolygon->setChecked(false);
        ui->actionDrawEllipse->setChecked(true);
        break;
    case SlippyMapWidget::PolygonDrawing:
        ui->actionDrawLine->setChecked(false);
        ui->actionDrawRectangle->setChecked(false);
        ui->actionDrawPolygon->setChecked(true);
        ui->actionDrawEllipse->setChecked(false);
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
        &SlippyMapLayerObjectPropertyPage::updateUi);

    QList<SlippyMapLayerObjectPropertyPage*> propertyPages;
    propertyPages.append(commonPropertyPage);

    for (auto *propertyPage : object->propertyPages()) {
        propertyPages.append(propertyPage);

        connect(object,
                &SlippyMapLayerObject::updated,
                propertyPage,
                &SlippyMapLayerObjectPropertyPage::updateUi);
    }

    for (auto *propertyPage : ExplorerApplication::pluginManager()->getPropertyPages()) {
        propertyPages.append(propertyPage);

        connect(object,
                &SlippyMapLayerObject::updated,
                propertyPage,
                &SlippyMapLayerObjectPropertyPage::updateUi);
    }

    ui->tabShapeEditor->clear();
    for (auto *propertyPage : propertyPages) {
        propertyPage->setupUi();
        ui->tabShapeEditor->addTab(propertyPage, propertyPage->tabTitle());
    }
    ui->tabShapeEditor->setVisible(true);
    ui->lblNoShapeSelected->setVisible(false);

    delete m_selectedObjectCopy;
    m_selectedObjectCopy = object->clone();
    m_selectedObject = object;

    ui->actionEdit_Copy->setEnabled(true);
    ui->actionEdit_Cut->setEnabled(true);
    ui->actionEdit_Delete->setEnabled(true);
    ui->actionEdit_Rename->setEnabled(true);
    ui->actionEdit_Properties->setEnabled(true);
}

void MainWindow::onSlippyMapLayerObjectDeactivated(SlippyMapLayerObject *object)
{
    (void)object;

    if (m_selectedObject != nullptr) {
        ui->tabShapeEditor->removeTab(1);
        ui->tabShapeEditor->setVisible(false);
        ui->lblNoShapeSelected->setVisible(true);
        m_selectedObject = nullptr;
        ui->actionEdit_Copy->setEnabled(false);
        ui->actionEdit_Cut->setEnabled(false);
        ui->actionEdit_Delete->setEnabled(false);
        ui->actionEdit_Rename->setEnabled(false);
        ui->actionEdit_Properties->setEnabled(false);
    }
}

void MainWindow::onSlippyMapLayerObjectWasDragged(SlippyMapLayerObject *object)
{
    object->setSynced(false);
    createUndoModifyObject(
            tr("Move %1").arg(object->label()),
            object);
}

void MainWindow::showPropertyPage(SlippyMapLayerObject *object)
{
    QList<SlippyMapLayerObjectPropertyPage*> propertyPages;
    propertyPages.append(new SlippyMapLayerObjectCommonPropertyPage(object, m_layerManager));

    for (auto *propertyPage : object->propertyPages())
        propertyPages.append(propertyPage);

    if (m_databaseMode)
        propertyPages.append(new DatabaseObjectPropertyPage(object));

    for (auto *propertyPage : ExplorerApplication::pluginManager()->getPropertyPages())
        propertyPages.append(propertyPage);

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
                    createUndoModifyObject(
                            tr("Edit ") + " " + object->label(),
                            object);

                    for (auto *propertyPage: propertyPages)
                        propertyPage->save();

                    object->setSynced(false);
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

    loadViewportData();
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
    if (m_databaseMode) {
        saveWorkspaceToDatabase();
        return;
    }

    QFile file(fileName);

    if (!file.open(QIODevice::WriteOnly)) {
        qCritical() << "Unable to open file" << fileName;
        QMessageBox::critical(this,
                              tr("File Error"),
                              tr("Unable to open file '%1'").arg(fileName));
        return;
    }

    m_workspaceFileName = fileName;

    int layerCount = 0;
    for (auto *layer: m_layerManager->layers()) {
        if (layer->isEditable())
            layerCount++;
    }

    QDataStream out(&file);
    out.setVersion(QDataStream::Qt_5_15);

    out << QString("layers");
    out << layerCount;

    for (auto *layer : m_layerManager->layers()) {
        if (layer->isEditable()) {
            out << layer->name();
            out << layer->description();
            out << layer->isVisible();
            out << layer->isEditable();
            out << layer->objects().count();

            for (auto *object : layer->objects()) {
                //out << QMetaType::type(object->metaObject()->className());
                out << QString(object->metaObject()->className());
                qDebug() << "Saving object type:" << QString(object->metaObject()->className());
                object->serialize(out);
            }
        }
    }

    setWorkspaceDirty(false);
}

void MainWindow::onActionFileSaveWorkspaceTriggered()
{
    if (m_databaseMode) {
        saveWorkspaceToDatabase();
        return;
    }

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
    if (m_workspaceDirty) {
        if (!closeWorkspace())
            return;
    }

    QString fileName = QFileDialog::getOpenFileName(
            this,
            tr("Open Workspace"));

    if (fileName.isEmpty()) return;

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
    QString layerName;
    QString layerDescription;
    bool layerVisible;
    bool layerEditable;
    int objectCount = 0;
    int layerCount = 0;

    in >> section;

    if (section == "layers") {
        in >> layerCount;

        for (int i = 0; i < layerCount; i++) {
            in >> layerName;
            in >> layerDescription;
            in >> layerVisible;
            in >> layerEditable;
            in >> objectCount;

            auto *layer = new SlippyMapLayer();
            layer->setName(layerName);
            layer->setDescription(layerDescription);
            layer->setVisible(layerVisible);
            layer->setEditable(layerEditable);

            m_layerManager->addLayer(layer);

            for (int j = 0; j < objectCount; j++) {
                in >> className;
                className.append("*");

                // get the type information from qt meta
                int typeId = QMetaType::type(className.toLocal8Bit());
                const QMetaObject *metaObject = QMetaType::metaObjectForType(typeId);

                // create a new object and cast to layer object
                QObject *o = metaObject->newInstance();
                auto *object = qobject_cast<SlippyMapLayerObject *>(o);

                // unserialize and add to layer
                object->unserialize(in);
                m_layerManager->addLayerObject(layer, object);
            }
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

    QString forecastZoneUrl = properties["forecastZone"].toString();
}

void MainWindow::onGpsDataProviderPositionUpdated(QString identifier, QPointF position, QHash<QString, QVariant> metadata)
{
    SlippyMapGpsMarker *marker;

    if (m_gpsMarkers.contains(identifier)) {
        marker = m_gpsMarkers[identifier];
        marker->setLabel(metadata["gps_label"].toString());
        marker->setPosition(position);
    }
    else {
        marker = new SlippyMapGpsMarker(position);
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

void MainWindow::onGpsDataProviderSatellitesUpdated(QString identifier,
                                                    const QList<NmeaSerialLocationDataProvider::SatelliteStatus> &satellites,
                                                    QHash<QString, QVariant> metadata)
{
    SlippyMapGpsMarker *marker;
    if (m_gpsMarkers.contains(identifier)) {
        marker = m_gpsMarkers[identifier];
        marker->setSatellites(satellites);
    }
}

void MainWindow::onTvwMarkersContextMenuRequested(const QPoint &point)
{
    QModelIndex index = ui->tvwMarkers->indexAt(point);
    if (index.isValid()) {
        if (!index.parent().isValid()) {
            SlippyMapLayer *layer = m_layerManager->layers().at(index.row());

            m_clearLayerAction->setVisible(true);
            m_deleteLayerAction->setVisible(true);
            m_renameLayerAction->setVisible(true);

            // can only do this if layer is editable
            m_clearLayerAction->setEnabled(layer->isEditable());
            m_deleteLayerAction->setEnabled(layer->isEditable());
            m_renameLayerAction->setEnabled(layer->isEditable());

            m_newLayerAction->setVisible(false);
            m_treeViewMenu->exec(ui->tvwMarkers->viewport()->mapToGlobal(point));
        }
    }
    else {
        m_newLayerAction->setVisible(true);
        m_clearLayerAction->setVisible(false);
        m_deleteLayerAction->setVisible(false);
        m_renameLayerAction->setVisible(false);
        m_treeViewMenu->exec(ui->tvwMarkers->viewport()->mapToGlobal(point));
    }
}

void MainWindow::showActiveObjectPropertyPage()
{
    if (m_selectedObject == nullptr) return;
    showPropertyPage(m_selectedObject);
}

void MainWindow::onPluginLayerObjectProviderMarkerAdded(SlippyMapLayerObject *object)
{
    //ui->slippyMap->addMarker(marker);

}

void MainWindow::createMarkerAtContextMenuPosition()
{
    double lon = ui->slippyMap->widgetX2long(m_contextMenuLocation.x());
    double lat = ui->slippyMap->widgetY2lat(m_contextMenuLocation.y());
    QPointF position(lon, lat);
    createMarkerAtPosition(position);
}

void MainWindow::createMarkerAtCurrentPosition()
{
    QPointF position = ui->slippyMap->position();
    createMarkerAtPosition(position);
}

void MainWindow::createMarkerAtPosition(const QPointF& position)
{
    SlippyMapWidgetMarker *marker = new SlippyMapWidgetMarker(position);
    marker->setLabel(SlippyMapWidget::latLonToString(position.y(), position.x()));
    marker->setDescription(tr("Test Label"));
    marker->setColor(m_fillColorSelector->color());

    SlippyMapLayer *target = m_layerManager->activeLayer();
    if (target == nullptr)
        target = m_layerManager->defaultLayer();
    if (target == nullptr)
        return;

    if (!target->isEditable()) {
        QMessageBox::warning(this,
                             tr("Layer Locked"),
                             tr("Cannot create marker on a locked layer."),
                             QMessageBox::StandardButtons(QMessageBox::Ok));
        return;
    }

    m_layerManager->addLayerObject(target, marker);
    setWorkspaceDirty(true);

    createUndoAddObject(
            tr("New Marker"),
            target,
            marker);
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

void MainWindow::onWindowSizeTimerTimeout()
{
    QSettings settings;
    settings.setValue("view/windowWidth", width());
    settings.setValue("view/windowHeight", height());
    settings.setValue("view/maximized", isMaximized());
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
        //connect(m_settingsDialog, &SettingsDialog::accepted, this, &MainWindow::refreshSettings);
    }

    int result = m_settingsDialog->exec();

    if (result == QDialog::Accepted) {
        if (m_settingsDialog->layersChanged())
            loadLayers();

        QSettings settings;
        bool enable = settings.value("map/zoom/centerOnCursor", DEFAULT_CENTER_ON_CURSOR_ZOOM).toBool();
        ui->slippyMap->setCenterOnCursorWhileZooming(enable);
        ui->slippyMap->setTileCacheDir(settings.value("map/cache/tiledir", QStandardPaths::writableLocation(QStandardPaths::CacheLocation)).toString());
        ui->slippyMap->setTileCachingEnabled(settings.value("map/cache/enable", true).toBool());
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

            connect(provider,
                    &LocationDataProvider::positionUpdated,
                    this,
                    &MainWindow::onGpsDataProviderPositionUpdated);

            connect(provider,
                    &NmeaSerialLocationDataProvider::satellitesUpdated,
                    this,
                    &MainWindow::onGpsDataProviderSatellitesUpdated);

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
                m_layerManager->setActiveLayer(m_layerManager->layers().at(i));
                break;
            }
        }
    }
    else {
        auto *object = static_cast<SlippyMapLayerObject*>(index.internalPointer());
        if (object != nullptr)
            showPropertyPage(object);
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

void MainWindow::on_actionImport_GPX_triggered()
{
    QString fileName = QFileDialog::getOpenFileName(
                this,
                tr("Choose Import File"),
                QDir::homePath(),
                tr("GPX Files (*.gpx)"));

    QFile file(fileName);

    if (file.open(QFile::ReadOnly)) {
        GPXParser parser;
        parser.read(&file);

        for (const GPXTrack& track : parser.tracks()) {
            for (const GPXTrackSegment& segment : track.segments()) {
                auto *layerTrack = new SlippyMapLayerTrack(track);
                QString trackName = parser.metadata().name();

                qDebug() << "Name:" << parser.metadata().name();
                qDebug() << "Time:" << parser.metadata().time();
                qDebug() << "Author:" << parser.metadata().author().name();
                qDebug() << "Bounds:" << parser.metadata().bounds();

                if (trackName.isEmpty()) {
                    QFileInfo trackFileInfo(fileName);
                    trackName = trackFileInfo.baseName();
                }

                layerTrack->setLabel(trackName);
                layerTrack->setDescription(parser.metadata().description());
                layerTrack->setLineColor(m_fillColorSelector->color());
                layerTrack->setStrokeColor(m_strokeColorSelector->color());
                layerTrack->setLineWidth(m_lineWidth->value());
                layerTrack->setStrokeWidth(m_strokeWidth->value());
                layerTrack->setWaypointColor(m_fillColorSelector->color().lighter());
                layerTrack->setWaypointRadius(m_lineWidth->value());
                m_layerManager->addLayerObject(layerTrack);

                createUndoAddObject(
                        tr("Import Track"),
                        m_layerManager->activeLayer(),
                        layerTrack);
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
        if (result == QMessageBox::Cancel)
            return false;
    }

    for (auto *layer : m_layerManager->layers()) {
        if (layer->isEditable()) {
            m_layerManager->takeLayer(layer);
            layer->removeAll();
            delete layer;
        }
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

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (m_workspaceDirty) {
        if (!closeWorkspace()) {
            event->ignore();
            return;
        }
    }

    QSqlDatabase::database().close();

    event->accept();
}

void MainWindow::createNewLayer()
{
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
        createUndoAddLayer(tr("New Layer"), newLayer);
    }
}

void MainWindow::deleteSelectedLayer()
{
    QModelIndex selectedLayer = ui->tvwMarkers->currentIndex();
    if (selectedLayer.isValid() && !selectedLayer.parent().isValid()) {
        SlippyMapLayer *layer = m_layerManager->layers().at(selectedLayer.row());
        if (layer->isEditable()) {
            int result = QMessageBox::question(
                    this,
                    tr("Delete Layer"),
                    tr("Do you want to delete the layer '%1'?").arg(layer->name()),
                    QMessageBox::StandardButtons(QMessageBox::Yes | QMessageBox::No));

            if (result == QMessageBox::Yes) {
                m_layerManager->takeLayer(layer);
                createUndoDeleteLayer(
                        tr("Delete Layer"),
                        layer);
            }
        }
    }
}

void MainWindow::clearSelectedLayer()
{
    QModelIndex selectedLayer = ui->tvwMarkers->currentIndex();
    if (selectedLayer.isValid() && !selectedLayer.parent().isValid()) {
        SlippyMapLayer *layer = m_layerManager->layers().at(selectedLayer.row());
        if (layer->isEditable()) {
            int result = QMessageBox::question(
                    this,
                    tr("Clear Layer"),
                    tr("Do you want to remove all objects from the layer '%1'? This action cannot be undone.").arg(layer->name()),
                    QMessageBox::StandardButtons(QMessageBox::Yes | QMessageBox::No));

            if (result == QMessageBox::Yes) {
                m_layerManager->removeLayerObjects(layer);
                m_historyManager->clearUndoHistory();
            }
        }
    }
}

void MainWindow::deleteActiveObject()
{
    if (m_selectedObject == nullptr) return;

    int result = QMessageBox::question(
            this,
            tr("Delete Object"),
            tr("Do you want to delete the object '%1'?").arg(m_selectedObject->label()),
            QMessageBox::StandardButtons(QMessageBox::Yes | QMessageBox::No));

    if (result == QMessageBox::Yes) {
        m_layerManager->removeLayerObject(
                m_layerManager->activeLayer(),
                m_selectedObject);
        createUndoDeleteObject(
                tr("Delete %1").arg(m_selectedObject->label()),
                m_layerManager->activeLayer(),
                m_selectedObject);

        // append to the delete list to be sent to db on next sync
        // unless it doesn't have an id yet
        if (m_databaseMode && !m_selectedObject->id().toString().isEmpty()) {
            m_databaseObjectDeleteList.append(m_selectedObject);
        }

        m_selectedObject = nullptr;

    }
}

void MainWindow::onTvwMarkersClicked(const QModelIndex &index)
{
    if (!index.isValid()) return;

    //
    // set the selected layer to active
    //
    if (!index.parent().isValid()) {
        auto *layer = m_layerManager->layers().at(index.row());
        m_layerManager->setActiveLayer(layer);
    }
    //
    // set the selected object to active
    //
    else {
        auto *object = static_cast<SlippyMapLayerObject*>(index.internalPointer());
        if (object != nullptr)
            ui->slippyMap->setActiveObject(object);
    }
}

void MainWindow::renameActiveLayer()
{
    if (m_layerManager->activeLayer() == nullptr) return;

    bool ok;
    QString name = QInputDialog::getText(
            this,
            tr("Rename Layer"),
            tr("Enter a new name for '%1':").arg(m_layerManager->activeLayer()->name()),
            QLineEdit::Normal,
            m_layerManager->activeLayer()->name(),
            &ok);

    if (ok) {
        m_layerManager->activeLayer()->setName(name);
        m_layerManager->updateActiveLayer();
    }
}

void MainWindow::deleteActiveLayer()
{
    if (m_layerManager->activeLayer() == nullptr) return;
    auto *layer = m_layerManager->activeLayer();

    if (layer->isEditable()) {
        int result = QMessageBox::question(
                this,
                tr("Clear Layer"),
                tr("Do you want to delete the layer '%1'?").arg(layer->name()),
                QMessageBox::StandardButtons(QMessageBox::Yes | QMessageBox::No));

        if (result == QMessageBox::Yes) {
            m_layerManager->takeLayer(layer);
            createUndoDeleteLayer(
                    tr("Delete Layer"),
                    layer);
        }
    }
}

void MainWindow::onSlippyMapPolygonSelected(const QList<QPointF>& points)
{
    if (m_layerManager->activeLayer() == nullptr) return;

    QVector<QPointF> pointVector = QVector<QPointF>::fromList(points);
    auto *polygon = new SlippyMapLayerPolygon(pointVector);
    polygon->setLabel(tr("New Polygon"));
    polygon->setStrokeColor(m_strokeColorSelector->color());
    polygon->setStrokeWidth(m_strokeWidth->value());
    polygon->setFillColor(m_fillColorSelector->color());
    m_layerManager->addLayerObject(polygon);

    createUndoAddObject(
            tr("New Polygon"),
            m_layerManager->activeLayer(),
            polygon);
}

void MainWindow::startPolygonSelection()
{
    ui->slippyMap->setDrawMode(SlippyMapWidget::PolygonDrawing);
}

void MainWindow::onSlippyMapPathSelected(const QList<QPointF> &points)
{
    qDebug() << "Path points selected:" << points;
    if (m_layerManager->activeLayer() == nullptr) return;

    QVector<QPointF> pointVector = QVector<QPointF>::fromList(points);
    auto *path = new SlippyMapLayerPath(pointVector);
    path->setLabel(tr("New Path"));
    path->setLineColor(m_fillColorSelector->color());
    path->setStrokeColor(m_strokeColorSelector->color());
    path->setStrokeWidth(m_strokeWidth->value());
    path->setLineWidth(m_lineWidth->value());
    m_layerManager->addLayerObject(path);

    createUndoAddObject(
            tr("New Path"),
            m_layerManager->activeLayer(),
            path);
}

void MainWindow::onAnimationTimerTimeout()
{
    if (m_animationState == Forward)
        ui->slippyMap->nextFrame();
    else if (m_animationState == Reverse)
        ui->slippyMap->previousFrame();
}

void MainWindow::onSlippyMapLayerObjectUpdated(SlippyMapLayerObject *object)
{
    ui->slippyMap->update();
}

void MainWindow::on_actionDrawLine_triggered()
{
    ui->slippyMap->setDrawMode(SlippyMapWidget::PathDrawing);
}

void MainWindow::undo()
{
    if (m_historyManager->undoCount() > 0) {
        HistoryManager::HistoryEvent event = m_historyManager->undoEvent();

        switch (event.action) {
            case HistoryManager::AddObject: {
                SlippyMapLayer *layer = event.layer;
                SlippyMapLayerObject *object = event.original;
                m_layerManager->removeLayerObject(layer, object);

                // the user may have created, then saved, then
                // wanted to undo, so now it may have an id and need
                // to be deleted off the db
                if (m_databaseMode && !object->id().toString().isEmpty())
                    m_databaseObjectDeleteList.append(object);

                break;
            }
            case HistoryManager::DeleteObject: {
                SlippyMapLayer *layer = event.layer;
                SlippyMapLayerObject *object = event.original;
                m_layerManager->addLayerObject(layer, object);

                // user undid the delete, so we can take it off
                // of the delete list if it's still there
                if (m_databaseMode && m_databaseObjectDeleteList.contains(object)) {
                    m_databaseObjectDeleteList.removeOne(object);
                }
                break;
            }
            case HistoryManager::ModifyObject: {
                // the clone contains the original contents
                SlippyMapLayerObject *temp = event.original->clone();
                SlippyMapLayerObject *object = event.original;
                SlippyMapLayerObject *clone = event.copy;

                // we need to replace the original with the copy
                event.original->copy(event.copy);

                // now put the "new" contents into copy for redo later
                event.copy->copy(temp);
                delete temp;
                break;
            }
            case HistoryManager::AddLayer: {
                SlippyMapLayer *layer = event.layer;
                m_layerManager->takeLayer(layer);

                // user could have added a layer, saved and then
                // deleted it, so delete from server
                if (m_databaseMode && !layer->id().toString().isEmpty())
                    m_databaseLayerDeleteList.append(layer);
                break;
            }
            case HistoryManager::ModifyLayer: {
                break;
            }
            case HistoryManager::DeleteLayer: {
                SlippyMapLayer *layer = event.layer;
                m_layerManager->addLayer(layer);

                // remove it from delete list
                if (m_databaseMode && m_databaseLayerDeleteList.contains(layer))
                    m_databaseLayerDeleteList.removeOne(layer);

                break;
            }
        }

        setWorkspaceDirty(true);
    }

    ui->actionEdit_Redo->setEnabled(true);
    ui->actionEdit_Redo->setText(tr("Redo") + " " + m_historyManager->currentRedoDescription());
    ui->actionEdit_Undo->setEnabled(m_historyManager->undoCount() > 0);
    if (m_historyManager->undoCount() > 0)
        ui->actionEdit_Undo->setText(tr("Undo") + " " + m_historyManager->currentUndoDescription());
}

void MainWindow::redo()
{
    if (m_historyManager->redoCount() > 0) {
        HistoryManager::HistoryEvent event = m_historyManager->redoEvent();

        switch (event.action) {
            case HistoryManager::DeleteObject: {
                SlippyMapLayer *layer = event.layer;
                SlippyMapLayerObject *object = event.original;
                m_layerManager->removeLayerObject(layer, object);

                // put it back on the delete list (unless it
                // doesn't have an id yet
                if (m_databaseMode && !object->id().toString().isEmpty())
                    m_databaseObjectDeleteList.append(object);

                break;
            }
            case HistoryManager::AddObject: {
                SlippyMapLayer *layer = event.layer;
                SlippyMapLayerObject *object = event.original;
                m_layerManager->addLayerObject(layer, object);

                // remove it from the delete list
                if (m_databaseMode && m_databaseObjectDeleteList.contains(object))
                    m_databaseObjectDeleteList.removeOne(object);

                break;
            }
            case HistoryManager::ModifyObject: {
                // the clone contains the original contents
                SlippyMapLayerObject *temp = event.original->clone();
                SlippyMapLayerObject *object = event.original;
                SlippyMapLayerObject *clone = event.copy;

                // we need to replace the original with the copy
                event.original->copy(event.copy);

                // now put the "new" contents into copy for undo later
                event.copy->copy(temp);
                delete temp;
                break;
            }
            case HistoryManager::AddLayer: {
                SlippyMapLayer *layer = event.layer;
                m_layerManager->addLayer(layer);

                // remove it from delete list
                if (m_databaseMode && m_databaseLayerDeleteList.contains(layer))
                    m_databaseLayerDeleteList.removeOne(layer);

                break;
            }
            case HistoryManager::DeleteLayer: {
                SlippyMapLayer *layer = event.layer;
                m_layerManager->takeLayer(layer);

                // put it back on the delete list (unless it
                // doesn't have an id yet
                if (m_databaseMode && !layer->id().toString().isEmpty())
                    m_databaseLayerDeleteList.append(layer);

                break;
            }
        }

        setWorkspaceDirty(true);
    }

    ui->actionEdit_Redo->setEnabled(m_historyManager->redoCount() > 0);
    if (m_historyManager->redoCount() > 0)
        ui->actionEdit_Redo->setText(tr("Redo") + " " + m_historyManager->currentRedoDescription());
    ui->actionEdit_Undo->setEnabled(true);
    if (m_historyManager->undoCount() > 0)
        ui->actionEdit_Undo->setText(tr("Undo") + " " + m_historyManager->currentUndoDescription());
}

void MainWindow::createUndoAddObject(const QString &description, SlippyMapLayer *layer, SlippyMapLayerObject *object)
{
    HistoryManager::HistoryEvent event;
    event.action = HistoryManager::AddObject;
    event.layer = layer;
    event.original = object;
    event.copy = nullptr;
    event.description = description;
    m_historyManager->addEvent(event);
    ui->actionEdit_Undo->setEnabled(true);
    ui->actionEdit_Undo->setText(tr("Undo") + " " + description);
    setWorkspaceDirty(true);
}

void MainWindow::createUndoModifyObject(const QString &description, SlippyMapLayerObject *object)
{
    HistoryManager::HistoryEvent event;
    event.description = description;
    event.action = HistoryManager::ModifyObject;
    event.original = object;
    // create a copy of the original object to restore later
    // if requested by undo
    event.copy = m_selectedObjectCopy;
    m_historyManager->addEvent(event);
    m_selectedObjectCopy = nullptr;
    setWorkspaceDirty(true);
}

void MainWindow::createUndoDeleteObject(const QString &description, SlippyMapLayer *layer, SlippyMapLayerObject *object)
{
    HistoryManager::HistoryEvent event;
    event.description = description;
    event.action = HistoryManager::DeleteObject;
    event.layer = layer;
    event.original = object;
    m_historyManager->addEvent(event);
    setWorkspaceDirty(true);
}

void MainWindow::createUndoAddLayer(const QString &description, SlippyMapLayer *layer)
{
    HistoryManager::HistoryEvent event;
    event.description = description;
    event.action = HistoryManager::AddLayer;
    event.layer = layer;
    m_historyManager->addEvent(event);
    setWorkspaceDirty(true);
}

void MainWindow::createUndoDeleteLayer(const QString &description, SlippyMapLayer *layer)
{
    HistoryManager::HistoryEvent event;
    event.description = description;
    event.action = HistoryManager::DeleteLayer;
    event.layer = layer;
    m_historyManager->addEvent(event);
    setWorkspaceDirty(true);
}

void MainWindow::undoEventAdded(HistoryManager::HistoryEvent event)
{
    auto *hist = m_historyManager;
    ui->actionEdit_Redo->setEnabled(hist->redoCount() > 0);
    if (hist->redoCount() > 0)
        ui->actionEdit_Redo->setText(tr("Redo") + " " + hist->currentRedoDescription());
    ui->actionEdit_Undo->setEnabled(hist->undoCount() > 0);
    if (hist->undoCount() > 0)
        ui->actionEdit_Undo->setText(tr("Undo") + " " + hist->currentUndoDescription());
}

void MainWindow::redoHistoryCleared()
{
    ui->actionEdit_Redo->setEnabled(false);
    ui->actionEdit_Redo->setText(tr("Redo"));
}

void MainWindow::cutActiveObject()
{
    if (m_selectedObject == nullptr) return;

    // create clipboard entry
    m_clipBoard.type = Clipboard::Object;
    m_clipBoard.action = Clipboard::Cut;
    m_clipBoard.layer = m_layerManager->activeLayer();
    m_clipBoard.object = m_selectedObject;

    // activate paste menu entry
    ui->actionEdit_Paste->setEnabled(true);

    // create a delete object undo item
    createUndoDeleteObject(
            tr("Cut"),
            m_layerManager->activeLayer(),
            m_selectedObject);

    // remove the item from the layer
    m_layerManager->removeLayerObject(m_layerManager->activeLayer(), m_selectedObject);
    m_selectedObject = nullptr;
}

void MainWindow::copyActiveObject()
{
    if (m_selectedObject == nullptr) return;

    // check for and delete last copied item
    if (m_clipBoard.action == Clipboard::Copy || m_clipBoard.action == Clipboard::Cut) {
        // don't delete the item we are copying!
        if (m_selectedObject != m_clipBoard.object)
            delete m_clipBoard.object;
    }

    // create clipboard entry
    m_clipBoard.type = Clipboard::Object;
    m_clipBoard.action = Clipboard::Copy;
    m_clipBoard.layer = m_layerManager->activeLayer();
    m_clipBoard.object = m_selectedObject->clone();

    // activate paste menu entry
    ui->actionEdit_Paste->setEnabled(true);
}

void MainWindow::pasteObject()
{
    switch (m_clipBoard.type) {
        case Clipboard::Object: {
            auto *layer = m_clipBoard.layer;
            auto *object = m_clipBoard.object;

            // paste it onto the active layer, if there is one.
            // otherwise go to the layer it came from?
            if (m_layerManager->activeLayer() != nullptr)
                layer = m_layerManager->activeLayer();

            // clone the object because you don't want to paste
            // the same object more than once
            m_layerManager->addLayerObject(layer, object->clone());

            createUndoAddObject(
                    tr("Paste"),
                    layer,
                    object);
            break;
        }
        default:
            break;
    }
}

void MainWindow::connectToDatabase()
{
    DatabaseConnectionDialog dlg(this);
    int status = dlg.exec();

    if (status == QDialog::Accepted) {
        QString connectionName = dlg.connectionName();
        QString address = dlg.databaseAddress();
        int port = dlg.databasePort();
        QString username = dlg.databaseUsername();
        QString password = dlg.databasePassword();
        QString database = dlg.databaseName();

        auto *db = ExplorerApplication::databaseManager();
        db->setHostAddress(address);
        db->setHostPort(port);
        db->setUsername(username);
        db->setPassword(password);
        db->setDatabaseName(database);

        if (db->connectDatabase()) {
            m_databaseMode = true;
            m_statusBarStatusLabel->setText(tr("Connected to %1").arg(connectionName));
            closeWorkspace();
            loadViewportData();
        }
        else {
            QMessageBox::critical(
                    this,
                    tr("Connection Error"),
                    tr("Unable to connect to database server."));
        }
    }
}

void MainWindow::loadViewportData()
{
    if (!m_databaseMode) return;

    auto boundingBox = ui->slippyMap->boundingBoxLatLon();

    QSqlQuery layerQueryStr;
    layerQueryStr.prepare(QString(
            "SELECT\n"
            "\"id\", \"name\", \"description\", \"order\"\n"
            "FROM\n"
            "osmexplorer.layers;"));
    auto layerQuery = QSqlQuery(layerQueryStr);
    layerQuery.exec();

    if (layerQuery.lastError().type() != QSqlError::NoError) {
        qCritical() << "Query failed:" << layerQuery.lastError().text();
        return;
    }

    while (layerQuery.next()) {
        QString layerId = layerQuery.value(0).toString();
        QString layerName = layerQuery.value(1).toString();
        QString layerDesc = layerQuery.value(2).toString();
        int order = layerQuery.value(3).toInt();

        SlippyMapLayer *layer = nullptr;
        for (auto l: m_layerManager->layers()) {
            qDebug() << "Matching" << l->id().toString() << "with" << layerId;
            if (layerId.compare(l->id().toString()) == 0) {
                layer = l;
            }
        }

        if (layer == nullptr) {
            layer = new SlippyMapLayer();
            layer->setId(layerId);
            layer->setSynced(true); // so we get the first update
            m_layerManager->addLayer(layer);
        }

        //
        // don't overwrite local changes
        //
        if (layer->isSynced()) {
            layer->setName(layerName);
            layer->setDescription(layerDesc);
        }

        auto queryString = QString(
                "SELECT id, label, description, ST_AsEWKT(geom), type, to_json(data) FROM osmexplorer.objects "
                "WHERE layer_id = ? AND geom && ST_MakeEnvelope(?, ?, ?, ?, 4326)");

        QSqlQuery objectQuery;
        objectQuery.prepare(queryString);
        objectQuery.addBindValue(layer->id().toString());
        objectQuery.addBindValue(boundingBox.x());
        objectQuery.addBindValue(boundingBox.y());
        objectQuery.addBindValue(boundingBox.x() + boundingBox.width());
        objectQuery.addBindValue(boundingBox.y() + boundingBox.height());
        objectQuery.exec();

        while (objectQuery.next()) {
            QString wkt = objectQuery.value(3).toString().replace("SRID=4326;", "");
            QVariant id = objectQuery.value(0).toString();
            QString label = objectQuery.value(1).toString();
            QString description = objectQuery.value(2).toString();
            QString className = objectQuery.value(4).toString().append("*");
            QString data = objectQuery.value(5).toString();

            QJsonDocument document = QJsonDocument::fromJson(data.toUtf8());
            QJsonObject root = document.object();

            for (auto *object: layer->objects()) {
                if (object->id().compare(id) == 0) {
                    if (object->isSynced()) {
                        // update with new data
                        object->setLabel(label);
                        object->setDescription(description);
                        object->hydrateFromDatabase(root, wkt);
                    }
                    goto continue2;
                }
            }

            // get the type information from qt meta
            int typeId = QMetaType::type(className.toLocal8Bit());
            const QMetaObject *metaObject = QMetaType::metaObjectForType(typeId);

            // create a new object and cast to layer object
            QObject *o = metaObject->newInstance();
            auto *object = qobject_cast<SlippyMapLayerObject *>(o);

            object->setId(id);
            object->setLabel(label);
            object->setDescription(description);
            object->hydrateFromDatabase(root, wkt);

            m_layerManager->addLayerObject(layer, object);
continue2:;
        }

        ui->statusBar->showMessage(tr("Loaded layer %1 (%2 objects)")
            .arg(layer->name())
            .arg(layer->objects().count()), 3000);
    }
}

void MainWindow::saveWorkspaceToDatabase()
{
    if (!m_databaseMode) return;

    int result = QMessageBox::question(
        this,
        tr("Sync Work"),
        tr("Do you want to sync your changes to the database?"),
        QMessageBox::Yes | QMessageBox::No);

    if (result != QMessageBox::Yes) return;

    for (auto *layer: m_databaseLayerDeleteList) {
        qDebug() << "Deleting layer" << layer->id() << layer->name();

        QString queryString(
                "DELETE FROM "
                DATABASE_SCHEMA_NAME "." DATABASE_LAYERS_TABLE " "
                "WHERE \"id\" = ?;");

        QSqlQuery deleteQuery;
        deleteQuery.prepare(queryString);
        deleteQuery.addBindValue(layer->id());

        if (!deleteQuery.exec()) {
            qCritical() << "Delete error:" << deleteQuery.lastError().text();
            qCritical() << "Executed query:" << deleteQuery.lastQuery();
            QMessageBox::critical(
                    this,
                    tr("Database Error"),
                    tr("Unable to delete layer: %1").arg(deleteQuery.lastError().text()));
            return;
        }
    }

    for (auto *layer: m_layerManager->layers()) {
        if (layer->isEditable()) {
            if (!layer->isSynced()) {
                qDebug() << "Updating layer" << layer->id() << layer->name();

                if (layer->id().isNull()) {
                    layer->setId(QVariant(QUuid::createUuid().toString()));
                }

                QString layerInsertString(
                            "INSERT INTO "
                            DATABASE_SCHEMA_NAME "." DATABASE_LAYERS_TABLE " "
                            "(\"id\", \"created\", \"name\", \"description\", \"order\") "
                            "VALUES "
                            "(:id, CURRENT_TIMESTAMP, :name, :description, :order) "
                            "ON CONFLICT (id) DO UPDATE "
                            "SET "
                            "\"name\" = EXCLUDED.name, "
                            "\"description\" = EXCLUDED.description, "
                            "\"order\" = EXCLUDED.order, "
                            "\"updated\" = CURRENT_TIMESTAMP;");
                QSqlQuery layerInsertQuery;
                layerInsertQuery.prepare(layerInsertString);
                layerInsertQuery.bindValue(":id", layer->id());
                layerInsertQuery.bindValue(":name", layer->name());
                layerInsertQuery.bindValue(":description", layer->description());
                layerInsertQuery.bindValue(":order", layer->order());

                if (!layerInsertQuery.exec()) {
                    qCritical() << "Upsert error:" << layerInsertQuery.lastError().text();
                    qCritical() << "Executed query:" << layerInsertQuery.lastQuery();

                    QMessageBox::critical(
                        this,
                        tr("Database Error"),
                        tr("Unable to save layer: %1").arg(layerInsertQuery.lastError().text()));
                    return;
                }

                layer->setSynced(true);
            }

            //
            // objects that were deleted need to be deleted
            // on the server
            //
            for (auto *object: m_databaseObjectDeleteList) {
                qDebug() << "Deleting object" << object->id() << object->label();

                QString queryString(
                    "DELETE FROM "
                        DATABASE_SCHEMA_NAME "." DATABASE_OBJECTS_TABLE " "
                        "WHERE \"id\" = ?;");

                QSqlQuery deleteQuery;
                deleteQuery.prepare(queryString);
                deleteQuery.addBindValue(object->id());

                if (!deleteQuery.exec()) {
                    qCritical() << "Delete error:" << deleteQuery.lastError().text();
                    qCritical() << "Executed query:" << deleteQuery.lastQuery();
                    QMessageBox::critical(
                            this,
                            tr("Database Error"),
                            tr("Unable to delete object: %1").arg(deleteQuery.lastError().text()));
                    return;
                }

                // object will be deleted by history manager
                m_databaseObjectDeleteList.removeOne(object);
            }

            for (auto *object: layer->objects()) {
                if (!object->isSynced()) {
                    qDebug() << "Updating object" << object->id() << object->label();

                    QString queryString(
                            "INSERT INTO " DATABASE_SCHEMA_NAME "." DATABASE_OBJECTS_TABLE " "
                            "(\"id\", \"layer_id\", \"type\", \"created\", \"label\", \"description\", \"geom\", \"data\") "
                            "VALUES "
                            "(:id, :layer_id, :type, CURRENT_TIMESTAMP, :label, :description, :geom, :data) "
                            "ON CONFLICT (id) DO UPDATE "
                            "SET "
                            "layer_id = EXCLUDED.layer_id, "
                            "type = EXCLUDED.type, "
                            "updated = CURRENT_TIMESTAMP, "
                            "label = EXCLUDED.label, "
                            "description = EXCLUDED.description, "
                            "geom = EXCLUDED.geom, "
                            "data = EXCLUDED.data;");

                    if (object->id().toString().isEmpty()) {
                        object->setId(QUuid::createUuid().toString());
                    }

                    QString geom;
                    QJsonObject json;

                    object->saveToDatabase(json, geom);
                    geom.prepend("SRID=4326;");

                    qDebug() << "Geometry string:" << geom;

                    QJsonDocument data(json);

                    QSqlQuery updateQuery;
                    updateQuery.prepare(queryString);
                    updateQuery.bindValue(":id", object->id());
                    updateQuery.bindValue(":layer_id", layer->id());
                    updateQuery.bindValue(":type", QString(object->metaObject()->className()));
                    updateQuery.bindValue(":label", object->label());
                    updateQuery.bindValue(":description", object->description());
                    updateQuery.bindValue(":geom", geom);
                    updateQuery.bindValue(":data", QString::fromLatin1(data.toJson(QJsonDocument::Compact)));

                    if (!updateQuery.exec()) {
                        qCritical() << "Upsert error:" << updateQuery.lastError().text();
                        qCritical() << "Executed query:" << updateQuery.lastQuery();
                        QMessageBox::critical(
                            this,
                            tr("Database Error"),
                            tr("Unable to save object: %1").arg(updateQuery.lastError().text()));
                        return;
                    }

                    object->setSynced(true);
                }
            }
        }
    }

    setWorkspaceDirty(false);
}
