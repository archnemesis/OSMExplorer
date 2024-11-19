#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QHash>
#include <QPalette>

#include <SlippyMap/SlippyMapWidget.h>
#include <SlippyMap/SlippyMapWidgetMarker.h>

#include "Weather/NationalWeatherServiceInterface.h"
#include "nmeaseriallocationdataprovider.h"
#include "Application/HistoryManager.h"
#include "Map/SlippyMapGpsMarker.h"
#include "Network/ServerInterface.h"
#include "GeoCoding/GeoCodingInterface.h"
#include "GeoCoding/GeoCodingListModel.h"

class SlippyMapLayerProxyModel;

namespace Ui {
class MainWindow;
}

class DirectionListItemWidget;
class ExplorerPluginInterface;
class GeoCodingListModel;
class HistoryManager;
class LocationDataProvider;
class MapDataImportDialog;
class ObjectBrowserDialog;
class QAction;
class QActionGroup;
class QComboBox;
class QCompleter;
class QLabel;
class QListWidgetItem;
class QMenu;
class QMessageBox;
class QNetworkAccessManager;
class QNetworkReply;
class QProgressBar;
class QSpinBox;
class ServerConnectionDialog;
class SettingsDialog;
class SlippyMapGpsMarker;
class SlippyMapLayerObjectPropertyPage;
class TextLogViewerForm;
class WeatherForecastWindow;
class WeatherStationMarker;

namespace color_widgets {
    class ColorSelector;
}

namespace SlippyMap
{
    class SlippyMapLayer;
    class SlippyMapLayerPolygon;
    class SlippyMapLayerPath;
};

using namespace SlippyMap;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

    void setWindowTitle(const QString& title);

protected slots:
    void onSlippyMapCenterChanged(double latitude, double longitude);
    void onSlippyMapZoomLevelChanged(int zoom);
    void onSlippyMapTileRequestStarted();
    void onSlippyMapTileRequestFinished();
    void onSlippyMapTileRequestPending(int tiles);
    void onSlippyMapCursorPositionChanged(double latitude, double longitude);
    void onSlippyMapCursorEntered();
    void onSlippyMapCursorLeft();
    void onSlippyMapContextMenuActivated(double latitude, double longitude);
    void onSlippyMapSearchTextChanged(const QString &text);
    void onSlippyMapContextMenuRequested(const QPoint& point);
    void onSlippyMapRectSelected(QRect rect);
    void onSlippyMapPolygonSelected(const QList<QPointF>& points);
    void onSlippyMapPathSelected(const QList<QPointF>& points);
    void onSlippyMapPointSelected(const QPointF& point);
    void onSlippyMapDrawModeChanged(SlippyMapWidget::DrawMode mode);
    void onSlippyMapLayerObjectActivated(const SlippyMapLayerObject::Ptr& object);
    void onSlippyMapLayerObjectDeactivated(const SlippyMapLayerObject::Ptr& object);
    void onSlippyMapLayerObjectDoubleClicked(const SlippyMapLayerObject::Ptr& object);
    void onSlippyMapLayerObjectUpdated(const SlippyMapLayerObject::Ptr& object);
    void onSlippyMapLayerObjectWasDragged(const SlippyMapLayerObject::Ptr& object);
    void onSlippyMapDragFinished();

    void onWeatherService_stationListReady(
            const QList<NationalWeatherServiceInterface::WeatherStation>& stations);
    void onWeatherService_forecastReady(
            const NationalWeatherServiceInterface::Forecast12Hr& forecast);

    void saveWorkspace();
    void openWorkspace();
    void newWorkspace();
    void onActionFileCloseWorkspaceTriggered();
    void weatherNetworkAccessManager_onRequestFinished(QNetworkReply *reply);
    void onGpsDataProviderPositionUpdated(QString identifier, QPointF position, QHash<QString,QVariant> metadata);
    void onGpsDataProviderSatellitesUpdated(
            QString identifier,
            const QList<NmeaSerialLocationDataProvider::SatelliteStatus>& satellites,
            QHash<QString, QVariant> metadata);
    void onTvwMarkersContextMenuRequested(const QPoint& point);
    void onTvwMarkersClicked(const QModelIndex& index);
    void advanceLayerAnimationFrame();
    void centerMapOnContextMenuPosition();
    void clearSelectedLayer();
    void copyActiveObject();
    void createMarkerAtContextMenuPosition();
    void createMarkerAtCurrentPosition();
    void createMarkerAtPosition(const QPointF& position);
    void createNewLayer();
    void cutActiveObject();
    void deleteActiveLayer();
    void deleteActiveObject();
    void deleteObject(const SlippyMapLayerObject::Ptr& object);
    void deleteSelectedLayer();
    void disableDrawing();
    void enableDrawing();
    void getGeoCodedLocation();
    void loadViewportData();
    void openOrCreateWorkspace(const QList<ServerInterface::Workspace>& workspaces);
    void pasteObject();
    void redo();
    void redoHistoryCleared();
    void refreshSettings();
    void renameActiveLayer();
    void saveObject(const SlippyMapLayerObject::Ptr& object);
    void saveWindowSize();
    void showActiveObjectPropertyPage();
    void showAddGpsSourceDialog();
    void showGpsLogDialog();
    void showObjectBrowserDialog();
    void showSettingsDialog();
    void startPolygonSelection();
    void startServerLogin();
    void undo();
    void undoEventAdded(HistoryManager::HistoryEvent event);
    void activateLayerAtIndex(const QModelIndex &index);

    // todo: finish renaming these
    void on_actionDrawLine_triggered();
    void on_actionDrawRectangle_triggered();
    void on_actionDrawEllipse_triggered();
    void on_actionDrawMarker_triggered();
    void on_actionImport_GPX_triggered();
    void on_actionMarkerImport_triggered();
    void on_actionToolsOSMImport_triggered();
    void on_geoCodingInterface_locationFound(QList<GeoCodingInterface::GeoCodedAddress> locations);

protected:
    enum AnimationState {
        Forward,
        Reverse,
        Paused
    };

    enum SyncRequestType {
        SyncRequestPut,
        SyncRequestDelete
    };

    enum SyncCompleteAction {
        SyncCompleteNoAction,
        SyncCompleteQuit,
        SyncCompleteCloseWorkspace,
        SyncCompleteOpenWorkspace
    };

    struct ServerSyncRequest {
        SyncRequestType type;
        QNetworkRequest request;
        QByteArray data;
        SlippyMapLayer::Ptr layer;
        SlippyMapLayerObject::Ptr object;
    };

    class Clipboard {
    public:
        enum EntryType {
            NoEntry,
            Layer,
            Object
        };

        enum ActionType {
            NoAction,
            Cut,
            Copy
        };

        EntryType type = NoEntry;
        ActionType action = NoAction;
        SlippyMapLayer::Ptr layer;
        SlippyMapLayerObject::Ptr object;
    };

    bool closeWorkspace();
    void loadMarkers();
    void loadLayers();
    void loadPluginLayers();
    void loadStartupSettings();
    void resizeEvent(QResizeEvent *event) override;
    void saveTileLayerSettings();
    void setWorkspaceDirty(bool dirty);
    void setupContextMenus();
    void setupMap();
    void setupToolbar();
    void setupWeather();
    void showPropertiesDialog(const SlippyMapLayerObject::Ptr& object);
    void updateRecentFileList();
    void createUndoAddObject(const QString& description, SlippyMapLayer::Ptr layer, const SlippyMapLayerObject::Ptr& object);
    void createUndoModifyObject(const QString& description, const SlippyMapLayerObject::Ptr& object);
    void createUndoDeleteObject(const QString &description,
                                const SlippyMapLayer::Ptr& layer,
                                const SlippyMapLayerObject::Ptr& object);
    void createUndoAddLayer(const QString& description, SlippyMapLayer::Ptr layer);
    void createUndoDeleteLayer(const QString &description, SlippyMapLayer::Ptr layer);
    void closeEvent(QCloseEvent *event) override;
    void processDatabaseUpdates();
    void processDatabaseUpdateQueue();
    void setDatabaseMode(bool databaseMode);
    void loadWorkspaces();

private:
    Ui::MainWindow *ui;
    AnimationState m_animationState = Forward;
    Clipboard m_clipBoard;
    DirectionListItemWidget *m_currentRouteListItemWidget = nullptr;
    GeoCodingInterface *m_geoCodingInterface;
    GeoCodingListModel *m_geoCodingListModel;
    HistoryManager *m_historyManager;
    MapDataImportDialog *m_importDialog = nullptr;
    NationalWeatherServiceInterface *m_weatherService = nullptr;
    ObjectBrowserDialog *m_objectBrowser = nullptr;
    SyncCompleteAction m_syncCompleteAction;
    QAction *m_addMarkerAction = nullptr;
    QAction *m_centerMapAction = nullptr;
    QAction *m_clearLayerAction = nullptr;
    QAction *m_coordAction = nullptr;
    QAction *m_copyCoordinatesAction = nullptr;
    QAction *m_copyLatitudeAction = nullptr;
    QAction *m_copyLongitudeAction = nullptr;
    QAction *m_deleteLayerAction = nullptr;
    QAction *m_deleteObjectAction = nullptr;
    QAction *m_deleteShapeAction = nullptr;
    QAction *m_editShapeAction = nullptr;
    QAction *m_getForecastHereAction = nullptr;
    QAction *m_markerDeleteAction = nullptr;
    QAction *m_objectPropertiesAction = nullptr;
    QAction *m_layerVisibilityAction = nullptr;
    QAction *m_markerLockedAction = nullptr;
    QAction *m_newLayerAction = nullptr;
    QAction *m_renameLayerAction = nullptr;
    QAction *m_zoomInHereMapAction = nullptr;
    QAction *m_zoomOutHereMapAction = nullptr;
    QActionGroup *m_drawingActionGroup = nullptr;
    QCompleter *m_locationSearchCompleter = nullptr;
    QHash<QString,SlippyMapGpsMarker::Ptr> m_gpsMarkers;
    QLabel *m_statusBarGpsStatusLabel;
    QLabel *m_statusBarPositionLabel;
    QLabel *m_statusBarStatusLabel;
    QLineEdit *m_toolBarLatitudeInput;
    QLineEdit *m_toolBarLongitudeInput;
    QLineEdit *m_toolBarGeoCodingInput;
    QList<ExplorerPluginInterface*> m_plugins;
    QList<LocationDataProvider*> m_gpsProviders;
    QList<QString> m_recentFileList;
    QList<SlippyMapWidgetLayer*> m_layers;
    QList<QAction*> m_layerShowHideActions;
    QList<ServerSyncRequest> m_serverSyncRequestQueue;
    QList<SlippyMapLayer::Ptr> m_databaseLayerDeleteList;
    QList<SlippyMapLayer::Ptr> m_databaseLayerUpdateList;
    QList<SlippyMapLayerObject::Ptr> m_databaseObjectDeleteList;
    QList<SlippyMapLayerObject::Ptr> m_databaseObjectUpdateList;
    QList<SlippyMapWidgetMarker::Ptr> m_loadedMarkers;
    QList<SlippyMapWidgetMarker::Ptr> m_weatherStationMarkers;
    QListWidgetItem *m_currentRouteListItem = nullptr;
    QMap<SlippyMapWidgetMarker*,QListWidgetItem*> m_markerListItemMap;
    QMenu *m_contextMenu = nullptr;
    QMenu *m_treeViewMenu = nullptr;
    QMessageBox *m_loadingDialog = nullptr;
    QNetworkAccessManager *m_weatherNetworkAccessManager;
    QNetworkAccessManager *m_serverNetworkManager;
    QPalette m_defaultPalette;
    QPoint m_contextMenuLocation;
    QPointF m_contextMenuPoint;
    QPointF m_slippyContextMenuLocation;
    QProgressBar *m_serverRequestProgress;
    QPushButton *m_currentLocationButton;
    QPushButton *m_toolBarLatLonButton;
    QPushButton *m_zoomInButton;
    QPushButton *m_zoomOutButton;
    QSpinBox *m_lineWidth;
    QSpinBox *m_strokeWidth;
    QString m_workspaceFileName;
    QString m_workspaceName;
    QTimer *m_animationTimer = nullptr;
    QTimer *m_saveWindowSizeTimer = nullptr;
    QUuid m_workspaceId;
    QUuid m_userId;
    ServerConnectionDialog *m_serverConnectionDialog = nullptr;
    ServerInterface *m_serverInterface = nullptr;
    SettingsDialog *m_settingsDialog = nullptr;
    SlippyMapLayer::Ptr m_defaultMarkerLayer = nullptr;
    SlippyMapLayer::Ptr m_gpsMarkerLayer = nullptr;
    SlippyMapLayer::Ptr m_weatherLayer;
    SlippyMapLayer::Ptr m_databaseLayer;
    SlippyMapLayer::Ptr m_searchMarkerLayer;
    SlippyMapLayerManager *m_layerManager = nullptr;
    SlippyMapLayerProxyModel *m_layerManagerProxy = nullptr;
    SlippyMapLayerObject::WeakPtr m_selectedObject;
    SlippyMapLayerObject::Ptr m_selectedObjectCopy = nullptr;
    SlippyMapLayerObjectPropertyPage *m_selectedObjectPropertyPage = nullptr;
    SlippyMapLayerPolygon *m_forecastZonePolygon;
    SlippyMapWidget::LineSet *m_currentRouteLineSet = nullptr;
    TextLogViewerForm *m_nmeaLog = nullptr;
    WeatherForecastWindow *m_weatherForecastWindow = nullptr;
    WeatherStationMarker *m_weatherStationMarker;
    bool m_databaseMode = false;
    bool m_workspaceDirty = true;
    color_widgets::ColorSelector *m_fillColorSelector;
    color_widgets::ColorSelector *m_strokeColorSelector;
    int m_requestCount = 0;

};

#endif // MAINWINDOW_H
