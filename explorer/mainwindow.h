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
#include "slippymapwidgetshape.h"

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
class SlippyMapShapePropertyPage;

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
    QPalette m_defaultPalette;
    int m_requestCount = 0;
    QLabel *m_statusBarPositionLabel;
    QLabel *m_statusBarStatusLabel;
    QLabel *m_statusBarGpsStatusLabel;
    QMap<SlippyMapWidgetMarker*,QListWidgetItem*> m_markerListItemMap;
    QColor m_directionLineColor;
    QListWidgetItem *m_currentRouteListItem = nullptr;
    DirectionListItemWidget *m_currentRouteListItemWidget = nullptr;
    SlippyMapWidget::LineSet *m_currentRouteLineSet = nullptr;
    QPointF m_slippyContextMenuLocation;
    QAction *m_directionsToHereAction;
    QAction *m_directionsFromHereAction;
    QTimer *m_saveSplitterPosTimer = nullptr;
    QTimer *m_saveWindowSizeTimer = nullptr;
    SettingsDialog *m_settingsDialog = nullptr;
    QNetworkAccessManager *m_net;
    QMessageBox *m_loadingDialog = nullptr;
    QList<SlippyMapWidgetLayer*> m_layers;
    QList<LocationDataProvider*> m_gpsProviders;
    QHash<QString,SlippyMapWidgetMarker*> m_gpsMarkers;
    TextLogViewerForm *m_nmeaLog = nullptr;
    SlippyMapWidgetMarkerModel *m_markerModel = nullptr;
    SlippyMapWidgetMarkerGroup *m_markerModelGroup_myMarkers;
    SlippyMapWidgetMarkerGroup *m_markerModelGroup_gpsMarkers;
    SlippyMapWidgetMarkerGroup *m_markerModelGroup_aprsDotFiMarkers;
    QList<ExplorerPluginInterface*> m_plugins;
    QMap<ExplorerPluginInterface*,SlippyMapWidgetMarkerGroup> m_pluginMarkerGroupMap;
    QMenu *m_markerMenu = nullptr;
    QAction *m_markerPropertiesAction = nullptr;
    QAction *m_markerDeleteAction = nullptr;
    QList<SlippyMapWidgetMarker*> m_loadedMarkers;

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

    SlippyMapWidgetShape *m_selectedShape = nullptr;

protected slots:
    void onSlippyMapCenterChanged(double latitude, double longitude);
    void onSlippyMapZoomLevelChanged(int zoom);
    void onSlippyMapTileRequestStarted();
    void onSlippyMapTileRequestFinished();
    void onSlippyMapCursorPositionChanged(double latitude, double longitude);
    void onSlippyMapCursorEntered();
    void onSlippyMapCursorLeft();
    void onSlippyMapMarkerAdded(SlippyMapWidgetMarker *marker);
    void onSlippyMapMarkerDeleted(SlippyMapWidgetMarker *marker);
    void onSlippyMapMarkerUpdated(SlippyMapWidgetMarker *marker);
    void onSlippyMapMarkerEditRequested(SlippyMapWidgetMarker *marker);
    void onSlippyMapContextMenuActivated(double latitude, double longitude);
    void onSlippyMapSearchTextChanged(const QString &text);
    void onSlippyMapContextMenuRequested(const QPoint& point);
    void onSlippyMapRectSelected(QRect rect);
    void onSlippyMapDrawModeChanged(SlippyMapWidget::DrawMode mode);
    void onSlippyMapShapeActivated(SlippyMapWidgetShape *shape);
    void onSlippyMapShapeDeactivated(SlippyMapWidgetShape *shape);

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
