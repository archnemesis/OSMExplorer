#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QAction>
#include <QCompleter>
#include <QFile>
#include <QFileDialog>
#include <QGraphicsDropShadowEffect>
#include <QGuiApplication>
#include <QInputDialog>
#include <QJsonArray>
#include <QJsonDocument>
#include <QLabel>
#include <QListWidgetItem>
#include <QStringListModel>
#include <QLocalServer>
#include <QLocalSocket>
#include <QMenuBar>
#include <QMenu>
#include <QMessageBox>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QPalette>
#include <QPluginLoader>
#include <QProgressBar>
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
#include "Network/ServerConnectionDialog.h"
#include "Network/WorkspaceSelectionDialog.h"
#include "GeoCoding/RadarGeoCodingInterface.h"

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
#include <QToolButton>

#include "Application/PluginManager.h"
#include "Dialog/ObjectBrowserDialog.h"
#include "GeoCoding/GeoCodingListModel.h"
#include "Map/SlippyMapGpsMarker.h"
#include "Map/SlippyMapLayerObjectFilesPropertyPage.h"
#include "Map/SlippyMapLayerProxyModel.h"
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
    setWindowTitle(tr("Untitled.osm"));

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
    ui->sidebarSaveButton->setVisible(false);
    ui->sidebarResetButton->setVisible(false);
    m_defaultPalette = qApp->palette();

    /*
     * Text inputs for label and description that let you edit the marker label.
     */
    connect(ui->selectedObjectName,
        &QLineEdit::textEdited,
        [this](const QString& text) {
        if (!m_selectedObject.isNull()) {
            const auto& obj = m_selectedObject.toStrongRef();
            obj->setLabel(text);
        }
    });

    connect(ui->selectedObjectDescription, &QPlainTextEdit::textChanged, [this]() {
        if (!m_selectedObject.isNull()) {
            SlippyMapLayerObject::Ptr obj = m_selectedObject.toStrongRef();
            obj->setDescription(ui->selectedObjectDescription->toPlainText());
        }
    });

    /*
     * Enable showing forecast data as the map moves
     */
    // connect(ui->actionWeather_ShowWFOGrid,
    //     &QAction::toggled,
    //     [this](bool state){
    //         if (state) {
    //             m_weatherService->getWeatherStationList(QPointF(
    //                     ui->slippyMap->longitude(),
    //                     ui->slippyMap->latitude()));
    //         }
    //         else {
    //             // TODO: remove the weather markers
    //         }
    //     });

    m_historyManager = ExplorerApplication::historyManager();

    connect(m_historyManager,
            &HistoryManager::undoEventAdded,
            this,
            &MainWindow::undoEventAdded);

    connect(m_historyManager,
            &HistoryManager::redoHistoryCleared,
            this,
            &MainWindow::redoHistoryCleared);

    /*
     * TreeView signals and slots
     */
    connect(ui->tvwMarkers,
        &QTreeView::activated,
        this,
        &MainWindow::activateLayerAtIndex);

    connect(ui->tvwMarkers,
            &QTreeView::customContextMenuRequested,
            this,
            &MainWindow::onTvwMarkersContextMenuRequested);

    connect(ui->tvwMarkers,
            &QTreeView::clicked,
            this,
            &MainWindow::onTvwMarkersClicked);

    m_serverRequestProgress = new QProgressBar();
    m_serverRequestProgress->setVisible(false);

    m_statusBarPositionLabel = new QLabel();
    m_statusBarPositionLabel->setFrameStyle(QFrame::Sunken);
    m_statusBarStatusLabel = new QLabel();

    m_statusBarGpsStatusLabel = new QLabel();
    m_statusBarGpsStatusLabel->setFrameStyle(QFrame::Sunken);
    m_statusBarGpsStatusLabel->setText("GPS Position: 0.000 N 0.000 E");

    statusBar()->addPermanentWidget(m_statusBarStatusLabel, 1);
    statusBar()->addPermanentWidget(m_serverRequestProgress);
    statusBar()->addPermanentWidget(m_statusBarGpsStatusLabel);
    statusBar()->addPermanentWidget(m_statusBarPositionLabel);

    loadStartupSettings();
    setupMap();
    setupWeather();
    refreshSettings();
    setupMenuBar();
    setupToolBar();
    setupContextMenus();
    loadLayers();
    loadPluginLayers();
    startLocalServer();

    m_animationTimer = new QTimer();
    m_animationTimer->setInterval(1000);
    connect(m_animationTimer,
            &QTimer::timeout,
            this,
            &MainWindow::advanceLayerAnimationFrame);
    m_animationTimer->start();

    m_saveWindowSizeTimer = new QTimer();
    m_saveWindowSizeTimer->setSingleShot(true);
    m_saveWindowSizeTimer->setInterval(1000);
    connect(m_saveWindowSizeTimer,
            &QTimer::timeout,
            this,
            &MainWindow::saveWindowSize);

    for (ExplorerPluginInterface *plugin : ExplorerApplication::pluginManager()->getPlugins()) {
        QList<QDockWidget *> dockWidgets = plugin->dockWidgetList();
        for (QDockWidget *dockWidget : dockWidgets) {
            dockWidget->setParent(this);
            addDockWidget(Qt::LeftDockWidgetArea, dockWidget);
        }
    }

    QSettings settings;
    bool visibility = settings.value("dock/visibility", true).toBool();
    ui->dockWidget->setVisible(visibility);

    connect(ui->dockWidget,
            &OEDockWidget::closed,
            [this]() {
        QSettings().setValue("dock/visibility", false);
        m_actionTools_sidebar->setChecked(false);
    });

    loadMarkers();

    m_serverInterface = ExplorerApplication::serverInterface();
    m_serverNetworkManager = new QNetworkAccessManager(this);

    /*
     * Geo-coding
     */
    m_geoCodingInterface = new RadarGeoCodingInterface(this);
    connect(m_geoCodingInterface,
            &GeoCodingInterface::locationsFound,
            this,
            &MainWindow::on_geoCodingInterface_locationFound);

    setWorkspaceDirty(false);
    disableDrawing();
    loadWorkspaces();

    //startServerLogin();
}

MainWindow::~MainWindow()
{
    ui->tvwMarkers->setModel(nullptr);
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

            m_menuLayer_tileLayersMenu->addAction(action);
        }
    }


    for (const auto& layer : pluginManager->getLayers()) {
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

    m_weatherLayer = SlippyMapLayer::Ptr::create();
    m_weatherLayer->setName(tr("Weather"));
    m_weatherLayer->setVisible(true);
    m_weatherLayer->setEditable(false);
    m_weatherLayer->setColor(QColor(0x00, 0x00, 0xCC));
    m_layerManager->addLayer(m_weatherLayer);
}

void MainWindow::setupUi()
{

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

    m_deleteLayerAction = new QAction();
    m_deleteLayerAction->setText(tr("Delete"));
    m_treeViewMenu->addAction(m_deleteLayerAction);
    connect(m_deleteLayerAction,
            &QAction::triggered,
            this,
            &MainWindow::deleteSelectedLayer);

    m_clearLayerAction = new QAction();
    m_clearLayerAction->setText(tr("Clear Layer"));
    m_treeViewMenu->addAction(m_clearLayerAction);
    connect(m_clearLayerAction,
            &QAction::triggered,
            this,
            &MainWindow::clearSelectedLayer);

    m_treeViewMenu->addSeparator();

    m_layerVisibilityAction = new QAction();
    m_layerVisibilityAction->setText(tr("Visible"));
    m_layerVisibilityAction->setCheckable(true);
    m_treeViewMenu->addAction(m_layerVisibilityAction);
    connect(m_layerVisibilityAction, &QAction::toggled, [this](bool checked) {
        QModelIndex index = ui->tvwMarkers->currentIndex();
        if (!index.isValid()) return;
        Q_ASSERT(index.row() < m_layerManager->layers().count());
        SlippyMapLayer::Ptr layer = m_layerManager->layers().at(index.row());
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
                SlippyMapLayer::Ptr layer = m_layerManager->layers().at(index.row());
                layer->setEditable(!checked);
            });

    // causing multiple layers to be shown/hidden with checkbox in tvw
//    connect(ui->tvwMarkers, &QTreeView::clicked, [this](const QModelIndex& index) {
//        if (!index.isValid() || index.parent().isValid()) return;
//        Q_ASSERT(index.row() < m_layerManager->layers().count());
//        SlippyMapLayer::Ptr layer = m_layerManager->layers().at(index.row());
//        m_layerVisibilityAction->setChecked(layer->isVisible());
//        m_markerLockedAction->setChecked(!layer->isEditable());
//    });

    connect(ui->tvwMarkers, &QTreeView::activated, [this](const QModelIndex& index) {
        if (!index.isValid() || index.parent().isValid()) return;
        Q_ASSERT(index.row() < m_layerManager->layers().count());
        SlippyMapLayer::Ptr layer = m_layerManager->layers().at(index.row());
        m_layerVisibilityAction->setChecked(layer->isVisible());
        m_markerLockedAction->setChecked(!layer->isEditable());
    });

    /*
     * SlippyMapWidget context menu
     */

    m_coordAction = new QAction();
    m_coordAction->setEnabled(false);

    m_addMarkerAction = new QAction();
    m_addMarkerAction->setText(tr("Add Marker"));
    m_addMarkerAction->setIcon(QIcon(":/icons/toolbar/marker-add.svg"));
    connect(m_addMarkerAction,
            &QAction::triggered,
            this,
            &MainWindow::createMarkerAtContextMenuPosition);

    m_deleteObjectAction = new QAction();
    m_deleteObjectAction->setText(tr("Delete..."));
    connect(m_deleteObjectAction,
            &QAction::triggered,
            this,
            &MainWindow::deleteActiveObject);

    m_objectPropertiesAction = new QAction();
    m_objectPropertiesAction->setText(tr("Properties..."));
    connect(m_objectPropertiesAction,
            &QAction::triggered,
            this,
            &MainWindow::showActiveObjectPropertyPage);

    m_centerMapAction = new QAction();
    m_centerMapAction->setText(tr("Center Here"));
    connect(m_centerMapAction,
            &QAction::triggered,
            this,
            &MainWindow::centerMapOnContextMenuPosition);

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
    m_contextMenu->addAction(m_actionEdit_cut);
    m_contextMenu->addAction(m_actionEdit_copy);
    m_contextMenu->addAction(m_actionEdit_paste);
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

    m_layerManagerProxy = new SlippyMapLayerProxyModel(this);
    m_layerManagerProxy->setSourceModel(m_layerManager);

    ui->tvwMarkers->setModel(m_layerManagerProxy);
    ui->tvwMarkers->setContextMenuPolicy(Qt::CustomContextMenu);

    /*
     * GPS Marker Layer
     */
    m_gpsMarkerLayer = SlippyMapLayer::Ptr::create();
    m_gpsMarkerLayer->setName(tr("GPS"));
    m_gpsMarkerLayer->setEditable(false);
    m_gpsMarkerLayer->setColor(QColor("#FF00CC00"));
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
    connect(ui->slippyMap, &SlippyMapWidget::pointSelected, this, &MainWindow::onSlippyMapPointSelected);
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

    bool crosshairs = QSettings().value("map/crosshairs", false).toBool();
    ui->slippyMap->setCrosshairsEnabled(crosshairs);

    ui->slippyMap->setFocus(Qt::OtherFocusReason);

    m_searchMarkerLayer = SlippyMapLayer::Ptr::create(m_layerManager);
    m_searchMarkerLayer->setName(tr("Search Markers"));
    m_searchMarkerLayer->setEditable(false);
    m_searchMarkerLayer->setShowInLayerView(false);
    m_layerManager->addLayer(m_searchMarkerLayer);

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

    m_menuLayer_tileLayersMenu->clear();
    m_layers.clear();

    int layerCount = settings.beginReadArray("layers");
    for (int i = 0; i < layerCount; i++) {
        settings.setArrayIndex(i);
        QString name = settings.value("name").toString();
        QString description = settings.value("description").toString();
        QString tileUrl = settings.value("tileServer").toString();
        int zOrder = settings.value("zOrder").toInt();
        bool visible = settings.value("visible", true).toBool();

        auto *layer = new SlippyMapWidgetLayer(tileUrl);
        layer->setName(name);
        layer->setDescription(description);
        layer->setZOrder(zOrder);
        layer->setVisible(visible);
        layer->setBaseLayer(true);
        layer->setCacheDuration(86400);
        ui->slippyMap->addLayer(layer);
        m_layers.append(layer);

        QAction *layerShowHide = new QAction();
        layerShowHide->setCheckable(true);
        layerShowHide->setChecked(visible);
        layerShowHide->setText(name);
        connect(layerShowHide, &QAction::triggered, [=]() {
            layer->setVisible(layerShowHide->isChecked());
            ui->slippyMap->update();
            saveTileLayerSettings();
        });
        m_menuLayer_tileLayersMenu->addAction(layerShowHide);
    }
    settings.endArray();

    if (layerCount == 0) {
        QMessageBox::information(
                this,
                tr("Layers"),
                tr("There are no layers configured. To get started, add one or more layers in Settings."),
                QMessageBox::Ok);
        showSettingsDialog();
    }
}

void MainWindow::saveTileLayerSettings()
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

void MainWindow::setupToolBar()
{
    /**
     * Lat/Lon inputs in the toolbar
     */
    m_toolBarLatitudeInput = new QLineEdit();
    m_toolBarLatitudeInput->setPlaceholderText(tr("00.000000 N"));
    m_toolBarLatitudeInput->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Minimum);

    m_toolBarLongitudeInput = new QLineEdit();
    m_toolBarLongitudeInput->setPlaceholderText(tr("000.000000 E"));
    m_toolBarLongitudeInput->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Minimum);

    m_toolBarGeoCodingInput = new QLineEdit();
    m_toolBarGeoCodingInput->setPlaceholderText(tr("Address or place name"));
    m_toolBarGeoCodingInput->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Minimum);
    m_toolBarGeoCodingInput->setMinimumWidth(300);

    m_toolBarLatLonButton = new QPushButton();
    m_toolBarLatLonButton->setText(tr("Go"));
    m_toolBarLatLonButton->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Minimum);
    connect(m_toolBarLatLonButton,
        &QToolButton::clicked,
        this,
        &MainWindow::getGeoCodedLocation);

    /**
     * Color and stroke width selectors for drawing
     */
    m_strokeColorSelector = new ColorSelector();
    m_strokeColorSelector->setObjectName("strokeColorSelector");
    m_strokeColorSelector->setFixedWidth(60);
    m_strokeColorSelector->setColor(qApp->palette().text().color());
    m_strokeColorSelector->setToolTip(tr("Stroke color"));
    connect(m_strokeColorSelector,
            &ColorSelector::colorSelected,
            [this](const QColor& color) {
                ui->slippyMap->setDrawingStrokeColor(color.lighter());
            });

    m_fillColorSelector = new ColorSelector();
    m_fillColorSelector->setObjectName("fillColorSelector");
    m_fillColorSelector->setFixedWidth(60);
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
    m_strokeWidth->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Minimum);
    m_strokeWidth->setToolTip(tr("Stroke width"));
    connect(m_strokeWidth,
            QOverload<int>::of(&QSpinBox::valueChanged),
            [this](int value) {
                ui->slippyMap->setDrawingStrokeWidth(value);
            });

    m_lineWidth = new QSpinBox();
    m_lineWidth->setMinimum(1);
    m_lineWidth->setValue(10);
    m_lineWidth->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Minimum);
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

    m_drawingActionGroup = new QActionGroup(this);
    m_drawingActionGroup->setExclusive(true);
    m_drawingActionGroup->addAction(m_actionObject_addMarker);
    m_drawingActionGroup->addAction(m_actionObject_addLine);
    m_drawingActionGroup->addAction(m_actionObject_addRectangle);
    m_drawingActionGroup->addAction(m_actionObject_addEllipse);
    m_drawingActionGroup->addAction(m_actionObject_addPolygon);

    ui->toolBar->addAction(m_actionFile_newWorkspace);
    ui->toolBar->addAction(m_actionFile_openWorkspace);
    ui->toolBar->addAction(m_actionFile_saveWorkspace);
    ui->toolBar->addSeparator();
    ui->toolBar->addActions(m_drawingActionGroup->actions());
    ui->toolBar->addSeparator();
    ui->toolBar->addWidget(m_strokeColorSelector);
    ui->toolBar->addWidget(m_fillColorSelector);
    ui->toolBar->addWidget(m_lineWidth);
    ui->toolBar->addWidget(m_strokeWidth);
    ui->toolBar->addSeparator();
    // ui->toolBar->addWidget(m_toolBarLatitudeInput);
    // ui->toolBar->addWidget(m_toolBarLongitudeInput);
    ui->toolBar->addWidget(m_toolBarGeoCodingInput);
    ui->toolBar->addWidget(m_toolBarLatLonButton);
    ui->toolBar->addSeparator();
    ui->toolBar->addAction(m_actionTools_connectGps);
    ui->toolBar->addAction(m_actionTools_disconnectGps);
    ui->toolBar->addAction(m_actionTools_centerGps);
    ui->toolBar->addAction(m_actionTools_recordGpsPosition);
    ui->toolBar->addSeparator();
    ui->toolBar->addAction(m_actionLayer_previousFrame);
    ui->toolBar->addAction(m_actionLayer_play);
    ui->toolBar->addAction(m_actionLayer_nextFrame);
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

void MainWindow::setupMenuBar()
{
    /* ----- File Menu ----- */

    m_actionFile_newWorkspace = new QAction(tr("New Workspace..."));
    m_actionFile_newWorkspace->setIcon(QIcon(":/icons/toolbar/new.svg"));
    connect(m_actionFile_newWorkspace,
            &QAction::triggered,
            this,
            &MainWindow::newWorkspace);

    m_actionFile_openWorkspace = new QAction(tr("Open Workspace..."));
    m_actionFile_openWorkspace->setIcon(QIcon(":/icons/toolbar/open.svg"));
    connect(m_actionFile_openWorkspace,
            &QAction::triggered,
            this,
            &MainWindow::openWorkspace);

    m_actionFile_saveWorkspace = new QAction(tr("Save Workspace"));
    m_actionFile_saveWorkspace->setIcon(QIcon(":/icons/toolbar/save.svg"));
    connect(m_actionFile_saveWorkspace,
        &QAction::triggered,
        this,
            QOverload<>::of(&MainWindow::saveWorkspace)
            );

    m_actionFile_closeWorkspace = new QAction(tr("Close Workspace"));
    connect(m_actionFile_closeWorkspace,
            &QAction::triggered,
            this,
            &MainWindow::closeWorkspace);

    m_actionFile_import = new QAction(tr("Import..."));
    connect(m_actionFile_import,
        &QAction::triggered,
        [this]() {
            QMessageBox::information(
                this,
                tr("Not Implemented"),
                tr("This feature has not been implemented."));
        });


    m_actionFile_export = new QAction(tr("Export..."));
    connect(m_actionFile_export,
        &QAction::triggered,
        [this]() {
            QMessageBox::information(
                this,
                tr("Not Implemented"),
                tr("This feature has not been implemented."));
        });

    m_actionFile_quit = new QAction(tr("&Quit"));
    connect(m_actionFile_quit,
        &QAction::triggered,
        this,
        &MainWindow::close);

    m_menuFile = new QMenu(tr("&File"));
    m_menuFile->addAction(m_actionFile_newWorkspace);
    m_menuFile->addAction(m_actionFile_openWorkspace);
    m_menuFile->addAction(m_actionFile_closeWorkspace);
    m_menuFile->addSeparator();
    m_menuFile->addAction(m_actionFile_saveWorkspace);
    m_menuFile->addSeparator();
    m_menuFile->addAction(m_actionFile_import);
    m_menuFile->addAction(m_actionFile_export);
    m_menuFile->addSeparator();
    m_menuFile->addAction(m_actionFile_quit);

    menuBar()->addMenu(m_menuFile);

    /* ----- Edit Menu ----- */

    m_actionEdit_cut = new QAction(tr("Cut"));
    m_actionEdit_cut->setEnabled(false);
    m_actionEdit_cut->setIcon(QIcon(":/icons/toolbar/cut.svg"));
    connect(m_actionEdit_cut,
            &QAction::triggered,
            this,
            &MainWindow::cutActiveObject);

    m_actionEdit_copy = new QAction(tr("Copy"));
    m_actionEdit_copy->setEnabled(false);
    m_actionEdit_copy->setIcon(QIcon(":/icons/toolbar/copy.svg"));
    connect(m_actionEdit_copy,
            &QAction::triggered,
            this,
            &MainWindow::copyActiveObject);

    m_actionEdit_paste = new QAction(tr("Paste"));
    m_actionEdit_paste->setEnabled(false);
    m_actionEdit_paste->setIcon(QIcon(":/icons/toolbar/paste.svg"));
    connect(m_actionEdit_paste,
            &QAction::triggered,
            this,
            &MainWindow::pasteObject);

    m_actionEdit_undo = new QAction(tr("Undo"));
    m_actionEdit_undo->setEnabled(false);
    m_actionEdit_undo->setIcon(QIcon(":/icons/toolbar/undo.svg"));
    connect(m_actionEdit_undo,
            &QAction::triggered,
            this,
            &MainWindow::undo);

    m_actionEdit_redo = new QAction(tr("Redo"));
    m_actionEdit_redo->setEnabled(false);
    m_actionEdit_redo->setIcon(QIcon(":/icons/toolbar/redo.svg"));
    connect(m_actionEdit_redo,
            &QAction::triggered,
            this,
            &MainWindow::redo);

    m_actionEdit_delete = new QAction(tr("Delete"));
    m_actionEdit_delete->setEnabled(false);
    connect(m_actionEdit_delete,
            &QAction::triggered,
            this,
            &MainWindow::deleteActiveObject);

    m_actionEdit_properties = new QAction(tr("Properties..."));
    m_actionEdit_properties->setEnabled(false);
    connect(m_actionEdit_properties,
            &QAction::triggered,
            this,
            &MainWindow::showActiveObjectPropertyPage);

    m_menuEdit = new QMenu(tr("&Edit"));
    m_menuEdit->addAction(m_actionEdit_undo);
    m_menuEdit->addAction(m_actionEdit_redo);
    m_menuEdit->addSeparator();
    m_menuEdit->addAction(m_actionEdit_cut);
    m_menuEdit->addAction(m_actionEdit_copy);
    m_menuEdit->addAction(m_actionEdit_paste);
    m_menuEdit->addAction(m_actionEdit_delete);
    m_menuEdit->addSeparator();
    m_menuEdit->addAction(m_actionEdit_properties);

    menuBar()->addMenu(m_menuEdit);

    /* ----- Layer Menu ----- */

    m_actionLayer_new = new QAction(tr("New..."));
    connect(m_actionLayer_new,
            &QAction::triggered,
            this,
            &MainWindow::createNewLayer);

    m_actionLayer_rename = new QAction(tr("Rename..."));
    connect(m_actionLayer_rename,
            &QAction::triggered,
            this,
            &MainWindow::renameActiveLayer);

    m_actionLayer_delete = new QAction(tr("Delete"));
    connect(m_actionLayer_delete,
        &QAction::triggered,
        this,
        &MainWindow::deleteSelectedLayer);

    m_actionLayer_sortAscending = new QAction(tr("Sort Ascending"));
    connect(m_actionLayer_sortAscending,
            &QAction::triggered,
            [this]() {
                m_layerManager->sort(SlippyMapLayerManager::LayerSortName, Qt::AscendingOrder);
            });

    m_actionLayer_sortDescending = new QAction(tr("Sort Descending"));
    connect(m_actionLayer_sortDescending,
            &QAction::triggered,
            [this]() {
                m_layerManager->sort(SlippyMapLayerManager::LayerSortName, Qt::DescendingOrder);
            });

    m_actionLayer_nextFrame = new QAction(tr("Previous Frame"));
    connect(m_actionLayer_nextFrame,
            &QAction::triggered,
            [this]() {
        ui->slippyMap->nextFrame();
    });

    m_actionLayer_previousFrame = new QAction(tr("Auto"));
    connect(m_actionLayer_previousFrame,
            &QAction::triggered,
            [this]() {
        ui->slippyMap->previousFrame();
    });

    m_actionLayer_play = new QAction(tr("Next Frame"));
    m_actionLayer_play->setCheckable(true);
    connect(m_actionLayer_play,
            &QAction::toggled,
            [this](bool state) {
        if (state)
            m_animationState = Forward;
        else
            m_animationState = Paused;
    });

    auto *sortActionGroup = new QActionGroup(this);
    sortActionGroup->addAction(m_actionLayer_sortAscending);
    sortActionGroup->addAction(m_actionLayer_sortDescending);
    sortActionGroup->setExclusive(true);

    m_menuLayer_tileLayersMenu = new QMenu(tr("Tile Layers"));

    m_menuLayer = new QMenu(tr("&Layer"));
    m_menuLayer->addAction(m_actionLayer_new);
    m_menuLayer->addAction(m_actionLayer_rename);
    m_menuLayer->addAction(m_actionLayer_delete);
    m_menuLayer->addSeparator();
    m_menuLayer->addActions(sortActionGroup->actions());
    m_menuLayer->addSeparator();
    m_menuLayer->addMenu(m_menuLayer_tileLayersMenu);

    menuBar()->addMenu(m_menuLayer);

    /* ----- Object Menu ----- */

    m_actionObject_addMarker = new QAction(tr("New Marker"));
    m_actionObject_addMarker->setCheckable(true);
    m_actionObject_addMarker->setIcon(QIcon(":/icons/toolbar/marker-add.svg"));
    connect(m_actionObject_addMarker,
        &QAction::triggered,
        this,
        &MainWindow::createMarkerAtCurrentPosition);

    m_actionObject_addLine = new QAction(tr("New Line"));
    m_actionObject_addLine->setCheckable(true);
    m_actionObject_addLine->setIcon(QIcon(":/icons/toolbar/path-add.svg"));
    connect(m_actionObject_addLine,
        &QAction::triggered,
        this,
        &MainWindow::setDrawLineMode);

    m_actionObject_addRectangle = new QAction(tr("New Rectangle"));
    m_actionObject_addRectangle->setCheckable(true);
    m_actionObject_addRectangle->setIcon(QIcon(":/icons/toolbar/rect-add.svg"));
    connect(m_actionObject_addRectangle,
        &QAction::triggered,
        this,
        &MainWindow::setDrawRectMode);

    m_actionObject_addEllipse = new QAction(tr("New Ellipse"));
    m_actionObject_addEllipse->setCheckable(true);
    m_actionObject_addEllipse->setIcon(QIcon(":/icons/toolbar/ellipse-add-extent.svg"));
    connect(m_actionObject_addEllipse,
        &QAction::triggered,
        this,
        &MainWindow::setDrawEllipseMode);

    m_actionObject_addPolygon = new QAction(tr("New Polygon"));
    m_actionObject_addPolygon->setCheckable(true);
    m_actionObject_addPolygon->setIcon(QIcon(":/icons/toolbar/polygon-add.svg"));
    connect(m_actionObject_addPolygon,
        &QAction::triggered,
        this,
        &MainWindow::setDrawPolygonMode);

    m_actionObject_properties = new QAction(tr("Properties..."));
    connect(m_actionEdit_properties,
        &QAction::triggered,
        this,
        &MainWindow::showActiveObjectPropertyPage);

    m_actionObject_delete = new QAction(tr("Delete"));
    connect(m_actionObject_delete,
        &QAction::triggered,
        this,
        &MainWindow::deleteActiveObject);

    m_actionObject_browser = new QAction(tr("Browser..."));
    connect(m_actionObject_browser,
        &QAction::triggered,
        this,
        &MainWindow::showObjectBrowserDialog);

    m_actionObject_importGpx = new QAction(tr("GPX"));
    connect(m_actionObject_importGpx,
        &QAction::triggered,
        this,
        &MainWindow::importGpx);

    m_actionObject_importCsv = new QAction(tr("CSV"));
    connect(m_actionObject_importCsv,
        &QAction::triggered,
        [this]() {
            QMessageBox::information(
                this,
                tr("Not Implemented"),
                tr("This feature has not been implemented."));
        });

    m_actionObject_importSvg = new QAction(tr("SVG"));
    connect(m_actionObject_importSvg,
        &QAction::triggered,
        [this]() {
            QMessageBox::information(
                this,
                tr("Not Implemented"),
                tr("This feature has not been implemented."));
    });

    m_actionObject_importGeoJson = new QAction(tr("GeoJSON"));
    connect(m_actionObject_importGeoJson,
        &QAction::triggered,
        [this]() {
            QMessageBox::information(
                this,
                tr("Not Implemented"),
                tr("This feature has not been implemented."));
        });

    m_menuObject_importMenu = new QMenu(tr("&Import"));
    m_menuObject_importMenu->addAction(m_actionObject_importGpx);
    m_menuObject_importMenu->addAction(m_actionObject_importCsv);
    m_menuObject_importMenu->addAction(m_actionObject_importSvg);
    m_menuObject_importMenu->addAction(m_actionObject_importGeoJson);

    m_menuObject = new QMenu(tr("&Object"));
    m_menuObject->addAction(m_actionObject_addMarker);
    m_menuObject->addAction(m_actionObject_addLine);
    m_menuObject->addAction(m_actionObject_addRectangle);
    m_menuObject->addAction(m_actionObject_addPolygon);
    m_menuObject->addMenu(m_menuObject_importMenu);
    m_menuObject->addSeparator();
    m_menuObject->addAction(m_actionObject_properties);
    m_menuObject->addAction(m_actionObject_delete);
    m_menuObject->addAction(m_actionObject_browser);

    menuBar()->addMenu(m_menuObject);

    /* ----- Tools Menu ----- */

    bool crosshairs = QSettings().value("map/crosshairs", false).toBool();

    m_actionTools_crosshairs = new QAction(tr("Crosshairs"));
    m_actionTools_crosshairs->setCheckable(true);
    m_actionTools_crosshairs->setChecked(crosshairs);
    connect(m_actionTools_crosshairs,
            &QAction::toggled,
            [this](bool state) {
                QSettings().setValue("map/crosshairs", state);
                ui->slippyMap->setCrosshairsEnabled(state);
            });

    m_actionTools_sidebar = new QAction(tr("Sidebar"));
    m_actionTools_sidebar->setCheckable(true);
    connect(m_actionTools_sidebar,
            &QAction::toggled,
            [this](bool state) {
        QSettings().setValue("dock/visibility", state);
        ui->dockWidget->setVisible(state);
    });

    m_actionTools_settings = new QAction(tr("Settings"));
    connect(m_actionTools_settings,
        &QAction::triggered,
        this,
        &MainWindow::showSettingsDialog);

    m_actionTools_connectGps = new QAction(tr("Connect GPS..."));
    m_actionTools_connectGps->setIcon(QIcon(":/icons/toolbar/gps-add.svg"));
    connect(m_actionTools_connectGps,
        &QAction::triggered,
        this,
        &MainWindow::showAddGpsSourceDialog);

    m_actionTools_disconnectGps = new QAction(tr("Disconnect GPS"));
    m_actionTools_disconnectGps->setEnabled(false);
    m_actionTools_disconnectGps->setIcon(QIcon(":/icons/toolbar/gps-disconnect.svg"));
    connect(m_actionTools_disconnectGps,
            &QAction::triggered,
            [this]() {
                QMessageBox::information(
                    this,
                    tr("Not Implemented"),
                    tr("This feature has not been implemented."));
            });

    m_actionTools_viewGpsLog = new QAction(tr("View GPS Log"));
    m_actionTools_viewGpsLog->setEnabled(false);
    connect(m_actionTools_viewGpsLog,
            &QAction::triggered,
            this,
            &MainWindow::showGpsLogDialog);

    m_actionTools_centerGps = new QAction(tr("Follow GPS"));
    m_actionTools_centerGps->setCheckable(true);
    m_actionTools_centerGps->setIcon(QIcon(":/icons/toolbar/gps-recenter.svg"));
    connect(m_actionTools_centerGps,
        &QAction::triggered,
        [this]() {
            QSettings().setValue("gps/followGps", true);
        });
    m_actionTools_centerGps->setChecked(
        QSettings().value("gps/followGps", false).toBool());

    m_actionTools_recordGpsPosition = new QAction(tr("Record Track"));
    m_actionTools_recordGpsPosition->setCheckable(true);
    m_actionTools_recordGpsPosition->setIcon(QIcon(":/icons/toolbar/record.svg"));

    m_menuTools = new QMenu(tr("&Tools"));
    m_menuTools->addAction(m_actionTools_sidebar);
    m_menuTools->addAction(m_actionTools_crosshairs);
    m_menuTools->addSeparator();
    m_menuTools->addAction(m_actionTools_connectGps);
    m_menuTools->addAction(m_actionTools_disconnectGps);
    m_menuTools->addAction(m_actionTools_viewGpsLog);
    m_menuTools->addAction(m_actionTools_centerGps);
    m_menuTools->addAction(m_actionTools_recordGpsPosition);
    m_menuTools->addSeparator();
    m_menuTools->addAction(m_actionTools_settings);

    menuBar()->addMenu(m_menuTools);

    /* ----- Plugin Menus ----- */

    for (auto *plugin : m_plugins) {
        for (auto *menu : plugin->mainMenuList())
            menuBar()->addMenu(menu);
    }

    /* ----- Help Menu ----- */

    m_actionHelp_about = new QAction(tr("&About"));
    m_menuHelp = new QMenu(tr("&Help"));
    m_menuHelp->addAction(m_actionHelp_about);

    menuBar()->addMenu(m_menuHelp);
}

void MainWindow::onSlippyMapZoomLevelChanged(int zoom)
{
    (void)zoom;
}

void MainWindow::onSlippyMapTileRequestStarted()
{
    m_requestCount++;

    int max = m_serverRequestProgress->maximum();

    m_serverRequestProgress->setMaximum(qMax(m_requestCount, max));
    m_serverRequestProgress->setValue(max - m_requestCount);

    if (m_requestCount > 0) {
        m_statusBarStatusLabel->setText(QString("Loading %1 tiles...").arg(m_requestCount));
        m_serverRequestProgress->setVisible(true);
    }
    else {
        m_statusBarStatusLabel->setText(tr("Ready"));
        m_serverRequestProgress->setVisible(false);
        m_serverRequestProgress->setMaximum(0);
    }
}

void MainWindow::onSlippyMapTileRequestFinished()
{
    m_requestCount--;
    int max = m_serverRequestProgress->maximum();
    m_serverRequestProgress->setValue(max - m_requestCount);

    if (m_requestCount > 0) {
        m_statusBarStatusLabel->setText(QString("Loading %1 tiles...").arg(m_requestCount));
        m_serverRequestProgress->setVisible(true);
    }
    else {
        m_statusBarStatusLabel->setText("");
        m_serverRequestProgress->setVisible(false);
        m_serverRequestProgress->setMaximum(0);
    }
}

void MainWindow::onSlippyMapTileRequestPending(int tiles)
{
    m_serverRequestProgress->setMaximum(tiles);
    m_serverRequestProgress->setValue(0);
    m_serverRequestProgress->setVisible(true);
}

void MainWindow::onSlippyMapCursorPositionChanged(double latitude, double longitude)
{
    QString tpl("%1 %2");
    QString lon;
    QString lat;

    if (latitude > 0) {
        lat = tpl.arg(fabs(latitude), 8, 'f', 7, '0').arg("N");
    }
    else {
        lat = tpl.arg(fabs(latitude), 8, 'f', 7, '0').arg("S");
    }

    if (longitude < 0) {
        lon = tpl.arg(fabs(longitude), 8, 'f', 7, '0').arg("W");
    }
    else {
        lon = tpl.arg(fabs(longitude), 8, 'f', 7, '0').arg("E");
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
    for (SlippyMapLayer::Ptr layer : m_layerManager->layers()) {
        for (const auto& object: layer->objects()) {
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

    auto poly = SlippyMapLayerPolygon::Ptr::create(points);
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

    saveObject(poly);
}

void MainWindow::onSlippyMapDrawModeChanged(SlippyMapWidget::DrawMode mode)
{
    switch (mode) {
    case SlippyMapWidget::NoDrawing:
        m_actionObject_addMarker->setChecked(false);
        m_actionObject_addLine->setChecked(false);
        m_actionObject_addRectangle->setChecked(false);
        m_actionObject_addPolygon->setChecked(false);
        m_actionObject_addEllipse->setChecked(false);
        statusBar()->clearMessage();
        break;
    case SlippyMapWidget::MarkerDrawing:
        //m_actionObject_addMarker->setChecked(true);
        break;
    case SlippyMapWidget::PathDrawing:
        m_actionObject_addLine->setChecked(true);
        statusBar()->showMessage(tr("Click to set path points. Double-click to finish drawing. Esc to cancel."));
        break;
    case SlippyMapWidget::RectDrawing:
        m_actionObject_addRectangle->setChecked(true);
        statusBar()->showMessage(tr("Drag to draw rectangle. Esc to cancel."));
        break;
    case SlippyMapWidget::EllipseDrawing:
        m_actionObject_addEllipse->setChecked(true);
        statusBar()->showMessage(tr("Drag to draw ellipse. Esc to cancel."));
        break;
    case SlippyMapWidget::PolygonDrawing:
        m_actionObject_addPolygon->setChecked(true);
        statusBar()->showMessage(tr("Click to set polygon points. Double-click to finish drawing. Esc to cancel."));
    default:
        ui->slippyMap->setCursor(Qt::CrossCursor);
        statusBar()->clearMessage();
        break;
    }
}

void MainWindow::onSlippyMapLayerObjectActivated(const SlippyMapLayerObject::Ptr& object)
{
    static QMetaObject::Connection saveButtonConnection;

    for (const auto& layer: m_layerManager->layers()) {
        if (layer->objects().contains(object)) {
            m_layerManager->setActiveLayer(layer);
            break;
        }
    }

    if (m_selectedObject == object) return;

    auto *commonPropertyPage = new SlippyMapLayerObjectCommonPropertyPage(
        object, m_layerManager);

    connect(object.get(),
        &SlippyMapLayerObject::updated,
        commonPropertyPage,
        &SlippyMapLayerObjectPropertyPage::updateUi);

    QList<SlippyMapLayerObjectPropertyPage*> propertyPages;
    propertyPages.append(commonPropertyPage);
    propertyPages.append(new SlippyMapLayerObjectFilesPropertyPage(object, this));

    for (auto *propertyPage : object->propertyPages(object)) {
        propertyPages.append(propertyPage);

        connect(object.get(),
                &SlippyMapLayerObject::updated,
                propertyPage,
                &SlippyMapLayerObjectPropertyPage::updateUi);
    }

    for (auto *propertyPage : ExplorerApplication::pluginManager()->getPropertyPages()) {
        propertyPages.append(propertyPage);

        connect(object.get(),
                &SlippyMapLayerObject::updated,
                propertyPage,
                &SlippyMapLayerObjectPropertyPage::updateUi);
    }

    ui->tabShapeEditor->clear();
    for (auto *propertyPage : propertyPages) {
        propertyPage->setupUi();
        ui->tabShapeEditor->addTab(propertyPage, propertyPage->tabTitle());
    }

    disconnect(saveButtonConnection);
    saveButtonConnection = connect(
           ui->sidebarSaveButton,
           &QPushButton::clicked,
           [this, object, propertyPages]() {
               if (object->isEditable()) {
                   createUndoModifyObject(
                           tr("Edit ") + " " + object->label(),
                           object);

                   for (auto *propertyPage: propertyPages)
                       propertyPage->save();

                   object->setSynced(false);
                   saveObject(object);
               }
           });

    ui->tabShapeEditor->setVisible(true);
    ui->lblNoShapeSelected->setVisible(false);
    ui->sidebarResetButton->setVisible(true);
    ui->sidebarSaveButton->setVisible(true);

    m_selectedObjectCopy.clear();
    m_selectedObjectCopy = SlippyMapLayerObject::Ptr(object->clone());
    m_selectedObject = object;

    m_actionEdit_cut->setEnabled(true);
    m_actionEdit_copy->setEnabled(true);
    m_actionEdit_paste->setEnabled(true);
    m_actionEdit_delete->setEnabled(true);
    m_actionEdit_properties->setEnabled(true);
    m_addMarkerAction->setEnabled(true);
    m_deleteObjectAction->setEnabled(true);
}

void MainWindow::onSlippyMapLayerObjectDeactivated(const SlippyMapLayerObject::Ptr& object)
{
    (void)object;

    if (m_selectedObject != nullptr) {
        ui->tabShapeEditor->removeTab(1);
        ui->tabShapeEditor->setVisible(false);
        ui->lblNoShapeSelected->setVisible(true);
        ui->sidebarResetButton->setVisible(false);
        ui->sidebarSaveButton->setVisible(false);
        m_selectedObject = nullptr;

        m_actionEdit_cut->setEnabled(false);
        m_actionEdit_copy->setEnabled(false);
        m_actionEdit_paste->setEnabled(false);
        m_actionEdit_delete->setEnabled(false);
        m_actionEdit_properties->setEnabled(false);
        m_addMarkerAction->setEnabled(false);
        m_deleteObjectAction->setEnabled(false);
    }
}

void MainWindow::onSlippyMapLayerObjectWasDragged(const SlippyMapLayerObject::Ptr& object)
{
    createUndoModifyObject(
            tr("Move %1").arg(object->label()),
            object);
    object->setSynced(false);
    saveObject(object);
}

void MainWindow::onSlippyMapLayerObjectDoubleClicked(const SlippyMapLayerObject::Ptr& object) {
    if (m_selectedObject == nullptr) return;
    showPropertiesDialog(object);
}

void MainWindow::onSlippyMapDragFinished()
{
    // if (ui->actionWeather_ShowWFOGrid->isChecked()) {
    //     m_weatherService->getWeatherStationList(
    //             QPointF(
    //                     ui->slippyMap->longitude(),
    //                     ui->slippyMap->latitude()));
    // }

    loadViewportData();
}

void MainWindow::showPropertiesDialog(const SlippyMapLayerObject::Ptr& object)
{
    static QMetaObject::Connection saveButtonConnection;
    QList<SlippyMapLayerObjectPropertyPage*> propertyPages;

    SlippyMapGpsMarker::Ptr gpsMarker = object.dynamicCast<SlippyMapGpsMarker>();
    if (!gpsMarker.isNull())
        qDebug() << "Got a GPS marker!";

    //
    // we don't show these for GPS markers
    //
    if (object->isEditable()) {
        auto *commonPropertyPage = new SlippyMapLayerObjectCommonPropertyPage(object, m_layerManager, this);
        auto *filesPropertyPage = new SlippyMapLayerObjectFilesPropertyPage(object, this);
        propertyPages.append(commonPropertyPage);
        propertyPages.append(filesPropertyPage);

        connect(object.get(),
            &SlippyMapLayerObject::updated,
            commonPropertyPage,
            &SlippyMapLayerObjectPropertyPage::updateUi);

        connect(object.get(),
            &SlippyMapLayerObject::updated,
            filesPropertyPage,
            &SlippyMapLayerObjectPropertyPage::updateUi);
    }

    for (auto *propertyPage : object->propertyPages(object)) {
        propertyPages.append(propertyPage);
        connect(object.get(),
            &SlippyMapLayerObject::updated,
            propertyPage,
            &SlippyMapLayerObjectPropertyPage::updateUi);
    }

    if (object->isEditable()) {
        propertyPages.append(new DatabaseObjectPropertyPage(object));

        for (auto *propertyPage : ExplorerApplication::pluginManager()->getPropertyPages()) {
            propertyPages.append(propertyPage);
            connect(object.get(),
                &SlippyMapLayerObject::updated,
                propertyPage,
                &SlippyMapLayerObjectPropertyPage::updateUi);
        }
    }

    auto *tabWidget = new QTabWidget();

    for (auto *propertyPage : propertyPages) {
        propertyPage->setupUi();
        tabWidget->addTab(propertyPage, propertyPage->tabTitle());
    }

    auto *dialog = new QDialog(this);

    auto *saveButton = new QPushButton(tr("Save"));

    disconnect(saveButtonConnection);
    saveButtonConnection = connect(saveButton,
            &QPushButton::clicked,
            [this, object, propertyPages, dialog]() {
                if (object->isEditable()) {
                    createUndoModifyObject(
                            tr("Edit ") + " " + object->label(),
                            object);

                    for (auto *propertyPage: propertyPages)
                        propertyPage->save();

                    object->setSynced(false);
                    saveObject(object);
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

void MainWindow::onWeatherService_stationListReady(
        const QList<NationalWeatherServiceInterface::WeatherStation>& stations
        )
{
    for (const auto& marker: m_weatherStationMarkers) {
        m_layerManager->removeLayerObject(m_weatherLayer, marker);
    }

    m_weatherStationMarkers.clear();

    for (const auto& station : m_weatherService->stations()) {
        auto marker = WeatherStationMarker::Ptr::create(station.stationId);
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

void MainWindow::saveWorkspace()
{
    processDatabaseUpdates();
}

void MainWindow::openWorkspace()
{
    if (!closeWorkspace())
        return;

    loadWorkspaces();
}

void MainWindow::newWorkspace()
{
    if (!closeWorkspace())
        return;

    bool ok;
    QString workspaceName = QInputDialog::getText(
            this,
            tr("New Workspace"),
            tr("Enter the name of the new workspace"),
            QLineEdit::Normal,
            tr("New Workspace"),
            &ok
            );

    if (ok) {
        ServerInterface::Workspace workspace;
        workspace.id = QUuid::createUuid();
        workspace.name = workspaceName;
        workspace.description = ""; // todo: custom dialog for name and description

        m_serverInterface->createWorkspace(workspace,
            [this, workspace]() {
                m_workspaceId = workspace.id;
                m_workspaceName = workspace.name;
                setWindowTitle(m_workspaceName);
            },
            [this](ServerInterface::RequestError error) {
                // todo: handle create workspace error
            });
    }
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

    auto marker = SlippyMapWidgetMarker::Ptr::create();
    marker->setLabel(gridId);
    marker->setPosition(QPointF(longitude, latitude));
    m_layerManager->addLayerObject(m_weatherLayer, marker);
    m_weatherLayer->setVisible(true);
    m_weatherLayer->setEditable(false);

    QString forecastZoneUrl = properties["forecastZone"].toString();
}

void MainWindow::updateGpsData(const NmeaSerialLocationDataProvider::PositionData& position)
{
    if (m_gpsMarker.isNull()) {
        m_gpsMarker = SlippyMapGpsMarker::Ptr::create(position.position());
        m_gpsMarker->setLabel(tr("GPS"));
        m_gpsMarker->setEditable(false);
        m_gpsMarker->setMovable(false);
        m_gpsMarker->setColor(Qt::green);
        m_layerManager->addLayerObject(m_gpsMarkerLayer, m_gpsMarker);
    }

    m_gpsMarker->setGpsData(position);
    m_gpsMarker->setPosition(position.position());
    m_gpsMarker->setSatellites(position.satellites());
    m_gpsMarker->setGpsTime(position.gpsTime());

    if (m_actionTools_centerGps->isChecked()) {
        ui->slippyMap->setCenter(position.position());
    }

    if (m_actionTools_recordGpsPosition->isChecked()) {
        if (m_gpsTrack.isNull()) {
            m_gpsTrack = SlippyMapLayerTrack::Ptr::create();
            m_gpsTrack->setLabel(tr("GPS"));
            m_layerManager->addLayerObject(m_gpsMarkerLayer, m_gpsTrack);
        }

        m_gpsTrack->appendPoint(position.position());
    }
    else {
        if (!m_gpsTrack.isNull()) {
            m_layerManager->removeLayerObject(m_gpsMarkerLayer, m_gpsTrack);
            m_gpsTrack.clear();
        }
    }

    QString cardinal_lat;
    QString cardinal_lon;

    if (position.position().y() >= 0) cardinal_lat = "N";
    else cardinal_lat = "S";

    if (position.position().x() >= 0) cardinal_lon = "E";
    else cardinal_lon = "W";

    m_statusBarGpsStatusLabel->setText(tr("GPS Postion: %1 %2 %3 %4")
        .arg(position.position().y(), 0, 'f', 7)
        .arg(cardinal_lat)
        .arg(position.position().x(), 0, 'f', 7)
        .arg(cardinal_lon));
}

void MainWindow::onTvwMarkersContextMenuRequested(const QPoint &point)
{
    QModelIndex index = ui->tvwMarkers->indexAt(point);
    if (index.isValid()) {
        if (!index.parent().isValid()) {
            SlippyMapLayer::Ptr layer = m_layerManager->layers().at(index.row());

            m_clearLayerAction->setVisible(true);
            m_deleteLayerAction->setVisible(true);
            m_renameLayerAction->setVisible(true);

            // can only do this if layer is editable
            m_clearLayerAction->setEnabled(layer->isEditable());
            m_deleteLayerAction->setEnabled(layer->isEditable());
            m_renameLayerAction->setEnabled(layer->isEditable());

            // update the visibility checkbox state
            m_layerVisibilityAction->setChecked(layer->isVisible());
            m_markerLockedAction->setChecked(!layer->isEditable());

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
    showPropertiesDialog(m_selectedObject);
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
    auto marker = SlippyMapWidgetMarker::Ptr::create(position);
    marker->setLabel(SlippyMapWidget::latLonToString(position.y(), position.x()));
    marker->setDescription(tr("Test Label"));
    marker->setColor(m_fillColorSelector->color());

    SlippyMapLayer::Ptr target = m_layerManager->activeLayer();
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

    saveObject(marker);
}

void MainWindow::centerMapOnContextMenuPosition()
{
    QPointF coords = ui->slippyMap->widgetCoordsToGeoCoords(m_contextMenuLocation);
    ui->slippyMap->setCenter(coords);
}

void MainWindow::saveWindowSize()
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

void MainWindow::showSettingsDialog()
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

void MainWindow::showAddGpsSourceDialog()
{
    GpsSourceDialog::GpsSourceInfo source =
            GpsSourceDialog::getNewSource(
                this,
                tr("Add GPS Source"));

    if (source.isValid) {
        if (source.sourceType == GpsSourceDialog::NmeaSource) {
            m_gpsLocationProvider = new NmeaSerialLocationDataProvider();
            m_gpsLocationProvider->setLabelText(source.label);
            m_gpsLocationProvider->setPortName(source.portName);
            m_gpsLocationProvider->setBaudRate(source.baudRate);
            m_gpsLocationProvider->setDataBits(source.dataBits);
            m_gpsLocationProvider->setStopBits(source.stopBits);
            m_gpsLocationProvider->setParity(source.parity);
            m_gpsLocationProvider->setFlowControl(source.flowControl);

            connect(m_gpsLocationProvider,
                &NmeaSerialLocationDataProvider::gpsUpdated,
                this,
                &MainWindow::updateGpsData);

            m_gpsLocationProvider->start();
            m_actionTools_connectGps->setEnabled(false);
            m_actionTools_disconnectGps->setEnabled(true);
            m_actionTools_viewGpsLog->setEnabled(true);
        }
    }
}

void MainWindow::showGpsLogDialog()
{
    Q_ASSERT(m_gpsLocationProvider != nullptr);

    if (m_nmeaLog == nullptr) {
        m_nmeaLog = new TextLogViewerForm();
        m_nmeaLog->setWindowTitle(tr("NMEA Log"));
        connect(m_gpsLocationProvider,
                &NmeaSerialLocationDataProvider::lineReceived,
                m_nmeaLog,
                &TextLogViewerForm::addLine);
    }

    m_nmeaLog->show();
}

void MainWindow::showObjectBrowserDialog()
{
    qDebug() << "Showing object browser";

    if (m_objectBrowser == nullptr) {
        m_objectBrowser = new ObjectBrowserDialog(m_workspaceId);
    }

    m_objectBrowser->setWindowTitle(tr("Object Browser"));
    m_objectBrowser->refresh();
    m_objectBrowser->show();
}

void MainWindow::activateLayerAtIndex(const QModelIndex &index)
{
    if (index.isValid() && !index.parent().isValid()) {
        auto mappedIndex = m_layerManagerProxy->mapToSource(index);
        int layerIndex = mappedIndex.row();
        for (int i = 0; i < m_layerManager->layers().count(); i++) {
            if (i == layerIndex) {
                m_layerManager->setActiveLayer(m_layerManager->layers().at(i));
                break;
            }
        }
    }
}

void MainWindow::setDrawRectMode()
{
    ui->slippyMap->setDrawMode(SlippyMapWidget::RectDrawing);
}

void MainWindow::setDrawEllipseMode()
{
    ui->slippyMap->setDrawMode(SlippyMapWidget::EllipseDrawing);
}

void MainWindow::setDrawMarkerMode()
{
    ui->slippyMap->setDrawMode(SlippyMapWidget::MarkerDrawing);
}

void MainWindow::importGpx()
{
    if (m_layerManager->activeLayer() == nullptr) {
        QMessageBox::warning(
                this,
                tr("No Layer"),
                tr("You must first select a layer"));
        return;
    }
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
                auto layerTrack = SlippyMapLayerTrack::Ptr::create(track);
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
                saveObject(layerTrack);
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

void MainWindow::on_geoCodingInterface_locationFound(QList<GeoCodingInterface::GeoCodedAddress> locations)
{
    for (const auto& location: locations) {
        qDebug() << "Location received:" << location.location;
    }

    if (locations.count() > 0) {
        QPointF coord = locations.at(0).location;
        ui->slippyMap->setCenter(coord);

        // get rid of existing marker
        m_layerManager->removeLayerObjects(m_searchMarkerLayer);

        auto marker = SlippyMapWidgetMarker::Ptr::create(this);
        marker->setLabel(locations.at(0).formatted);
        marker->setPosition(coord);
        marker->setEditable(false);
        marker->setMovable(false);
        marker->setColor(QColor(0xFFCC0000));
        m_layerManager->addLayerObject(m_searchMarkerLayer, marker);
    }
    else {
        QMessageBox::information(
            this,
            tr("Not Found"),
            tr("The location you entered was not found. Please try again."));
    }
}

void MainWindow::setWorkspaceDirty(bool dirty)
{
    QString fileName = m_workspaceFileName;
    if (fileName.isEmpty())
        fileName = "Untitled.osm";

    m_workspaceDirty = dirty;

    if (dirty) {
        m_actionFile_saveWorkspace->setEnabled(true);
        if (m_databaseMode)
            setWindowTitle(
                    tr("Online Workspace") + " - " + \
                    m_workspaceName + "*");
        else
            setWindowTitle(fileName + "*");
    }
    else {
        m_actionFile_saveWorkspace->setEnabled(false);
        if (m_databaseMode)
            setWindowTitle(
                    tr("Online Workspace") + " - " + \
                    m_workspaceName);
        else
            setWindowTitle(fileName);
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
            saveWorkspace();
            m_historyManager->clearUndoHistory();
            return true;
        }
        if (result == QMessageBox::Cancel)
            return false;
    }

    m_workspaceId = QUuid();

    for (const auto& layer : m_layerManager->layers()) {
        if (layer->isEditable()) {
            m_layerManager->takeLayer(layer);
            layer->removeAll();
        }
    }

    m_workspaceFileName.clear();
    m_historyManager->clearUndoHistory();
    setWorkspaceDirty(true);
    return true;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (m_workspaceDirty) {
        bool result = closeWorkspace();

        if (!result) {
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
        auto newLayer = SlippyMapLayer::Ptr::create();
        newLayer->setName(layerName);
        m_layerManager->addLayer(newLayer);
        m_databaseLayerUpdateList.append(newLayer);
        createUndoAddLayer(tr("New Layer"), newLayer);
        processDatabaseUpdates();
    }
}

void MainWindow::deleteSelectedLayer()
{
    QModelIndex selectedLayer = ui->tvwMarkers->currentIndex();
    if (selectedLayer.isValid() && !selectedLayer.parent().isValid()) {
        SlippyMapLayer::Ptr layer = m_layerManager->layers().at(selectedLayer.row());
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
                if (m_databaseMode) {
                    if (!m_databaseLayerDeleteList.contains(layer))
                        m_databaseLayerDeleteList.append(layer);
                }
            }
        }
    }
}

void MainWindow::clearSelectedLayer()
{
    QModelIndex selectedLayer = ui->tvwMarkers->currentIndex();
    if (selectedLayer.isValid() && !selectedLayer.parent().isValid()) {
        SlippyMapLayer::Ptr layer = m_layerManager->layers().at(selectedLayer.row());
        if (layer->isEditable()) {
            int result = QMessageBox::question(
                    this,
                    tr("Clear Layer"),
                    tr("Do you want to remove all objects from the layer '%1'? This action cannot be undone.").arg(layer->name()),
                    QMessageBox::StandardButtons(QMessageBox::Yes | QMessageBox::No));

            if (result == QMessageBox::Yes) {
                for (const auto& object: layer->objects())
                    m_databaseObjectDeleteList.append(object);

                m_layerManager->removeLayerObjects(layer);
                m_historyManager->clearUndoHistory();
                setWorkspaceDirty(true);
                processDatabaseUpdates();
            }
        }
    }
}

void MainWindow::deleteActiveObject()
{
    if (m_selectedObject.isNull()) return;

    SlippyMapLayerObject::Ptr object = m_selectedObject.toStrongRef();

    int result = QMessageBox::question(
            this,
            tr("Delete Object"),
            tr("Do you want to delete the object '%1'?").arg(object->label()),
            QMessageBox::StandardButtons(QMessageBox::Yes | QMessageBox::No));

    if (result == QMessageBox::Yes) {
        createUndoDeleteObject(
                tr("Delete %1").arg(object->label()),
                m_layerManager->activeLayer(),
                object);

        qDebug() << "Removing object" << object->label() << "from layerManager";
        m_layerManager->removeLayerObject(
                m_layerManager->activeLayer(),
                object);

        object->setSynced(false);

        // append to the delete list to be sent to db on next sync
        // unless it doesn't have an id yet
//        qDebug() << "Object" << object->label() << "going to delete list";
//        if (m_databaseMode && !object->id().toString().isEmpty()) {
//            m_databaseObjectDeleteList.append(object);
//        }

        deleteObject(object);
    }
}

void MainWindow::onTvwMarkersClicked(const QModelIndex &index)
{
    if (!index.isValid()) {
        ui->tvwMarkers->clearSelection();
        m_layerManager->deactivateLayer();
    }

    auto mappedIndex = m_layerManagerProxy->mapToSource(index);

    //
    // set the selected layer to active
    //
    if (!index.parent().isValid()) {
        const auto& layer = m_layerManager->layers().at(mappedIndex.row());
        m_layerManager->setActiveLayer(layer);
        ui->slippyMap->deactivateActiveObject();

        if (layer->isEditable())
            enableDrawing();
        else
            disableDrawing();

        return;
    }

    const auto& layer = m_layerManager->layers().at(mappedIndex.parent().row());
    const auto& object = layer->objects().at(mappedIndex.row());
    QPointF position = object->position();
    m_layerManager->setActiveLayer(layer);
    ui->slippyMap->setActiveObject(object);
    ui->slippyMap->setCenter(position);
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
        m_layerManager->activeLayer()->setSynced(false);
        m_layerManager->updateActiveLayer();
        setWorkspaceDirty(true);
        if (m_databaseMode) {
            if (!m_databaseLayerUpdateList.contains(m_layerManager->activeLayer()))
                m_databaseLayerUpdateList.append(m_layerManager->activeLayer());
        }
    }
}

void MainWindow::deleteActiveLayer()
{
    if (m_layerManager->activeLayer() == nullptr) return;
    const auto& layer = m_layerManager->activeLayer();

    if (layer->isEditable()) {
        int result = QMessageBox::question(
                this,
                tr("Clear Layer"),
                tr("Do you want to delete the layer '%1'?").arg(layer->name()),
                QMessageBox::StandardButtons(QMessageBox::Yes | QMessageBox::No));

        if (result == QMessageBox::Yes) {
            createUndoDeleteLayer(
                    tr("Delete Layer"),
                    layer);
            m_layerManager->takeLayer(layer);
            setWorkspaceDirty(true);
            if (m_databaseMode) {
                if (!m_databaseLayerDeleteList.contains(layer))
                    m_databaseLayerDeleteList.append(layer);
            }
        }
    }
}

void MainWindow::onSlippyMapPolygonSelected(const QList<QPointF>& points)
{
    if (m_layerManager->activeLayer() == nullptr) return;

    QVector<QPointF> pointVector = QVector<QPointF>::fromList(points);
    auto polygon = SlippyMapLayerPolygon::Ptr::create(pointVector);
    polygon->setLabel(tr("New Polygon"));
    polygon->setStrokeColor(m_strokeColorSelector->color());
    polygon->setStrokeWidth(m_strokeWidth->value());
    polygon->setFillColor(m_fillColorSelector->color());
    m_layerManager->addLayerObject(polygon);

    createUndoAddObject(
            tr("New Polygon"),
            m_layerManager->activeLayer(),
            polygon);
    saveObject(polygon);
}

void MainWindow::setDrawPolygonMode()
{
    ui->slippyMap->setDrawMode(SlippyMapWidget::PolygonDrawing);
}

void MainWindow::onSlippyMapPathSelected(const QList<QPointF> &points)
{
    qDebug() << "Path points selected:" << points;
    if (m_layerManager->activeLayer() == nullptr) return;

    QVector<QPointF> pointVector = QVector<QPointF>::fromList(points);
    auto path = SlippyMapLayerPath::Ptr::create(pointVector);
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
    saveObject(path);
}

void MainWindow::onSlippyMapPointSelected(const QPointF &point)
{
    createMarkerAtPosition(point);
}

void MainWindow::advanceLayerAnimationFrame()
{
    if (m_animationState == Forward)
        ui->slippyMap->nextFrame();
    else if (m_animationState == Reverse)
        ui->slippyMap->previousFrame();
}

void MainWindow::onSlippyMapLayerObjectUpdated(const SlippyMapLayerObject::Ptr& object)
{
    qDebug() << "Object updated";
    ui->slippyMap->update();
}

void MainWindow::setDrawLineMode()
{
    ui->slippyMap->setDrawMode(SlippyMapWidget::PathDrawing);
}

void MainWindow::undo()
{
    if (m_historyManager->undoCount() > 0) {
        HistoryManager::HistoryEvent event = m_historyManager->undoEvent();

        switch (event.action) {
            case HistoryManager::AddObject: {
                const auto& layer = event.layer;
                SlippyMapLayerObject::Ptr object = event.original;
                m_layerManager->removeLayerObject(layer, object);

                // the user may have created, then saved, then
                // wanted to undo, so now it may have an id and need
                // to be deleted off the db
                if (m_databaseMode && !object->id().toString().isEmpty())
                    m_databaseObjectDeleteList.append(object);

                break;
            }
            case HistoryManager::DeleteObject: {
                SlippyMapLayer::Ptr layer = event.layer;
                SlippyMapLayerObject::Ptr object = event.original;
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
                SlippyMapLayerObject::Ptr temp = SlippyMapLayerObject::Ptr(event.original->clone());
                SlippyMapLayerObject::Ptr object = event.original;
                SlippyMapLayerObject::Ptr clone = event.copy;

                // we need to replace the original with the copy
                event.original->copy(event.copy.get());

                // now put the "new" contents into copy for redo later
                event.copy->copy(temp.get());
                break;
            }
            case HistoryManager::AddLayer: {
                const auto& layer = event.layer;
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
                SlippyMapLayer::Ptr layer = event.layer;
                m_layerManager->addLayer(layer);

                // remove it from delete list
                if (m_databaseMode && m_databaseLayerDeleteList.contains(layer))
                    m_databaseLayerDeleteList.removeOne(layer);

                break;
            }
        }

        setWorkspaceDirty(true);
    }

    m_actionEdit_redo->setEnabled(true);
    m_actionEdit_redo->setText(tr("Redo") + " " + m_historyManager->currentRedoDescription());
    m_actionEdit_undo->setEnabled(m_historyManager->undoCount() > 0);
    if (m_historyManager->undoCount() > 0)
        m_actionEdit_undo->setText(tr("Undo") + " " + m_historyManager->currentUndoDescription());
}

void MainWindow::redo()
{
    if (m_historyManager->redoCount() > 0) {
        HistoryManager::HistoryEvent event = m_historyManager->redoEvent();

        switch (event.action) {
            case HistoryManager::DeleteObject: {
                SlippyMapLayer::Ptr layer = event.layer;
                SlippyMapLayerObject::Ptr object = event.original;
                m_layerManager->removeLayerObject(layer, object);

                // put it back on the delete list (unless it
                // doesn't have an id yet
                if (m_databaseMode && !object->id().toString().isEmpty())
                    m_databaseObjectDeleteList.append(object);

                break;
            }
            case HistoryManager::AddObject: {
                SlippyMapLayer::Ptr layer = event.layer;
                SlippyMapLayerObject::Ptr object = event.original;
                m_layerManager->addLayerObject(layer, object);

                // remove it from the delete list
                if (m_databaseMode && m_databaseObjectDeleteList.contains(object))
                    m_databaseObjectDeleteList.removeOne(object);

                break;
            }
            case HistoryManager::ModifyObject: {
                // the clone contains the original contents
                SlippyMapLayerObject::Ptr temp = SlippyMapLayerObject::Ptr(event.original->clone());
                SlippyMapLayerObject::Ptr object = event.original;
                SlippyMapLayerObject::Ptr clone = event.copy;

                // we need to replace the original with the copy
                event.original->copy(event.copy.get());

                // now put the "new" contents into copy for undo later
                event.copy->copy(temp.get());
                break;
            }
            case HistoryManager::AddLayer: {
                SlippyMapLayer::Ptr layer = event.layer;
                m_layerManager->addLayer(layer);

                // remove it from delete list
                if (m_databaseMode && m_databaseLayerDeleteList.contains(layer))
                    m_databaseLayerDeleteList.removeOne(layer);

                break;
            }
            case HistoryManager::DeleteLayer: {
                SlippyMapLayer::Ptr layer = event.layer;
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

    m_actionEdit_redo->setEnabled(m_historyManager->redoCount() > 0);
    if (m_historyManager->redoCount() > 0)
        m_actionEdit_redo->setText(tr("Redo") + " " + m_historyManager->currentRedoDescription());
    m_actionEdit_undo->setEnabled(true);
    if (m_historyManager->undoCount() > 0)
        m_actionEdit_undo->setText(tr("Undo") + " " + m_historyManager->currentUndoDescription());
}

void MainWindow::createUndoAddObject(const QString &description, SlippyMapLayer::Ptr layer, const SlippyMapLayerObject::Ptr& object)
{
    HistoryManager::HistoryEvent event;
    event.action = HistoryManager::AddObject;
    event.layer = SlippyMapLayer::Ptr(layer);
    event.original = object;
    event.copy = nullptr;
    event.description = description;
    m_historyManager->addEvent(event);
    m_actionEdit_undo->setEnabled(true);
    m_actionEdit_undo->setText(tr("Undo") + " " + description);
    setWorkspaceDirty(true);
}

void MainWindow::createUndoModifyObject(const QString &description, const SlippyMapLayerObject::Ptr& object)
{
    Q_CHECK_PTR(m_selectedObjectCopy);

    HistoryManager::HistoryEvent event;
    event.description = description;
    event.action = HistoryManager::ModifyObject;
    event.original = object;
    // create a copy of the original object to restore later
    // if requested by undo
    event.copy = m_selectedObjectCopy;
    m_historyManager->addEvent(event);
    m_selectedObjectCopy = SlippyMapLayerObject::Ptr(object->clone());
    setWorkspaceDirty(true);
}

void MainWindow::createUndoDeleteObject(const QString &description,
                                        const SlippyMapLayer::Ptr& layer,
                                        const SlippyMapLayerObject::Ptr& object)
{
    HistoryManager::HistoryEvent event;
    event.description = description;
    event.action = HistoryManager::DeleteObject;
    event.layer = layer;
    event.original = object;
    m_historyManager->addEvent(event);
    setWorkspaceDirty(true);
}

void MainWindow::createUndoAddLayer(const QString &description, SlippyMapLayer::Ptr layer)
{
    HistoryManager::HistoryEvent event;
    event.description = description;
    event.action = HistoryManager::AddLayer;
    event.layer = layer;
    m_historyManager->addEvent(event);
    setWorkspaceDirty(true);
}

void MainWindow::createUndoDeleteLayer(const QString &description, SlippyMapLayer::Ptr layer)
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
    m_actionEdit_redo->setEnabled(hist->redoCount() > 0);
    if (hist->redoCount() > 0)
        m_actionEdit_redo->setText(tr("Redo") + " " + hist->currentRedoDescription());
    m_actionEdit_undo->setEnabled(hist->undoCount() > 0);
    if (hist->undoCount() > 0)
        m_actionEdit_undo->setText(tr("Undo") + " " + hist->currentUndoDescription());
}

void MainWindow::redoHistoryCleared()
{
    m_actionEdit_redo->setEnabled(false);
    m_actionEdit_redo->setText(tr("Redo"));
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
    m_actionEdit_paste->setEnabled(true);

    // create a delete object undo item
    createUndoDeleteObject(
            tr("Cut"),
            m_layerManager->activeLayer(),
            m_selectedObject);

    // remove the item from the layer
    m_layerManager->removeLayerObject(m_layerManager->activeLayer(), m_selectedObject);
    deleteObject(m_selectedObject);
    m_selectedObject.clear();
}

void MainWindow::copyActiveObject()
{
    if (m_selectedObject.isNull()) return;

    const auto& object = m_selectedObject.toStrongRef();

    if (!m_clipBoard.object.isNull()) {
        // get rid of the object
        m_clipBoard.object.clear();
    }

    // create clipboard entry
    m_clipBoard.type = Clipboard::Object;
    m_clipBoard.action = Clipboard::Copy;
    m_clipBoard.layer = m_layerManager->activeLayer();
    m_clipBoard.object = SlippyMapLayerObject::Ptr(object->clone());

    // activate paste menu entry
    m_actionEdit_paste->setEnabled(true);
}

void MainWindow::pasteObject()
{
    switch (m_clipBoard.type) {
        case Clipboard::Object: {
            auto layer = m_clipBoard.layer;
            auto object = m_clipBoard.object;

            // paste it onto the active layer, if there is one.
            // otherwise go to the layer it came from?
            if (m_layerManager->activeLayer() != nullptr)
                layer = m_layerManager->activeLayer();
            else
                return;

            // clone the object because you don't want to paste
            // the same object
            auto clone = SlippyMapLayerObject::Ptr(object->clone());
            m_layerManager->addLayerObject(layer, clone);

            createUndoAddObject(
                    tr("Paste"),
                    layer,
                    object);

            // add to the database create/update list
            saveObject(object);
            break;
        }
        default:
            break;
    }
}

void MainWindow::startServerLogin()
{
    // m_serverInterface->login(
    //     this,
    //     [this](const QString& token) {
    //         // get the workspaces
    //         setDatabaseMode(true);
    //         loadWorkspaces();
    //     },
    //     [this](ServerInterface::RequestError error) {
    //         switch (error) {
    //         case ServerInterface::RequestFailedError:
    //         case ServerInterface::InvalidRequestError:
    //             QMessageBox::critical(
    //                 this,
    //                 tr("Server Error"),
    //                 tr("There was an error performing the request. Please try again."));
    //             break;
    //         case ServerInterface::AuthenticationError:
    //             QMessageBox::critical(
    //                 this,
    //                 tr("Login Error"),
    //                 tr("Username and/or password incorrect. Please try again."));
    //             break;
    //         case ServerInterface::UserCancelledError:
    //             return;
    //         default:
    //             break;
    //         }
    //
    //         startServerLogin();
    //     });
}

void MainWindow::loadViewportData()
{
    auto boundingBox = ui->slippyMap->boundingBoxLatLon();

    // for (const auto& layer: m_layerManager->layers()) {
    //     if (layer->isEditable())
    //         m_layerManager->removeLayerObjects(layer);
    // }

    m_serverInterface->getLayersForViewport(
        m_workspaceId,
        boundingBox,
        [this](const QList<ServerInterface::Layer>& layers) {
            for (const auto& layerdata: layers) {
                SlippyMapLayer::Ptr layer = nullptr;
                for (const auto& l: m_layerManager->layers()) {
                    if (layerdata.id.toString().compare(l->id().toString()) == 0) {
                        qDebug() << "Adding/updating layer" << layerdata.id;
                        layer = l;
                    }
                }

                if (layer == nullptr) {
                    layer = SlippyMapLayer::Ptr::create();
                    layer->setId(layerdata.id);
                    layer->setSynced(true); // so we get the first update
                    m_layerManager->addLayer(layer);
                }
                else {
                    // if the layer is in the delete list, don't worry about it
                    if (m_databaseLayerDeleteList.contains(layer)) {
                        continue;
                    }
                }

                //
                // don't overwrite local changes
                //
                if (layer->isSynced()) {
                    layer->setName(layerdata.name);
                    layer->setDescription(layerdata.description);
                    layer->setColor(layerdata.color);
                }

                for (const auto& objectdata: layerdata.objects) {
                    //replace("SRID=4326;", "");

                    QString geom = objectdata.geom;
                    geom.replace("SRID=4326;", "");

                    QString className = objectdata.type;
                    className.append("*");
                    bool found = false;

                    for (const auto& object: layer->objects()) {
                        if (object->id().compare(objectdata.id.toString()) == 0) {
                            if (object->isSynced()) {
                                // update with new data
                                object->setLabel(objectdata.label);
                                object->setDescription(objectdata.description);
                                object->hydrateFromDatabase(objectdata.data, geom);
                            }
                            found = true;
                        }
                    }

                    if (found) continue;

                    // prevent item from being recreated on viewport update if
                    // it's already in the delete list
                    for (const auto& object: m_databaseObjectDeleteList) {
                        if (object->id().compare(objectdata.id.toString()) == 0) {
                            found = true;
                        }
                    }

                    if (found) continue;

                    // get the type information from qt meta
                    int typeId = QMetaType::type(className.toLocal8Bit());
                    const QMetaObject *metaObject = QMetaType::metaObjectForType(typeId);

                    // create a new object and cast to layer object
                    QObject *o = metaObject->newInstance();
                    auto object = SlippyMapLayerObject::Ptr(qobject_cast<SlippyMapLayerObject *>(o));

                    object->setId(objectdata.id);
                    object->setLayerId(layer->id());
                    object->setLabel(objectdata.label);
                    object->setDescription(objectdata.description);
                    object->hydrateFromDatabase(objectdata.data, geom);
                    object->setSynced(true);

                    m_layerManager->addLayerObject(layer, object);
                }
            }

        },
        [this](ServerInterface::RequestError error) {
            // todo: handle layer load error
        });
}

void MainWindow::processDatabaseUpdates()
{
    if (!m_databaseLayerDeleteList.isEmpty()) {
        for (const auto &layer: m_databaseLayerDeleteList) {
            for (const auto& object: layer->objects()) {
                ServerSyncRequest sync;
                sync.type = SyncRequestDelete;
                sync.object = object;
                m_serverSyncRequestQueue.append(sync);
            }

            ServerSyncRequest sync;
            sync.type = SyncRequestDelete;
            sync.layer = layer;
            m_serverSyncRequestQueue.append(sync);
        }
        m_databaseLayerDeleteList.clear();
    }

    if (!m_databaseLayerUpdateList.isEmpty()) {
        for (const auto& layer: m_databaseLayerUpdateList) {
            if (!layer->isEditable()) continue;

            QJsonObject json;

            if (layer->id().toString().isEmpty())
                layer->setId(QUuid::createUuid().toString());

            ServerSyncRequest sync;
            sync.type = SyncRequestPut;
            sync.layer = layer;
            m_serverSyncRequestQueue.append(sync);
        }
        m_databaseLayerUpdateList.clear();
    }

    if (!m_databaseObjectDeleteList.isEmpty()) {
        for (const auto& object: m_databaseObjectDeleteList) {
            ServerSyncRequest sync;
            sync.type = SyncRequestDelete;
            sync.object = object;
            m_serverSyncRequestQueue.append(sync);
        }
        m_databaseObjectDeleteList.clear();
    }

    if (!m_databaseObjectUpdateList.isEmpty()) {
        for (const auto& object: m_databaseObjectUpdateList) {
            QJsonObject json;

            if (object->id().toString().isEmpty()) {
                object->setId(QUuid::createUuid().toString());
            }

            const auto& layer = m_layerManager->layerForObject(object);

            ServerSyncRequest sync;
            sync.type = SyncRequestPut;
            sync.object = object;
            sync.layer = layer;

            m_serverSyncRequestQueue.append(sync);
        }
        // all items on the queue, can clear this list
        m_databaseObjectUpdateList.clear();
    }

    // kick off the requests
    m_serverRequestProgress->setMinimum(0);
    m_serverRequestProgress->setMaximum(m_serverSyncRequestQueue.count());
    m_serverRequestProgress->setValue(0);
    m_serverRequestProgress->setVisible(true);
    processDatabaseUpdateQueue();
}

void MainWindow::processDatabaseUpdateQueue()
{
    if (!m_serverSyncRequestQueue.isEmpty()) {
        const ServerSyncRequest& sync = m_serverSyncRequestQueue.first();

        switch (sync.type) {
            case SyncRequestDelete: {
                if (!sync.object.isNull()) {
                    m_serverInterface->deleteObject(
                        sync.object->id().toUuid(),
                        [this]() {
                            int val = m_serverRequestProgress->value();
                            m_serverRequestProgress->setValue(val + 1);
                            m_serverSyncRequestQueue.removeFirst();
                            processDatabaseUpdateQueue();
                        },
                        [this](ServerInterface::RequestError error) {

                        });
                }
                else if (!sync.layer.isNull()) {
                    m_serverInterface->deleteLayer(
                        sync.layer->id().toUuid(),
                        [this]() {
                            int val = m_serverRequestProgress->value();
                            m_serverRequestProgress->setValue(val + 1);
                            m_serverSyncRequestQueue.removeFirst();
                            processDatabaseUpdateQueue();
                        },
                        [this](ServerInterface::RequestError error) {

                        });
                }
                break;
            }
            case SyncRequestPut: {
                if (!sync.object.isNull()) {
                    ServerInterface::Object object;
                    object.id = sync.object->id().toUuid();
                    object.layerId = sync.layer->id().toUuid();
                    object.label = sync.object->label();
                    object.description = sync.object->description();
                    object.type = QString(sync.object->metaObject()->className());
                    object.visible = sync.object->isVisible();

                    QJsonObject data;
                    QString geom;
                    sync.object->packageObjectData(data, geom);
                    object.data = data;
                    object.geom = geom;

                    m_serverInterface->saveObject(
                        object,
                        [this, sync]() {
                            m_serverSyncRequestQueue.removeFirst();
                            sync.object->setSynced(true);

                            int val = m_serverRequestProgress->value();
                            m_serverRequestProgress->setValue(val + 1);

                            processDatabaseUpdateQueue();
                        },
                        [this](ServerInterface::RequestError error) {
                            QMessageBox::critical(
                                this,
                                tr("Server Error"),
                                tr("There was an error syncing to the database."));
                            // todo: indicate that we aren't synced and the user should
                            //  click save
                        });
                }
                else if (!sync.layer.isNull()) {
                    ServerInterface::Layer layer;
                    layer.id = sync.layer->id().toUuid();
                    layer.workspaceId = m_workspaceId;
                    layer.name = sync.layer->name();
                    layer.description = sync.layer->description();
                    layer.order = sync.layer->order();
                    layer.color = sync.layer->color();

                    m_serverInterface->saveLayer(
                        layer,
                        [this, sync]() {
                            m_serverSyncRequestQueue.removeFirst();
                            sync.layer->setSynced(true);

                            int val = m_serverRequestProgress->value();
                            m_serverRequestProgress->setValue(val + 1);

                            processDatabaseUpdateQueue();
                        },
                        [this](ServerInterface::RequestError error) {
                            QMessageBox::critical(
                                this,
                                tr("Server Error"),
                                tr("There was an error syncing to the database."));
                            // todo: indicate that we aren't synced and the user should
                            //  click save
                        });
                }
                break;
            }
        }
    }
    else {
        m_serverRequestProgress->setVisible(false);
        setWorkspaceDirty(false);
    }
}

void MainWindow::disableDrawing()
{
    m_drawingActionGroup->setEnabled(false);
    m_addMarkerAction->setEnabled(false);
}

void MainWindow::enableDrawing()
{
    m_drawingActionGroup->setEnabled(true);
    m_addMarkerAction->setEnabled(true);
}

void MainWindow::openOrCreateWorkspace(const QList<ServerInterface::Workspace>& workspaces)
{
    WorkspaceSelectionDialog dlg(this);

    QMap<QUuid,QString> workspaceMap;
    for (const auto& workspace: workspaces) {
        workspaceMap[workspace.id] = workspace.name;
    }

    dlg.setWorkspaceList(workspaceMap);
    int result = dlg.exec();

    if (result == QDialog::Accepted) {
        if (dlg.createNew()) {
            QUuid id = QUuid::createUuid();
            QString name = dlg.newWorkspaceName();
            QString description = dlg.newWorkspaceDescription();

            ServerInterface::Workspace workspace;
            workspace.id = id;
            workspace.name = name;
            workspace.description = description;

            m_serverInterface->createWorkspace(workspace,
            [this, workspace]() {
                    m_workspaceId = workspace.id;
                    m_workspaceName = workspace.name;
                    setWindowTitle(
                            tr("Online Workspace") + " - " + \
                            m_workspaceName);
                    loadViewportData();
                    setWorkspaceDirty(false);
                },
                [this](ServerInterface::RequestError error) {
                    QMessageBox::critical(
                            this,
                            tr("Server Error"),
                            tr("Error processing request. Please try again."));
                });
        }
        else {
            m_workspaceId = dlg.existingWorkspaceId();
            m_workspaceName = workspaceMap[m_workspaceId];
            setWindowTitle(
                    tr("Online Workspace") + " - " + \
                    m_workspaceName);
            loadViewportData();
            setWorkspaceDirty(false);
        }
    }
    else
        close();
}

void MainWindow::setDatabaseMode(bool databaseMode)
{
    m_databaseMode = databaseMode;
}

void MainWindow::loadWorkspaces()
{
    m_serverInterface->getWorkspaceList(
        [this](const QList<ServerInterface::Workspace>& workspaces) {
            qDebug() << "Got" << workspaces.size() << "workspaces";
            openOrCreateWorkspace(workspaces);
        },
        [this](ServerInterface::RequestError error) {
            // todo: show error indicating loading workspaces failed
        });
}

void MainWindow::startLocalServer()
{
    m_localServer = new QLocalServer(this);
    connect(m_localServer,
        &QLocalServer::newConnection,
        [this]() {
            auto *socket = m_localServer->nextPendingConnection();
            qDebug() << "Got local connection!";
        });
}

void MainWindow::getGeoCodedLocation()
{
    if (!m_toolBarGeoCodingInput->text().isEmpty())
        m_geoCodingInterface->submitQuery(m_toolBarGeoCodingInput->text());
}

void MainWindow::setWindowTitle(const QString &title)
{
    QWidget::setWindowTitle(tr("OSMExplorer") + " - " + title);
}

void MainWindow::saveObject(const SlippyMapLayerObject::Ptr &object)
{
    m_databaseObjectUpdateList.append(object);
    processDatabaseUpdates();
}

void MainWindow::deleteObject(const SlippyMapLayerObject::Ptr &object)
{
    if (object->id().isNull()) return;
    m_databaseObjectDeleteList.append(object);
    processDatabaseUpdates();
}
