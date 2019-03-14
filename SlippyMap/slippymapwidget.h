#ifndef SLIPPYMAPWIDGET_H
#define SLIPPYMAPWIDGET_H

#include "defaults.h"
#include <QWidget>
#include <QPoint>
#include <QPointF>
#include <QMap>
#include <QMutex>
#include <QBrush>
#include <QPen>
#include <QFont>
#include <QRegularExpression>
#include <QCryptographicHash>
#include <QTimer>

class QNetworkAccessManager;
class QNetworkReply;
class QPaintEvent;
class QPixmap;
class QMouseEvent;
class QWheelEvent;
class QContextMenuEvent;
class QPushButton;
class QLineEdit;
class QCompleter;
class QMenu;
class QAction;
class QClipboard;
class QListWidget;
class SlippyMapWidgetMarkerModel;
class SlippyMapWidgetMarker;
class SlippyMapWidgetLayer;
class SlippyMapWidgetShape;

#if defined EXPORT_SYMBOLS
#define DECLARATION Q_DECL_EXPORT
#else
#define DECLARATION Q_DECL_IMPORT
#endif

class DECLARATION SlippyMapWidget : public QWidget
{
    Q_OBJECT
public:
    class LineSet {
    public:
        LineSet(QVector<QPointF> *segments, int width = 1, QColor color = Qt::black) {
            m_segments = segments;
            m_width = width;
            m_color = color;
        }
        QVector<QPointF> *segments() { return m_segments; }
        int width() { return m_width; }
        QColor color() { return m_color; }
    private:
        QVector<QPointF> *m_segments;
        int m_width;
        QColor m_color;
    };

    enum DrawMode {
        NoDrawing,
        RectDrawing,
        EllipseDrawing,
        PolygonDrawing
    };

    explicit SlippyMapWidget(QWidget *parent = nullptr);
    virtual ~SlippyMapWidget() override;
    static QString latLonToString(double lat, double lon);
    static QString geoCoordinatesToString(QPointF coords);
    void setTileServer(QString server);
    QString tileServer();
    QList<SlippyMapWidgetMarker*> markerList();
    SlippyMapWidgetMarker *addMarker(double latitude, double longitude);
    SlippyMapWidgetMarker *addMarker(double latitude, double longitude, QString label);
    SlippyMapWidgetMarker *addMarker(QPointF location);
    SlippyMapWidgetMarker *addMarker(QPointF location, QString label);
    void addMarker(SlippyMapWidgetMarker *marker);
    void deleteMarker(SlippyMapWidgetMarker *marker);
    void addLineSet(LineSet *lineSet);
    void removeLineSet(LineSet *lineSet);
    void addLayer(SlippyMapWidgetLayer *layer);
    QList<SlippyMapWidgetLayer*> layers();
    void takeLayer(SlippyMapWidgetLayer *layer);
    void addShape(SlippyMapWidgetShape *shape);
    QList<SlippyMapWidgetShape*> shapes();
    void setCenterOnCursorWhileZooming(bool enable);
    bool centerOnCursorWhileZooming();
    void setSearchBarVisible(bool visible);
    void setZoomButtonsVisible(bool visible);
    void setLocationButtonVisible(bool visible);
    void setZoomSliderVisible(bool visible);
    void setTileCachingEnabled(bool enabled);
    void setTileCacheDir(QString dir);
    void setModel(SlippyMapWidgetMarkerModel *model);
    double widgetX2long(qint32 x);
    double widgetY2lat(qint32 y);
    double degPerPixelX();
    double degPerPixelY();
    QRectF boundingBoxLatLon();
    qint32 long2widgetX(double lon);
    qint32 lat2widgety(double lat);
    QPointF widgetCoordsToGeoCoords(QPoint point);
    void setDrawMode(DrawMode mode);
    DrawMode drawMode();

public slots:
    void setCenter(double latitude, double longitude);
    void setCenter(QPointF position);
    void setZoomLevel(int zoom);
    void increaseZoomLevel();
    void decreaseZoomLevel();
    void setTextLocation(QString location);

protected slots:
    void searchBarReturnPressed();
    void addMarkerActionTriggered();
    void deleteMarkerActionTriggered();
    void setMarkerLabelActionTriggered();
    void centerMapActionTriggered();
    void zoomInHereActionTriggered();
    void zoomOutHereActionTriggered();
    void copyCoordinatesActionTriggered();
    void copyLatitudeActionTriggered();
    void copyLongitudeActionTriggered();
    void onMarkerChanged();
    void searchBarTextEdited(const QString &text);
    void remap();
    void onMarkerModelMarkerAdded(SlippyMapWidgetMarker *marker);
    void onMarkerModelMarkerRemoved(SlippyMapWidgetMarker *marker);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void enterEvent(QEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void contextMenuEvent(QContextMenuEvent *event) override;

signals:
    void centerChanged(double latitude, double longitude);
    void zoomLevelChanged(int zoom);
    void tileRequestInitiated();
    void tileRequestFinished();
    void cursorPositionChanged(double latitude, double longitude);
    void cursorLeft();
    void cursorEntered();
    void markerActivated(SlippyMapWidgetMarker *marker);
    void markerDeactivated(SlippyMapWidgetMarker *marker);
    void markerAdded(SlippyMapWidgetMarker *marker);
    void markerDeleted(SlippyMapWidgetMarker *marker);
    void markerUpdated(SlippyMapWidgetMarker *marker);
    void markerEditRequested(SlippyMapWidgetMarker *marker);
    void contextMenuRequested(const QPoint& point);
    void searchTextChanged(const QString &searchText);
    void rectSelected(const QRect &rect);
    void ellipseSelected(const QRect &rect);
    void polygonSelected(const QVector<QPoint> &points);
    void drawModeChanged(DrawMode mode);
    void shapeActivated(SlippyMapWidgetShape *shape);
    void shapeDeactivated(SlippyMapWidgetShape *shape);

private:
    class Tile {
    public:
        Tile(qint32 x, qint32 y, QPoint point) :
            m_x(x),
            m_y(y),
            m_point(point) {

        }
        Tile(qint32 x, qint32 y, QPoint point, QPixmap pixmap) :
            m_x(x),
            m_y(y),
            m_point(point),
            m_pixmap(pixmap) {
            m_loaded = true;
        }
        qint32 x() { return m_x; }
        qint32 y() { return m_y; }
        bool isLoaded() { return m_loaded; }
        bool isDiscarded() { return m_discarded; }
        void discard() { m_discarded = true; }
        QPoint point() { return m_point; }
        QPixmap pixmap() { return m_pixmap; }
        void setPoint(QPoint point) { m_point = point; }
        void setPixmap(QPixmap pixmap) { m_pixmap = pixmap; m_loaded = true; }
        void setPendingReply(QNetworkReply *reply) { m_pendingReply = reply; }
        QNetworkReply *pendingReply() { return m_pendingReply; }
    private:
        qint32 m_x;
        qint32 m_y;
        bool m_loaded = false;
        bool m_discarded = false;
        QPoint m_point;
        QPixmap m_pixmap;
        QMutex m_pixmapMutex;
        QNetworkReply *m_pendingReply = nullptr;
    };

    qint32 long2tilex(double lon, int z);
    qint32 lat2tiley(double lat, int z);
    double tilex2long(qint32 x, qint32 z);
    double tiley2lat(qint32 y, qint32 z);

    bool m_dragging = false;
    QPoint m_dragRealStart;
    QPoint m_dragStart;
    Qt::MouseButton m_dragButton;
    SlippyMapWidgetMarker *m_dragMarker = nullptr;
    int m_zoomLevel = 0;
    int m_tileX = 0;
    int m_tileY = 0;
    int m_maxZoom = 18;
    int m_minZoom = 0;
    double m_lat;
    double m_lon;
    bool m_positionSelected = false;
    QPoint m_selectedPosition;
    QMutex m_tileMutex;
    QNetworkAccessManager *m_net;
    QPushButton *m_zoomInButton;
    QPushButton *m_zoomOutButton;
    QPushButton *m_currentLocationButton;
    QLineEdit *m_searchBar;
    QString m_tileServer;

    QBrush m_scaleBrush;
    QPen m_scalePen;
    QBrush m_scaleTextBrush;
    QPen m_scaleTextPen;
    QColor m_scaleBarColor;
    QFont m_scaleTextFont;
    int m_scaleBarHeight = 10;
    int m_scaleBarMarginRight = 10;
    int m_scaleBarMarginBottom = 10;

    QRegularExpression m_locationParser;
    QCompleter *m_locationCompleter;

    SlippyMapWidgetMarkerModel *m_markerModel = nullptr;
    QList<SlippyMapWidgetMarker*> m_markers;
    SlippyMapWidgetMarker* m_activeMarker = nullptr;
    QBrush m_markerBrush;
    QPen m_markerPen;
    QBrush m_markerLabelBrush;
    QPen m_markerLabelPen;
    QBrush m_markerLabelTextBrush;
    QPen m_markerLabelTextPen;

    QPoint m_contextMenuLocation;

    QClipboard *m_clipboard;

    QMap<SlippyMapWidgetLayer*,QList<Tile*>> m_layerTileMaps;
    QList<SlippyMapWidgetLayer*> m_layers;
    QList<LineSet*> m_lineSets;
    QMap<LineSet*,QVector<QLineF>> m_lineSetPaths;

    QList<QAction*> m_contextMenuActions;

    /* autocomplete */
    QListWidget *m_searchSuggestionsList;

    /* configurable items */
    bool m_centerOnCursorWhileZooming = DEFAULT_CENTER_ON_CURSOR_ZOOM;
    bool m_searchBarVisible = true;
    bool m_zoomButtonsVisible = true;
    bool m_locationButtonVisible = true;
    bool m_zoomSliderVisible = true;
    bool m_cacheTiles = false;
    QString m_cacheTileDir;

    /* drawing */
    DrawMode m_drawMode = NoDrawing;
    QPoint m_drawModeRect_topLeft;
    QPoint m_drawModeRect_bottomRight;
    QBrush m_drawBrush;
    QPen m_drawPen;
    QList<SlippyMapWidgetShape*> m_shapes;
    SlippyMapWidgetShape *m_dragShape = nullptr;
    SlippyMapWidgetShape *m_activeShape = nullptr;
};

#endif // SLIPPYMAPWIDGET_H
