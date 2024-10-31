#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QHash>
#include <QPalette>

#include <SlippyMap/SlippyMapWidget.h>

#include "Weather/NationalWeatherServiceInterface.h"

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

namespace color_widgets {
    class ColorSelector;
}

namespace SlippyMap
{
    class SlippyMapWidgetMarker;
    class SlippyMapLayer;
    class SlippyMapLayerPolygon;
};

using namespace SlippyMap;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

protected:
    void resizeEvent(QResizeEvent *event) override;
    void loadPluginLayers();
    void loadMarkers();
    void setupContextMenus();
    void loadStartupSettings();
    void saveLayers();
    void saveWorkspace(QString fileName);
    bool closeWorkspace();
    void setupToolbar();
    void setWorkspaceDirty(bool dirty);
    void showPropertyPage(SlippyMapLayerObject *object);
    void updateRecentFileList();

private:
    Ui::MainWindow *ui;
    color_widgets::ColorSelector *m_strokeColorSelector;
    color_widgets::ColorSelector *m_fillColorSelector;
    DirectionListItemWidget *m_currentRouteListItemWidget = nullptr;
    MapDataImportDialog *m_importDialog = nullptr;
    NationalWeatherServiceInterface *m_weatherService = nullptr;
    QAction *m_markerDeleteAction = nullptr;
    QAction *m_markerVisibilityAction;
    QAction *m_animationPlayAction;
    QAction *m_animationPauseAction;
    QAction *m_animationForwardAction;
    QAction *m_animationReverseAction;
    QColor m_directionLineColor;
    QHash<QString,SlippyMapWidgetMarker*> m_gpsMarkers;
    QLabel *m_statusBarGpsStatusLabel;
    QLabel *m_statusBarPositionLabel;
    QLabel *m_statusBarStatusLabel;
    QLineEdit *m_toolBarLatitudeInput;
    QLineEdit *m_toolBarLongitudeInput;
    QList<ExplorerPluginInterface*> m_plugins;
    QList<LocationDataProvider*> m_gpsProviders;
    QList<QString> m_recentFileList;
    QList<SlippyMapWidgetLayer*> m_layers;
    QList<SlippyMapWidgetMarker*> m_loadedMarkers;
    QList<SlippyMapWidgetMarker*> m_weatherStationMarkers;
    QListWidgetItem *m_currentRouteListItem = nullptr;
    QMap<SlippyMapWidgetMarker*,QListWidgetItem*> m_markerListItemMap;
    QMenu *m_markerMenu = nullptr;
    QMessageBox *m_loadingDialog = nullptr;
    QNetworkAccessManager *m_weatherNetworkAccessManager;
    QPalette m_defaultPalette;
    QPointF m_contextMenuPoint;
    QPointF m_slippyContextMenuLocation;
    QPushButton *m_toolBarLatLonButton;
    QPushButton *m_zoomInButton;
    QPushButton *m_zoomOutButton;
    QPushButton *m_currentLocationButton;
    QSpinBox *m_strokeWidth;
    QString m_workspaceFileName;
    QTimer *m_saveSplitterPosTimer = nullptr;
    QTimer *m_saveWindowSizeTimer = nullptr;
    SettingsDialog *m_settingsDialog = nullptr;
    SlippyMapLayer *m_defaultMarkerLayer = nullptr;
    SlippyMapLayer *m_gpsMarkerLayer = nullptr;
    SlippyMapLayer* m_weatherLayer;
    SlippyMapLayerManager *m_layerManager = nullptr;
    SlippyMapLayerObject *m_selectedObject = nullptr;
    SlippyMapLayerObjectPropertyPage *m_selectedObjectPropertyPage = nullptr;
    SlippyMapLayerPolygon *m_forecastZonePolygon;
    SlippyMapWidget::LineSet *m_currentRouteLineSet = nullptr;
    WeatherForecastWindow *m_weatherForecastWindow = nullptr;
    WeatherStationMarker *m_weatherStationMarker;
    TextLogViewerForm *m_nmeaLog = nullptr;
    int m_requestCount = 0;
    bool m_workspaceDirty = true;

    QPoint m_contextMenuLocation;
    QMenu *m_contextMenu = nullptr;
    QAction *m_coordAction = nullptr;
    QAction *m_addMarkerAction = nullptr;
    QAction *m_deleteMarkerAction = nullptr;
    QAction *m_markerPropertiesAction = nullptr;
    QAction *m_centerMapAction = nullptr;
    QAction *m_zoomInHereMapAction = nullptr;
    QAction *m_zoomOutHereMapAction = nullptr;
    QAction *m_copyCoordinatesAction = nullptr;
    QAction *m_copyLatitudeAction = nullptr;
    QAction *m_copyLongitudeAction = nullptr;
    QAction *m_editShapeAction = nullptr;
    QAction *m_deleteShapeAction = nullptr;
    QAction *m_getForecastHereAction = nullptr;
    QAction *m_newLayerAction = nullptr;
    QAction *m_deleteLayerAction = nullptr;

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
    void onSlippyMapDrawModeChanged(SlippyMapWidget::DrawMode mode);
    void onSlippyMapLayerObjectActivated(SlippyMapLayerObject *object);
    void onSlippyMapLayerObjectDeactivated(SlippyMapLayerObject *object);
    void onSlippyMapLayerObjectDoubleClicked(SlippyMapLayerObject *object);
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
    void onTvwMarkersContextMenuRequested(const QPoint& point);
    void onMarkerMenuPropertiesActionTriggered();
    void onPluginLayerObjectProviderMarkerAdded(SlippyMapLayerObject *object);
    void onAddMarkerActionTriggered();
    void onDeleteMarkerActionTriggered();
    void onEditMarkerActionTriggered();
    void onCenterMapActionTriggered();
    void setDarkModeEnabled(bool enabled);
    void onEditShapeActionTriggered();
    /**
     * @brief Save splitter position after finish moving.
     */
    void onSplitterPosTimerTimeout();

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
      * @brief Delete the active layer (ask first).
      */
      void deleteLayer();
private slots:
    void on_actionNewMarker_triggered();
    void on_actionViewSidebar_toggled(bool arg1);
    void on_actionViewClearRoute_triggered();
    void on_actionFileSettings_triggered();
    void on_actionMapGpsAddSource_triggered();
    void on_actionViewGpsLog_triggered();
    void on_tvwMarkers_activated(const QModelIndex &index);
    void on_tvwMarkers_clicked(const QModelIndex &index);
    void on_actionDrawRectangle_triggered();
    void on_actionDrawEllipse_triggered();
    void on_actionImport_triggered();
    void on_actionMarkerImport_triggered();
    void on_actionToolsOSMImport_triggered();

};

#endif // MAINWINDOW_H
