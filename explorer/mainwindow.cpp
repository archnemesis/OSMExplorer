#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "slippymapwidget.h"
#include "slippymapwidgetlayer.h"
#include "markerdialog.h"
#include "markerlistitemwidget.h"
#include "directionlistitemwidget.h"
#include "settingsdialog.h"
#include "defaults.h"
#include "nmeaseriallocationdataprovider.h"
#include "gpssourcedialog.h"
#include "textlogviewerform.h"
#include "slippymapwidgetmarkermodel.h"
#include "explorerplugininterface.h"

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

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    loadPlugins();

    ui->setupUi(this);
    //connect(ui->splitter, &QSplitter::splitterMoved, this, &MainWindow::onSplitterMoved);

    m_net = new QNetworkAccessManager();
    connect(m_net, &QNetworkAccessManager::finished, this, &MainWindow::onNetworkRequestFinished);

//    // set style
//    qApp->setStyle(QStyleFactory::create("Fusion"));
//    // increase font size for better reading
//    QFont defaultFont = QApplication::font();
//    defaultFont.setPointSize(defaultFont.pointSize()+2);
//    qApp->setFont(defaultFont);
//    // modify palette to dark
//    QPalette darkPalette;
//    darkPalette.setColor(QPalette::Window,QColor(53,53,53));
//    darkPalette.setColor(QPalette::WindowText,Qt::white);
//    darkPalette.setColor(QPalette::Disabled,QPalette::WindowText,QColor(127,127,127));
//    darkPalette.setColor(QPalette::Base,QColor(42,42,42));
//    darkPalette.setColor(QPalette::AlternateBase,QColor(66,66,66));
//    darkPalette.setColor(QPalette::ToolTipBase,Qt::white);
//    darkPalette.setColor(QPalette::ToolTipText,Qt::white);
//    darkPalette.setColor(QPalette::Text,Qt::white);
//    darkPalette.setColor(QPalette::Disabled,QPalette::Text,QColor(127,127,127));
//    darkPalette.setColor(QPalette::Dark,QColor(35,35,35));
//    darkPalette.setColor(QPalette::Shadow,QColor(20,20,20));
//    darkPalette.setColor(QPalette::Button,QColor(53,53,53));
//    darkPalette.setColor(QPalette::ButtonText,Qt::white);
//    darkPalette.setColor(QPalette::Disabled,QPalette::ButtonText,QColor(127,127,127));
//    darkPalette.setColor(QPalette::BrightText,Qt::red);
//    darkPalette.setColor(QPalette::Link,QColor(42,130,218));
//    darkPalette.setColor(QPalette::Highlight,QColor(42,130,218));
//    darkPalette.setColor(QPalette::Disabled,QPalette::Highlight,QColor(80,80,80));
//    darkPalette.setColor(QPalette::HighlightedText,Qt::white);
//    darkPalette.setColor(QPalette::Disabled,QPalette::HighlightedText,QColor(127,127,127));

//    qApp->setPalette(darkPalette);

    QSettings settings;

    if (settings.contains("view/sidebarWidth")) {
        double ratio = settings.value("view/sidebarWidth").toDouble();
        int sidebar_width = (int)((double)width() * ratio);
        int map_width = width() - sidebar_width;
        QList<int> widths;
        widths.append(sidebar_width);
        widths.append(map_width);
        //ui->splitter->setSizes(widths);
    }

    if (settings.contains("view/sidebarVisible")) {
        //ui->toolBox->setVisible(settings.value("view/sidebarVisible").toBool());
        ui->actionViewSidebar->setChecked(true);
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
        ui->slippyMap->addLayer(layer);
        m_layers.append(layer);

        QAction *layerShowHide = new QAction();
        layerShowHide->setCheckable(true);
        layerShowHide->setChecked(visible);
        layerShowHide->setText(name);
        connect(layerShowHide, &QAction::triggered, [=]() {
            layer->setVisible(layerShowHide->isChecked());
            ui->slippyMap->update();
        });
        ui->menuFileLayers->addAction(layerShowHide);
    }
    settings.endArray();

    if (layerCount == 0) {
        QMessageBox::information(
                    this,
                    tr("OSMExplorer"),
                    tr("There are no layers configured. To get started, add one or more layers in Settings."),
                    QMessageBox::Ok);
        on_actionFileSettings_triggered();
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
    connect(ui->slippyMap, &SlippyMapWidget::searchTextChanged, this, &MainWindow::onSlippyMapSearchTextChanged);

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

    if (settings.contains("view/windowWidth") && settings.contains("view/windowHeight")) {
        int width = settings.value("view/windowWidth").toInt();
        int height = settings.value("view/windowHeight").toInt();
        resize(width, height);
    }

    m_markerModel = new SlippyMapWidgetMarkerModel();
    m_markerModelGroup_myMarkers = new SlippyMapWidgetMarkerGroup(tr("My Places"));
    m_markerModelGroup_gpsMarkers = new SlippyMapWidgetMarkerGroup(tr("GPS Sources"));
    m_markerModelGroup_aprsDotFiMarkers = new SlippyMapWidgetMarkerGroup(tr("aprs.fi"));
    m_markerModel->addMarkerGroup(m_markerModelGroup_myMarkers);
    m_markerModel->addMarkerGroup(m_markerModelGroup_gpsMarkers);

    qDebug() << "Loading plugin marker groups...";

    for (ExplorerPluginInterface *plugin : m_plugins) {
        QList<SlippyMapWidgetMarkerGroup *> pluginGroups = plugin->markerGroupList();
        if (pluginGroups.count() > 0) {
            for (SlippyMapWidgetMarkerGroup *group : pluginGroups) {
                m_markerModel->addMarkerGroup(group);
            }
        }
    }

    ui->tvwMarkers->setModel(m_markerModel);
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
            m_plugins.append(qobject_cast<ExplorerPluginInterface*>(plugin));
        }
    }
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

void MainWindow::onSlippyMapMarkerAdded(SlippyMapWidgetMarker *marker)
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

void MainWindow::onSlippyMapMarkerDeleted(SlippyMapWidgetMarker *marker)
{
    if (m_markerListItemMap.contains(marker)) {
        delete m_markerListItemMap[marker];
        m_markerListItemMap.remove(marker);
    }
}

void MainWindow::onSlippyMapMarkerUpdated(SlippyMapWidgetMarker *marker)
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

void MainWindow::onSlippyMapSearchTextChanged(const QString &text)
{
    qDebug() << "Search Text:" << text;
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
    (void)pos;
    (void)index;
    m_saveSplitterPosTimer->stop();
    m_saveSplitterPosTimer->start();
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

void MainWindow::onDataProviderAprsFiPositionUpdated(QString identifier, QPointF position, QHash<QString, QVariant> metadata)
{
//    SlippyMapWidgetMarker *marker;

//    if (m_dataProviderAprsFiMarkers.contains(identifier)) {
//        marker = m_dataProviderAprsFiMarkers[identifier];
//        marker->setLabel(identifier);
//        marker->setPosition(position);
//    }
//    else {
//        marker = new SlippyMapWidgetMarker(position);
//        marker->setLabel(identifier);
//        m_markerModelGroup_aprsDotFiMarkers->addMarker(marker);
//        m_dataProviderAprsFiMarkers[identifier] = marker;
//        ui->slippyMap->addMarker(marker);
//    }

//    ui->slippyMap->update();
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
        m_gpsMarkers[identifier] = marker;
        m_markerModelGroup_gpsMarkers->addMarker(marker);
        ui->slippyMap->addMarker(marker);
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
    SlippyMapWidgetMarker *marker = MarkerDialog::getNewMarker(this, tr("New Marker"));
    if (marker != nullptr) {
        ui->slippyMap->addMarker(marker);
    }
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

            QSslConfiguration config = QSslConfiguration::defaultConfiguration();
            config.setProtocol(QSsl::TlsV1_2);
            QNetworkRequest request(req);
            request.setSslConfiguration(config);
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
