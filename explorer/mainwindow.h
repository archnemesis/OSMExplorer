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

class WeatherStationMarker;

namespace Ui {
class MainWindow;
}

class DirectionListItemWidget;
class ExplorerPluginInterface;
class LocationDataProvider;
class MapDataImportDialog;
class QAction;
class QComboBox;
class QLabel;
class QListWidgetItem;
class QMenu;
class QMessageBox;
class QNetworkAccessManager;
class QNetworkReply;
class QSpinBox;
class SettingsDialog;
class SlippyMapLayerObjectPropertyPage;
class TextLogViewerForm;
class WeatherForecastWindow;
class SlippyMapGpsMarker;
class HistoryManager;

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

protected:
    bool closeWorkspace();
    void loadMarkers();
    void loadLayers();
    void loadPluginLayers();
    void loadStartupSettings();
    void resizeEvent(QResizeEvent *event) override;
    void saveLayers();
    void saveWorkspaceToDatabase();
    void saveWorkspace(QString fileName);
    void setWorkspaceDirty(bool dirty);
    void setupContextMenus();
    void setupMap();
    void setupToolbar();
    void setupWeather();
    void showPropertyPage(const SlippyMapLayerObject::Ptr& object);
    void updateRecentFileList();
    void createUndoAddObject(const QString& description, SlippyMapLayer::Ptr layer, const SlippyMapLayerObject::Ptr& object);
    void createUndoModifyObject(const QString& description, const SlippyMapLayerObject::Ptr& object);
    void createUndoDeleteObject(const QString &description,
                                const SlippyMapLayer::Ptr& layer,
                                const SlippyMapLayerObject::Ptr& object);
    void createUndoAddLayer(const QString& description, SlippyMapLayer::Ptr layer);
    void createUndoDeleteLayer(const QString &description, SlippyMapLayer::Ptr layer);
    void closeEvent(QCloseEvent *event) override;

private:
    enum AnimationState {
        Forward,
        Reverse,
        Paused
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

    Ui::MainWindow *ui;
    AnimationState m_animationState = Forward;
    Clipboard m_clipBoard;
    DirectionListItemWidget *m_currentRouteListItemWidget = nullptr;
    HistoryManager *m_historyManager;
    MapDataImportDialog *m_importDialog = nullptr;
    NationalWeatherServiceInterface *m_weatherService = nullptr;
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
    QAction *m_markerVisibilityAction = nullptr;
    QAction *m_markerLockedAction = nullptr;
    QAction *m_newLayerAction = nullptr;
    QAction *m_renameLayerAction = nullptr;
    QAction *m_zoomInHereMapAction = nullptr;
    QAction *m_zoomOutHereMapAction = nullptr;
    QHash<QString,SlippyMapGpsMarker::Ptr> m_gpsMarkers;
    QLabel *m_statusBarGpsStatusLabel;
    QLabel *m_statusBarPositionLabel;
    QLabel *m_statusBarStatusLabel;
    QLineEdit *m_toolBarLatitudeInput;
    QLineEdit *m_toolBarLongitudeInput;
    QList<ExplorerPluginInterface*> m_plugins;
    QList<LocationDataProvider*> m_gpsProviders;
    QList<QString> m_recentFileList;
    QList<SlippyMapWidgetLayer*> m_layers;
    QList<QAction*> m_layerShowHideActions;
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
    QPalette m_defaultPalette;
    QPoint m_contextMenuLocation;
    QPointF m_contextMenuPoint;
    QPointF m_slippyContextMenuLocation;
    QPushButton *m_currentLocationButton;
    QPushButton *m_toolBarLatLonButton;
    QPushButton *m_zoomInButton;
    QPushButton *m_zoomOutButton;
    QSpinBox *m_lineWidth;
    QSpinBox *m_strokeWidth;
    QString m_workspaceFileName;
    QTimer *m_animationTimer = nullptr;
    QTimer *m_saveWindowSizeTimer = nullptr;
    ServerInterface *m_serverInterface = nullptr;
    SettingsDialog *m_settingsDialog = nullptr;
    SlippyMapLayer::Ptr m_defaultMarkerLayer = nullptr;
    SlippyMapLayer::Ptr m_gpsMarkerLayer = nullptr;
    SlippyMapLayer::Ptr m_weatherLayer;
    SlippyMapLayer::Ptr m_databaseLayer;
    SlippyMapLayerManager *m_layerManager = nullptr;
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

protected slots:
    void onSlippyMapCenterChanged(double latitude, double longitude);
    void onSlippyMapZoomLevelChanged(int zoom);
    void onSlippyMapTileRequestStarted();
    void onSlippyMapTileRequestFinished();
    void onSlippyMapCursorPositionChanged(double latitude, double longitude);
    void onSlippyMapCursorEntered();
    void onSlippyMapCursorLeft();
    void onSlippyMapContextMenuActivated(double latitude, double longitude);
    void onSlippyMapSearchTextChanged(const QString &text);
    void onSlippyMapContextMenuRequested(const QPoint& point);
    void onSlippyMapRectSelected(QRect rect);
    void onSlippyMapPolygonSelected(const QList<QPointF>& points);
    void onSlippyMapPathSelected(const QList<QPointF>& points);
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

    void onActionFileSaveWorkspaceTriggered();
    void onActionFileOpenWorkspaceTriggered();
    void onActionFileCloseWorkspaceTriggered();
    void weatherNetworkAccessManager_onRequestFinished(QNetworkReply *reply);
    void onGpsDataProviderPositionUpdated(QString identifier, QPointF position, QHash<QString,QVariant> metadata);
    void onGpsDataProviderSatellitesUpdated(
            QString identifier,
            const QList<NmeaSerialLocationDataProvider::SatelliteStatus>& satellites,
            QHash<QString, QVariant> metadata);
    void onTvwMarkersContextMenuRequested(const QPoint& point);
    void onTvwMarkersClicked(const QModelIndex& index);
    void showActiveObjectPropertyPage();
    void onPluginLayerObjectProviderMarkerAdded(const SlippyMapLayerObject::Ptr& object);
    void createMarkerAtContextMenuPosition();
    void createMarkerAtCurrentPosition();
    void createMarkerAtPosition(const QPointF& position);
    void onDeleteMarkerActionTriggered();
    void onEditMarkerActionTriggered();
    void onCenterMapActionTriggered();
    void setDarkModeEnabled(bool enabled);
    void onEditShapeActionTriggered();
    void startPolygonSelection();
    void onAnimationTimerTimeout();
    void undo();
    void redo();
    void undoEventAdded(HistoryManager::HistoryEvent event);
    void redoHistoryCleared();
    void cutActiveObject();
    void copyActiveObject();
    void pasteObject();
    void connectToDatabase();
    void loadViewportData();
    void onServerInterfaceLayersRequestFinished();

    /**
     * @brief Save window size after finish moving.
     */
    void onWindowSizeTimerTimeout();

    /**
     * @brief Refresh settings that would have immediate impact.
     */
    void refreshSettings();

    /**
     * @brief Ask user for a layer name and create a new layer.
     */
    void createNewLayer();

    /**
     * @brief Delete the selected layer from the context menu (ask first).
     */
    void deleteSelectedLayer();

    /**
     * @brief Delete the active layer from the menu bar.
     */
    void deleteActiveLayer();

    /**
     * @brief Clear the active layer of all objects.
     */
    void clearSelectedLayer();

    /**
     * @brief Delete the active marker.
     */
    void deleteActiveObject();

    /**
     * @brief Renames the active layer.
     */
    void renameActiveLayer();

private slots:
    void on_actionNewMarker_triggered();
    void on_actionViewSidebar_toggled(bool arg1);
    void on_actionViewClearRoute_triggered();
    void on_actionFileSettings_triggered();
    void on_actionMapGpsAddSource_triggered();
    void on_actionViewGpsLog_triggered();
    void on_tvwMarkers_activated(const QModelIndex &index);
    void on_tvwMarkers_clicked(const QModelIndex &index);
    void on_actionDrawLine_triggered();
    void on_actionDrawRectangle_triggered();
    void on_actionDrawEllipse_triggered();
    void on_actionImport_GPX_triggered();
    void on_actionMarkerImport_triggered();
    void on_actionToolsOSMImport_triggered();

};

#endif // MAINWINDOW_H
