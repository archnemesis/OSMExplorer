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
    setWindowTitle(tr("Untitled.osm"));
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

    connect(ui->actionFile_NewWorkspace,
            &QAction::triggered,
            this,
            &MainWindow::newWorkspace);

    connect(ui->actionFile_SaveWorkspace,
        &QAction::triggered,
        this,
            QOverload<>::of(&MainWindow::saveWorkspace)
            );

    connect(ui->actionFile_OpenWorkspace,
            &QAction::triggered,
            this,
            &MainWindow::openWorkspace);

    connect(ui->actionFile_CloseWorkspace,
            &QAction::triggered,
            this,
            &MainWindow::onActionFileCloseWorkspaceTriggered);

    connect(ui->actionExit,
        &QAction::triggered,
        this,
        &MainWindow::close);

    connect(ui->actionLayer_SortAscending,
            &QAction::triggered,
            [this]() {
        m_layerManager->sort(SlippyMapLayerManager::LayerSortName, Qt::AscendingOrder);
    });

    connect(ui->actionLayer_SortDescending,
            &QAction::triggered,
            [this]() {
                m_layerManager->sort(SlippyMapLayerManager::LayerSortName, Qt::DescendingOrder);
            });

    QActionGroup *sortActionGroup = new QActionGroup(this);
    sortActionGroup->addAction(ui->actionLayer_SortAscending);
    sortActionGroup->addAction(ui->actionLayer_SortDescending);
    sortActionGroup->setExclusive(true);

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
            &MainWindow::startServerLogin);

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
    loadLayers();
    setupMap();
    setupWeather();
    setupContextMenus();
    refreshSettings();
    setupToolbar();

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

    QSettings settings;
    bool visibility = settings.value("dock/visibility", true).toBool();
    ui->dockWidget->setVisible(visibility);
    ui->actionViewSidebar->setChecked(visibility);

    connect(ui->dockWidget,
            &OEDockWidget::closed,
            [this]() {
        QSettings().setValue("dock/visibility", false);
        ui->actionViewSidebar->setChecked(false);
    });

    connect(ui->actionViewSidebar,
            &QAction::toggled,
            [this](bool state) {
        QSettings().setValue("dock/visibility", state);
        ui->dockWidget->setVisible(state);
    });

    loadMarkers();

    m_serverInterface = new ServerInterface(this);

    connect(m_serverInterface,
        &ServerInterface::layersRequestFinished,
        this,
        &MainWindow::onServerInterfaceLayersRequestFinished);

    connect(m_serverInterface,
            &ServerInterface::authTokenRequestSucceeded,
            this,
            &MainWindow::onServerInterfaceAuthTokenRequestFinished);

    connect(m_serverInterface,
            &ServerInterface::authTokenRequestFailed,
            this,
            &MainWindow::onServerInterfaceAuthTokenRequestFailed);

    connect(m_serverInterface,
            &ServerInterface::workspacesRequestFinished,
            this,
            &MainWindow::onServerInterfaceWorkspacesRequestFinished);

    m_serverNetworkManager = new QNetworkAccessManager(this);

    /*
     * Default layer setup
     */
    m_defaultMarkerLayer = SlippyMapLayer::Ptr::create();
    m_defaultMarkerLayer->setName(tr("Layer1"));
    m_layerManager->addLayer(m_defaultMarkerLayer);
    m_layerManager->setDefaultLayer(m_defaultMarkerLayer);

    setWorkspaceDirty(false);
    disableDrawing();

    startServerLogin();
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
    m_contextMenu->addAction(ui->actionEdit_Cut);
    m_contextMenu->addAction(ui->actionEdit_Copy);
    m_contextMenu->addAction(ui->actionEdit_Paste);
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
    ui->actionView_Crosshairs->setChecked(crosshairs);
    connect(ui->actionView_Crosshairs,
            &QAction::toggled,
            [this](bool state) {
                QSettings().setValue("map/crosshairs", state);
                ui->slippyMap->setCrosshairsEnabled(state);
            });

    ui->slippyMap->setFocus(Qt::OtherFocusReason);
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

void MainWindow::setupToolbar()
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

    m_toolBarLatLonButton = new QPushButton();
    m_toolBarLatLonButton->setText(tr("Go"));
    m_toolBarLatLonButton->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Minimum);

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

    m_drawingActionGroup = new QActionGroup(this);
    m_drawingActionGroup->setExclusive(true);
    m_drawingActionGroup->addAction(ui->actionDrawMarker);
    m_drawingActionGroup->addAction(ui->actionDrawLine);
    m_drawingActionGroup->addAction(ui->actionDrawRectangle);
    m_drawingActionGroup->addAction(ui->actionDrawEllipse);
    m_drawingActionGroup->addAction(ui->actionDrawPolygon);
    m_drawingActionGroup->addAction(ui->actionObjectAddFiles);

    ui->toolBar->addAction(ui->actionFile_NewWorkspace);
    ui->toolBar->addAction(ui->actionFile_SaveWorkspace);
    ui->toolBar->addAction(ui->actionFile_OpenWorkspace);
    ui->toolBar->addSeparator();
    ui->toolBar->addAction(ui->actionDrawMarker);
    ui->toolBar->addAction(ui->actionDrawLine);
    ui->toolBar->addAction(ui->actionDrawRectangle);
    ui->toolBar->addAction(ui->actionDrawEllipse);
    ui->toolBar->addAction(ui->actionDrawPolygon);
    ui->toolBar->addAction(ui->actionObjectAddFiles);
    ui->toolBar->addSeparator();
    ui->toolBar->addWidget(m_strokeColorSelector);
    ui->toolBar->addWidget(m_fillColorSelector);
    ui->toolBar->addWidget(m_lineWidth);
    ui->toolBar->addWidget(m_strokeWidth);
    ui->toolBar->addSeparator();
    ui->toolBar->addWidget(m_toolBarLatitudeInput);
    ui->toolBar->addWidget(m_toolBarLongitudeInput);
    ui->toolBar->addAction(tr("Go"));
    ui->toolBar->addSeparator();
    ui->toolBar->addAction(ui->actionMapGpsAddSource);
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
////    connect(marker, &SlippyMapWidgetMarker::changed, this, &MainWindow::saveWorkspace);
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
        ui->actionDrawMarker->setChecked(false);
        ui->actionDrawLine->setChecked(false);
        ui->actionDrawRectangle->setChecked(false);
        ui->actionDrawPolygon->setChecked(false);
        ui->actionDrawEllipse->setChecked(false);
        break;
    case SlippyMapWidget::MarkerDrawing:
        ui->actionDrawMarker->setChecked(true);
        break;
    case SlippyMapWidget::PathDrawing:
        ui->actionDrawLine->setChecked(true);
        break;
    case SlippyMapWidget::RectDrawing:
        ui->actionDrawRectangle->setChecked(true);
        break;
    case SlippyMapWidget::EllipseDrawing:
        ui->actionDrawEllipse->setChecked(true);
        break;
    case SlippyMapWidget::PolygonDrawing:
        ui->actionDrawPolygon->setChecked(true);
    default:
        ui->slippyMap->setCursor(Qt::CrossCursor);
        break;
    }
}

void MainWindow::onSlippyMapLayerObjectActivated(const SlippyMapLayerObject::Ptr& object)
{
    static QMetaObject::Connection saveButtonConnection;

    if (m_selectedObject == object) return;

    auto *commonPropertyPage = new SlippyMapLayerObjectCommonPropertyPage(
        object, m_layerManager);

    connect(object.get(),
        &SlippyMapLayerObject::updated,
        commonPropertyPage,
        &SlippyMapLayerObjectPropertyPage::updateUi);

    QList<SlippyMapLayerObjectPropertyPage*> propertyPages;
    propertyPages.append(commonPropertyPage);

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

    ui->actionEdit_Copy->setEnabled(true);
    ui->actionEdit_Cut->setEnabled(true);
    ui->actionEdit_Delete->setEnabled(true);
    ui->actionEdit_Rename->setEnabled(true);
    ui->actionEdit_Properties->setEnabled(true);
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
        ui->actionEdit_Copy->setEnabled(false);
        ui->actionEdit_Cut->setEnabled(false);
        ui->actionEdit_Delete->setEnabled(false);
        ui->actionEdit_Rename->setEnabled(false);
        ui->actionEdit_Properties->setEnabled(false);
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

void MainWindow::showPropertyPage(const SlippyMapLayerObject::Ptr& object)
{
    static QMetaObject::Connection saveButtonConnection;
    QList<SlippyMapLayerObjectPropertyPage*> propertyPages;
    propertyPages.append(new SlippyMapLayerObjectCommonPropertyPage(object, m_layerManager));

    for (auto *propertyPage : object->propertyPages(object))
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

void MainWindow::onSlippyMapLayerObjectDoubleClicked(const SlippyMapLayerObject::Ptr& object) {
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
    if (m_databaseMode) {
        createWorkspace();
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

void MainWindow::saveWorkspace(const QString& fileName)
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

    int layerCount = 0;
    for (const auto& layer: m_layerManager->layers()) {
        if (layer->isEditable())
            layerCount++;
    }

    QDataStream out(&file);
    out.setVersion(QDataStream::Qt_5_15);

    out << QString("layers");
    out << layerCount;

    for (const auto& layer : m_layerManager->layers()) {
        if (layer->isEditable()) {
            out << layer->name();
            out << layer->description();
            out << layer->isVisible();
            out << layer->isEditable();
            out << layer->objects().count();

            for (const auto& object: layer->objects()) {
                //out << QMetaType::type(object->metaObject()->className());
                out << QString(object->metaObject()->className());
                qDebug() << "Saving object type:" << QString(object->metaObject()->className());
                object->serialize(out);
            }
        }
    }

    setWorkspaceDirty(false);
}

void MainWindow::openWorkspace()
{
    if (!closeWorkspace())
        return;

    // if connected to server, get the workspace selection
    // dialog again
    if (m_databaseMode) {
        m_serverInterface->requestWorkspaces();
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

            auto layer = SlippyMapLayer::Ptr::create();
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
                auto object = SlippyMapLayerObject::Ptr(
                        qobject_cast<SlippyMapLayerObject *>(o));

                // unserialize and add to layer
                object->unserialize(in);
                m_layerManager->addLayerObject(layer, object);
            }
        }
    }

    setWorkspaceDirty(false);
}

void MainWindow::newWorkspace()
{
    if (!closeWorkspace())
        return;

    // if connected to server, get the workspace selection
    // dialog again
    if (m_databaseMode) {
        m_serverInterface->requestWorkspaces();
        return;
    }

    setWindowTitle("Untitled.osm");
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

    auto marker = SlippyMapWidgetMarker::Ptr::create();
    marker->setLabel(gridId);
    marker->setPosition(QPointF(longitude, latitude));
    m_layerManager->addLayerObject(m_weatherLayer, marker);
    m_weatherLayer->setVisible(true);
    m_weatherLayer->setEditable(false);

    QString forecastZoneUrl = properties["forecastZone"].toString();
}

void MainWindow::onGpsDataProviderPositionUpdated(QString identifier, QPointF position, QHash<QString, QVariant> metadata)
{
    SlippyMapGpsMarker::Ptr marker;

    if (m_gpsMarkers.contains(identifier)) {
        marker = m_gpsMarkers[identifier];
        marker->setLabel(metadata["gps_label"].toString());
        marker->setPosition(position);
    }
    else {
        marker = SlippyMapGpsMarker::Ptr::create(position);
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
    SlippyMapGpsMarker::Ptr marker;
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
    showPropertyPage(m_selectedObject);
}

void MainWindow::onPluginLayerObjectProviderMarkerAdded(const SlippyMapLayerObject::Ptr& object)
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

    for (SlippyMapLayer::Ptr layer : m_layerManager->layers()) {
        for (const auto& object: layer->objects()) {
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
        auto *ptr = static_cast<SlippyMapLayerObject*>(index.internalPointer());
        for (const auto& layer: m_layerManager->layers()) {
            for (const auto& object: layer->objects()) {
                if (object == ptr)
                    showPropertyPage(object);
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

void MainWindow::on_actionDrawMarker_triggered()
{
    ui->slippyMap->setDrawMode(SlippyMapWidget::MarkerDrawing);
}

void MainWindow::on_actionImport_GPX_triggered()
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

void MainWindow::setWorkspaceDirty(bool dirty)
{
    QString fileName = m_workspaceFileName;
    if (fileName.isEmpty())
        fileName = "Untitled.osm";

    m_workspaceDirty = dirty;

    if (dirty) {
        ui->actionFile_SaveWorkspace->setEnabled(true);
        if (m_databaseMode)
            setWindowTitle(
                    tr("Online Workspace") + " - " + \
                    m_workspaceName + "*");
        else
            setWindowTitle(fileName + "*");
    }
    else {
        ui->actionFile_SaveWorkspace->setEnabled(false);
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

    //
    // set the selected layer to active
    //
    if (!index.parent().isValid()) {
        const auto& layer = m_layerManager->layers().at(index.row());
        m_layerManager->setActiveLayer(layer);
        if (layer->isEditable())
            enableDrawing();
        else
            disableDrawing();
    }
    //
    // set the selected object to active
    //
    else {
        auto *ptr = static_cast<SlippyMapLayerObject*>(index.internalPointer());
        for (const auto& layer: m_layerManager->layers()) {
            for (const auto& object: layer->objects()) {
                if (object == ptr)
                    ui->slippyMap->setActiveObject(object);
            }
        }
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

void MainWindow::startPolygonSelection()
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

void MainWindow::onAnimationTimerTimeout()
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

    ui->actionEdit_Redo->setEnabled(m_historyManager->redoCount() > 0);
    if (m_historyManager->redoCount() > 0)
        ui->actionEdit_Redo->setText(tr("Redo") + " " + m_historyManager->currentRedoDescription());
    ui->actionEdit_Undo->setEnabled(true);
    if (m_historyManager->undoCount() > 0)
        ui->actionEdit_Undo->setText(tr("Undo") + " " + m_historyManager->currentUndoDescription());
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
    ui->actionEdit_Undo->setEnabled(true);
    ui->actionEdit_Undo->setText(tr("Undo") + " " + description);
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
    ui->actionEdit_Paste->setEnabled(true);
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
    if (m_serverConnectionDialog == nullptr) {
        m_serverConnectionDialog = new ServerConnectionDialog(this);
        m_serverConnectionDialog->setWindowTitle("Login");
    }

    QSettings settings;

    QString username = settings.value("server/username", "").toString();
    QString password = settings.value("server/password", "").toString();

    m_serverConnectionDialog->setUsername(username);
    m_serverConnectionDialog->setPassword(password);

    if (!username.isEmpty())
        m_serverConnectionDialog->setRemember(true);

    int status = m_serverConnectionDialog->exec();

    if (status == QDialog::Accepted) {
        if (!closeWorkspace())
            return;

        setDatabaseMode(true);
        m_statusBarStatusLabel->setText(tr("Ready"));

        QString username = m_serverConnectionDialog->username();
        QString password = m_serverConnectionDialog->password();

        m_serverInterface->requestAuthToken(username, password);
    }
    else {
        close();
    }
}

void MainWindow::loadViewportData()
{
    if (!m_databaseMode) return;

    auto boundingBox = ui->slippyMap->boundingBoxLatLon();
    m_serverInterface->requestLayersForViewport(m_workspaceId, boundingBox);
}

void MainWindow::onServerInterfaceLayersRequestFinished()
{
    // only keep in memory that which we can see
//    for (const auto& layer: m_layerManager->layers()) {
//        m_layerManager->removeLayerObjects(layer);
//    }

    const auto& layers = m_serverInterface->layers();
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
            object->setLabel(objectdata.label);
            object->setDescription(objectdata.description);
            object->hydrateFromDatabase(objectdata.data, geom);
            object->setSynced(true);

            m_layerManager->addLayerObject(layer, object);
        }
    }
}

//void MainWindow::createWorkspace()
//{
//
//}

void MainWindow::createWorkspace()
{
    if (m_workspaceId.isNull()) {
        // todo: dispatch request to create workspace,
        //  it will then re-trigger this method to
        //  sync everything
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

            m_serverInterface->requestCreateWorkspace(workspace,
                [this, workspace]() {
                    m_workspaceId = workspace.id;
                    m_workspaceName = workspace.name;
                    setWindowTitle(m_workspaceName);
                }
            );
        }

    }
}

void MainWindow::processDatabaseUpdates()
{
    if (!m_databaseLayerDeleteList.isEmpty()) {
        for (const auto &layer: m_databaseLayerDeleteList) {
            for (const auto& object: layer->objects()) {
                qDebug() << "Processing object" << object->label() << "for layer" << layer->name();
                QNetworkRequest request;
                QString url = QString(OSM_SERVER_HOST "/objects/%1").arg(object->id().toString());
                request.setUrl(QUrl(url));
                request.setRawHeader("Authorization",
                                     QString("Bearer %1").arg(m_serverInterface->authToken()).toUtf8());

                ServerSyncRequest sync;
                sync.type = SyncRequestDelete;
                sync.request = request;
                sync.object = object;
                m_serverSyncRequestQueue.append(sync);
            }

            qDebug() << "Processing" << layer->name() << "for delete";
            QNetworkRequest request;
            QString url = QString(OSM_SERVER_HOST "/layers/%1").arg(layer->id().toString());
            request.setUrl(QUrl(url));
            request.setRawHeader("Authorization",
                                 QString("Bearer %1").arg(m_serverInterface->authToken()).toUtf8());

            ServerSyncRequest sync;
            sync.type = SyncRequestDelete;
            sync.request = request;
            sync.layer = layer;
            m_serverSyncRequestQueue.append(sync);
        }
    }

    if (!m_databaseLayerUpdateList.isEmpty()) {
        for (const auto& layer: m_databaseLayerUpdateList) {
            if (!layer->isEditable()) continue;

            QJsonObject json;

            if (layer->id().toString().isEmpty())
                layer->setId(QUuid::createUuid().toString());

            json["id"] = layer->id().toString();
            json["workspace_id"] = m_workspaceId.toString();
            json["name"] = layer->name();
            json["description"] = layer->description();
            json["order"] = layer->order();
            json["color"] = layer->color().name(QColor::HexArgb);

            QJsonDocument jsonDocument(json);
            QByteArray jsonData = jsonDocument.toJson();

            QNetworkRequest request;
            request.setUrl(QUrl(OSM_SERVER_HOST "/layers"));
            request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
            request.setRawHeader("Authorization",
                                 QString("Bearer %1").arg(m_serverInterface->authToken()).toUtf8());

            ServerSyncRequest sync;
            sync.type = SyncRequestPut;
            sync.request = request;
            sync.data = jsonData;
            sync.layer = layer;
            m_serverSyncRequestQueue.append(sync);
        }
    }

    if (!m_databaseObjectDeleteList.isEmpty()) {
        for (const auto& object: m_databaseObjectDeleteList) {
            qDebug() << "Processing object" << object->label() << "for delete";
            QNetworkRequest request;
            QString url = QString(OSM_SERVER_HOST "/objects/%1").arg(object->id().toString());
            request.setUrl(QUrl(url));
            request.setRawHeader("Authorization",
                                 QString("Bearer %1").arg(m_serverInterface->authToken()).toUtf8());

            ServerSyncRequest sync;
            sync.type = SyncRequestDelete;
            sync.request = request;
            sync.object = object;
            m_serverSyncRequestQueue.append(sync);
        }
    }

    if (!m_databaseObjectUpdateList.isEmpty()) {
        for (const auto& object: m_databaseObjectUpdateList) {
            QJsonObject json;

            if (object->id().toString().isEmpty()) {
                object->setId(QUuid::createUuid().toString());
            }

            const auto& layer = m_layerManager->layerForObject(object);

            json["id"] = object->id().toString();
            json["layer_id"] = layer->id().toString();
            json["label"] = object->label();
            json["description"] = object->description();
            json["type"] = QString(object->metaObject()->className());
            json["visible"] = object->isVisible();

            QJsonObject data;
            QString geom;
            object->saveToDatabase(data, geom);
            json["geom"] = geom;
            json["data"] = data;

            QNetworkRequest request;
            request.setUrl(QUrl(OSM_SERVER_HOST "/objects"));
            request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
            request.setRawHeader("Authorization",
                                 QString("Bearer %1").arg(m_serverInterface->authToken()).toUtf8());

            ServerSyncRequest sync;
            sync.type = SyncRequestPut;
            sync.object = object;
            sync.request = request;
            sync.data = QJsonDocument(json).toJson();

            m_serverSyncRequestQueue.append(sync);
        }
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
                qDebug() << "Starting DELETE request for layer/object update";
                auto *reply = m_serverNetworkManager->deleteResource(
                        sync.request);
                connect(reply,
                        &QNetworkReply::finished,
                        [this, reply, sync]() {
                            if (reply->error() != QNetworkReply::NoError) {
                                qCritical() << "DELETE request failed:" << reply->errorString();
                                QMessageBox::critical(
                                        this,
                                        tr("Server Error"),
                                        tr("Error processing request. Please try again."));
                                return;
                            }

                            qDebug() << "DELETE request finished successfully";
                            m_serverSyncRequestQueue.removeFirst();

                            if (!sync.layer.isNull()) {
                                m_databaseLayerDeleteList.removeOne(sync.layer);
                                sync.layer->setSynced(true);
                            }
                            if (!sync.object.isNull()) {
                                m_databaseObjectDeleteList.removeOne(sync.object);
                                sync.object->setSynced(true);
                            }

                            int val = m_serverRequestProgress->value();
                            m_serverRequestProgress->setValue(val + 1);
                            processDatabaseUpdateQueue();
                });
                break;
            }
            case SyncRequestPut: {
                qDebug() << "Starting PUT request for layer/object update";
                auto *reply = m_serverNetworkManager->put(
                        sync.request,
                        sync.data);
                connect(reply,
                        &QNetworkReply::finished,
                        [this, reply, sync]() {
                            if (reply->error() != QNetworkReply::NoError) {
                                qCritical() << "PUT request failed:" << reply->errorString();
                                QMessageBox::critical(
                                        this,
                                        tr("Server Error"),
                                        tr("Error processing request. Please try again."));
                                return;
                            }

                            qDebug() << "PUT request finished successfully";
                            m_serverSyncRequestQueue.removeFirst();

                            if (!sync.layer.isNull()) {
                                m_databaseLayerUpdateList.removeOne(sync.layer);
                                sync.layer->setSynced(true);
                            }
                            if (!sync.object.isNull()) {
                                m_databaseObjectUpdateList.removeOne(sync.object);
                                sync.object->setSynced(true);
                            }

                            int val = m_serverRequestProgress->value();
                            m_serverRequestProgress->setValue(val + 1);
                            processDatabaseUpdateQueue();
                        });
                break;
            }
        }
    }
    else {
        m_serverRequestProgress->setVisible(false);
        setWorkspaceDirty(false);
    }
}

void MainWindow::onServerInterfaceAuthTokenRequestFinished()
{
    QSettings settings;

    if (m_serverConnectionDialog->remember()) {
        settings.setValue("server/username", m_serverConnectionDialog->username());
        settings.setValue("server/password", m_serverConnectionDialog->password());
    }
    else {
        settings.setValue("server/username", "");
        settings.setValue("server/password", "");
        m_serverConnectionDialog->setUsername("");
        m_serverConnectionDialog->setPassword("");
    }

    m_serverInterface->requestWorkspaces();
}

void MainWindow::onServerInterfaceAuthTokenRequestFailed()
{
    QMessageBox::critical(this,
                          tr("Authentication Failure"),
                          tr("Your credentials were invalid. Please try again."));
    startServerLogin();
}

void MainWindow::disableDrawing()
{
    m_drawingActionGroup->setEnabled(false);
}

void MainWindow::enableDrawing()
{
    m_drawingActionGroup->setEnabled(true);
}

void MainWindow::onServerInterfaceWorkspacesRequestFinished()
{
    WorkspaceSelectionDialog dlg(this);

    QMap<QUuid,QString> workspaces;
    for (const auto& workspace: m_serverInterface->workspaces()) {
        workspaces[workspace.id] = workspace.name;
    }

    dlg.setWorkspaceList(workspaces);
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

            QJsonObject root;
            root["id"] = id.toString();
            root["name"] = name;
            root["description"] = description;

            QJsonDocument doc(root);
            QByteArray data = doc.toJson();

            QNetworkRequest request;
            request.setUrl(QUrl(OSM_SERVER_HOST "/workspaces"));
            request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
            request.setRawHeader("Authorization",
                                 QString("Bearer %1").arg(m_serverInterface->authToken()).toUtf8());

            QNetworkReply *reply = m_serverNetworkManager->put(request, data);
            connect(reply,
                    &QNetworkReply::finished,
                    [this, id, reply, name]() {
                            if (reply->error() != QNetworkReply::NoError) {
                                qCritical() << "Create workspace request failed";
                                qCritical() << reply->errorString();
                                QMessageBox::critical(
                                        this,
                                        tr("Server Error"),
                                        tr("Error processing request. Please try again."));
                                return;
                            }

                            auto body = QJsonDocument::fromJson(reply->readAll());
                            auto root = body.object();

                            if (root["status"] != "ok") {
                                qCritical() << "Server error" << root["reason"];
                                QMessageBox::critical(
                                        this,
                                        tr("Server Error"),
                                        root["reason"].toString());
                                return;
                            }

                            m_workspaceId = id;
                            m_workspaceName = name;
                            setWindowTitle(
                                    tr("Online Workspace") + " - " + \
                                    m_workspaceName);
                            loadViewportData();
                            setWorkspaceDirty(false);
                        });

        }
        else {
            m_workspaceId = dlg.existingWorkspaceId();
            m_workspaceName = workspaces[m_workspaceId];
            setWindowTitle(
                    tr("Online Workspace") + " - " + \
                    m_workspaceName);
            loadViewportData();
            setWorkspaceDirty(false);
        }
    }
}

void MainWindow::setDatabaseMode(bool databaseMode)
{
    m_databaseMode = databaseMode;
    ui->actionFile_SaveWorkspaceAs->setEnabled(!databaseMode);
}

void MainWindow::setWindowTitle(const QString &title)
{
    QWidget::setWindowTitle(tr("OSMExplorer") + " - " + title);
}

void MainWindow::setupMenuBar()
{
    m_mainMenuBar = new QMenuBar(this);

    m_fileMenu = new QMenu(tr("&File"));
    m_editMenu = new QMenu(tr("&Edit"));
    m_layerMenu = new QMenu(tr("&Layer"));
    m_objectMenu = new QMenu(tr("&Object"));
    m_drawMenu = new QMenu(tr("&Draw"));
    m_viewMenu = new QMenu(tr("&View"));
    m_toolsMenu = new QMenu(tr("&Tools"));
    m_helpMenu = new QMenu(tr("&Help"));

    m_exitAction = new QAction(tr("E&xit"));
    m_exitAction->setShortcut(QKeySequence::fromString("Ctrl+Q"));

    m_newWorkspaceAction = new QAction(tr("&New Workspace"));
    m_newWorkspaceAction->setShortcut(QKeySequence::fromString("Ctrl+N"));
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
