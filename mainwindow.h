#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMap>
#include <QTimer>
#include "slippymapwidget.h"

namespace Ui {
class MainWindow;
}

class QComboBox;
class QLabel;
class QListWidgetItem;
class DirectionListItemWidget;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void resizeEvent(QResizeEvent *event) override;

private:
    Ui::MainWindow *ui;
    int m_requestCount = 0;
    QLabel *m_statusBarPositionLabel;
    QLabel *m_statusBarStatusLabel;
    QMap<SlippyMapWidget::Marker*,QListWidgetItem*> m_markerListItemMap;
    QColor m_directionLineColor;
    QListWidgetItem *m_currentRouteListItem = nullptr;
    DirectionListItemWidget *m_currentRouteListItemWidget = nullptr;
    SlippyMapWidget::LineSet *m_currentRouteLineSet = nullptr;
    QPointF m_slippyContextMenuLocation;
    QAction *m_directionsToHereAction;
    QAction *m_directionsFromHereAction;
    QTimer *m_saveSplitterPosTimer;
    QTimer *m_saveWindowSizeTimer;

protected slots:
    void onSlippyMapCenterChanged(double latitude, double longitude);
    void onSlippyMapZoomLevelChanged(int zoom);
    void onSlippyMapTileRequestStarted();
    void onSlippyMapTileRequestFinished();
    void onSlippyMapCursorPositionChanged(double latitude, double longitude);
    void onSlippyMapCursorEntered();
    void onSlippyMapCursorLeft();
    void onSlippyMapMarkerAdded(SlippyMapWidget::Marker *marker);
    void onSlippyMapMarkerDeleted(SlippyMapWidget::Marker *marker);
    void onSlippyMapMarkerUpdated(SlippyMapWidget::Marker *marker);
    void onSlippyMapContextMenuActivated(double latitude, double longitude);
    void onDirectionsToHereTriggered();
    void onDirectionsFromHereTriggered();
    void onSplitterMoved(int pos, int index);
    void onSplitterPosTimerTimeout();
    void onWindowSizeTimerTimeout();
private slots:
    void on_actionNewMarker_triggered();
    void on_actionViewSidebar_toggled(bool arg1);
    void on_actionDebugOpenDirectionsFile_triggered();
    void on_actionViewClearRoute_triggered();
};

#endif // MAINWINDOW_H
