#ifndef SLIPPYMAPWIDGET_H
#define SLIPPYMAPWIDGET_H

#include <QWidget>
#include <QPoint>
#include <QPointF>
#include <QMap>
#include <QMutex>
#include <QBrush>
#include <QPen>
#include <QFont>
#include <QRegularExpression>

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

class SlippyMapWidget : public QWidget
{
    Q_OBJECT
public:
    class Marker {
    public:
        Marker(double latitude, double longitude, QString label) {
            m_label = label;
            m_latitude = latitude;
            m_longitude = longitude;
        }
        Marker(double latitude, double longitude) {
            m_latitude = latitude;
            m_longitude = longitude;
        }
        void setLatitude(double latitude) { m_latitude = latitude; }
        void setLongitude(double longitude) { m_longitude = longitude; }
        void setLabel(QString label) { m_label = label; }
        void setColor(QColor color) { m_color = color; }
        double latitude() { return m_latitude; }
        double longitude() { return m_longitude; }
        QString label() { return m_label; }
        QColor color() { return m_color; }
    private:
        double m_latitude;
        double m_longitude;
        QString m_label;
        QColor m_color;
    };

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

    explicit SlippyMapWidget(QWidget *parent = nullptr);
    virtual ~SlippyMapWidget() override;
    static QString latLonToString(double lat, double lon);
    void setTileServer(QString server);
    QString tileServer();
    QList<Marker*> markerList();
    void addMarker(double latitude, double longitude);
    void addMarker(double latitude, double longitude, QString label);
    void addMarker(Marker *marker);
    void deleteMarker(Marker *marker);
    void addLineSet(LineSet *lineSet);
    void removeLineSet(LineSet *lineSet);
    void addContextMenuAction(QAction *action);
    void removeContextMenuAction(QAction *action);

public slots:
    void setCenter(double latitude, double longitude);
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
    void markerActivated(Marker *marker);
    void markerDeactivated(Marker *marker);
    void markerAdded(Marker *marker);
    void markerDeleted(Marker *marker);
    void markerUpdated(Marker *marker);
    void contextMenuActivated(double latitude, double longitude);

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
    qint32 long2widgetX(double lon);
    qint32 lat2widgety(double lat);
    double widgetX2long(qint32 x);
    double widgetY2lat(qint32 y);
    double degPerPixelX();
    double degPerPixelY();
    QRectF boundingBoxLatLon();

    void remap();
    void setupContextMenu();

    bool m_dragging = false;
    QPoint m_dragRealStart;
    QPoint m_dragStart;
    Qt::MouseButton m_dragButton;
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
    QList<Tile*> *m_tileSet;
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

    QList<Marker*> m_markers;
    Marker* m_activeMarker = nullptr;
    QBrush m_markerBrush;
    QPen m_markerPen;
    QBrush m_markerLabelBrush;
    QPen m_markerLabelPen;
    QBrush m_markerLabelTextBrush;
    QPen m_markerLabelTextPen;

    QMenu *m_contextMenu;
    QAction *m_coordAction;
    QAction *m_addMarkerAction;
    QAction *m_deleteMarkerAction;
    QAction *m_setMarkerLabelAction;
    QAction *m_centerMapAction;
    QAction *m_zoomInHereMapAction;
    QAction *m_zoomOutHereMapAction;
    QAction *m_copyCoordinatesAction;
    QAction *m_copyLatitudeAction;
    QAction *m_copyLongitudeAction;
    QPoint m_contextMenuLocation;

    QClipboard *m_clipboard;

    QList<LineSet*> m_lineSets;
    QMap<LineSet*,QVector<QLineF>> m_lineSetPaths;

    QList<QAction*> m_contextMenuActions;
};

#endif // SLIPPYMAPWIDGET_H
