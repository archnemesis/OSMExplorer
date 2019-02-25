#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMap>
#include <QHash>
#include <QVariant>
#include <QTimer>
#include "slippymapwidget.h"
#include "slippymapwidgetmarker.h"
#include "slippymapwidgetlayer.h"
#include "mapmarkermodel.h"

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
class LocationDataProvider;
class TextLogViewerForm;
class ExplorerPluginInterface;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void resizeEvent(QResizeEvent *event) override;
    void loadPlugins();

private:
    Ui::MainWindow *ui;
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
    MapMarkerModel *m_markerModel = nullptr;
    MapMarkerModel::MarkerGroup *m_markerModelGroup_myMarkers;
    MapMarkerModel::MarkerGroup *m_markerModelGroup_gpsMarkers;
    MapMarkerModel::MarkerGroup *m_markerModelGroup_aprsDotFiMarkers;
    QList<ExplorerPluginInterface*> m_plugins;

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
    void onSlippyMapContextMenuActivated(double latitude, double longitude);
    void onSlippyMapSearchTextChanged(const QString &text);
    void onDirectionsToHereTriggered();
    void onDirectionsFromHereTriggered();
    void onSplitterMoved(int pos, int index);
    void onNetworkRequestFinished(QNetworkReply *reply);
    void onDataProviderAprsFiPositionUpdated(QString identifier, QPointF position, QHash<QString,QVariant> metadata);
    void onGpsDataProviderPositionUpdated(QString identifier, QPointF position, QHash<QString,QVariant> metadata);

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
};

#endif // MAINWINDOW_H
