#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMap>
#include <QHash>
#include <QVariant>
#include <QTimer>
#include <QPalette>
#include "slippymapwidget.h"
#include "slippymapwidgetmarker.h"
#include "slippymapwidgetlayer.h"
#include "slippymapwidgetmarkermodel.h"
#include "slippymaplayerobject.h"
#include "slippymaplayermarker.h"
#include "slippymaplayer.h"
#include "slippymaplayermanager.h"

namespace Ui {
class MainWindow;
}

class QComboBox;
class QLabel;
class QListWidgetItem;
class DirectionListItemWidget;
class SettingsDialog;
class QNetworkAccessManager;
class QNetworkReply;
class QMessageBox;
class QMenu;
class QAction;
class LocationDataProvider;
class TextLogViewerForm;
class ExplorerPluginInterface;
class SlippyMapLayerObjectPropertyPage;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

protected:
    void resizeEvent(QResizeEvent *event) override;
    void loadPlugins();
    void loadMarkers();
    void setupContextMenus();
    void loadStartupSettings();
    void saveLayers();
    void setupToolbar();

private:
    Ui::MainWindow *ui;
    DirectionListItemWidget *m_currentRouteListItemWidget = nullptr;
    QAction *m_directionsFromHereAction;
    QAction *m_directionsToHereAction;
    QAction *m_markerDeleteAction = nullptr;
    QAction *m_markerPropertiesAction = nullptr;
    QColor m_directionLineColor;
    QHash<QString,SlippyMapLayerMarker*> m_gpsMarkers;
    QLabel *m_statusBarGpsStatusLabel;
    QLabel *m_statusBarPositionLabel;
    QLabel *m_statusBarStatusLabel;
    QList<ExplorerPluginInterface*> m_plugins;
    QList<LocationDataProvider*> m_gpsProviders;
    QList<SlippyMapWidgetLayer*> m_layers;
    QList<SlippyMapWidgetMarker*> m_loadedMarkers;
    QListWidgetItem *m_currentRouteListItem = nullptr;
    QMap<ExplorerPluginInterface*,SlippyMapWidgetMarkerGroup> m_pluginMarkerGroupMap;
    QMap<SlippyMapWidgetMarker*,QListWidgetItem*> m_markerListItemMap;
    QMenu *m_markerMenu = nullptr;
    QMessageBox *m_loadingDialog = nullptr;
    QNetworkAccessManager *m_net;
    QPalette m_defaultPalette;
    QPointF m_slippyContextMenuLocation;
    QTimer *m_saveSplitterPosTimer = nullptr;
    QTimer *m_saveWindowSizeTimer = nullptr;
    SettingsDialog *m_settingsDialog = nullptr;
    SlippyMapLayer *m_gpsMarkerLayer = nullptr;
    SlippyMapLayer *m_defaultMarkerLayer = nullptr;
    SlippyMapLayerManager *m_layerManager = nullptr;
    SlippyMapWidget::LineSet *m_currentRouteLineSet = nullptr;
    TextLogViewerForm *m_nmeaLog = nullptr;
    int m_requestCount = 0;

    QPoint m_contextMenuLocation;
    QMenu *m_contextMenu = nullptr;
    QAction *m_coordAction = nullptr;
    QAction *m_addMarkerAction = nullptr;
    QAction *m_deleteMarkerAction = nullptr;
    QAction *m_setMarkerLabelAction = nullptr;
    QAction *m_centerMapAction = nullptr;
    QAction *m_zoomInHereMapAction = nullptr;
    QAction *m_zoomOutHereMapAction = nullptr;
    QAction *m_copyCoordinatesAction = nullptr;
    QAction *m_copyLatitudeAction = nullptr;
    QAction *m_copyLongitudeAction = nullptr;
    QAction *m_editShapeAction = nullptr;
    QAction *m_deleteShapeAction = nullptr;

    SlippyMapLayerObject *m_selectedObject = nullptr;

protected slots:
    void onSlippyMapCenterChanged(double latitude, double longitude);
    void onSlippyMapZoomLevelChanged(int zoom);
    void onSlippyMapTileRequestStarted();
    void onSlippyMapTileRequestFinished();
    void onSlippyMapCursorPositionChanged(double latitude, double longitude);
    void onSlippyMapCursorEntered();
    void onSlippyMapCursorLeft();
    //void onSlippyMapMarkerAdded(SlippyMapWidgetMarker *marker);
    //void onSlippyMapMarkerDeleted(SlippyMapWidgetMarker *marker);
    //void onSlippyMapMarkerUpdated(SlippyMapWidgetMarker *marker);
    //void onSlippyMapMarkerEditRequested(SlippyMapWidgetMarker *marker);
    void onSlippyMapContextMenuActivated(double latitude, double longitude);
    void onSlippyMapSearchTextChanged(const QString &text);
    void onSlippyMapContextMenuRequested(const QPoint& point);
    void onSlippyMapRectSelected(QRect rect);
    void onSlippyMapDrawModeChanged(SlippyMapWidget::DrawMode mode);
    void onSlippyMapLayerObjectActivated(SlippyMapLayerObject *object);
    void onSlippyMapLayerObjectDeactivated(SlippyMapLayerObject *object);

    void saveMarkers();
    void onDirectionsToHereTriggered();
    void onDirectionsFromHereTriggered();
    void onNetworkRequestFinished(QNetworkReply *reply);
    void onGpsDataProviderPositionUpdated(QString identifier, QPointF position, QHash<QString,QVariant> metadata);
    void onTvwMarkersContextMenuRequested(const QPoint& point);
    void onMarkerMenuPropertiesActionTriggered();
    void onPluginMarkerProviderMarkerAdded(SlippyMapWidgetMarker *marker);
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
private slots:
    void on_actionNewMarker_triggered();
    void on_actionViewSidebar_toggled(bool arg1);
    void on_actionViewClearRoute_triggered();
    void on_actionFileSettings_triggered();
    void on_btnDirectionsGo_clicked();
    void on_actionMapGpsAddSource_triggered();
    void on_actionViewGpsLog_triggered();
    void on_tvwMarkers_activated(const QModelIndex &index);
    void on_tvwMarkers_clicked(const QModelIndex &index);
    void on_actionDrawRectangle_triggered();
    void on_actionDrawEllipse_triggered();
};

#endif // MAINWINDOW_H
