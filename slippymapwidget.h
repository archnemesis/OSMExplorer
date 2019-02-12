#ifndef SLIPPYMAPWIDGET_H
#define SLIPPYMAPWIDGET_H

#include <QWidget>
#include <QPoint>
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
class QPushButton;
class QLineEdit;
class QCompleter;

class SlippyMapWidget : public QWidget
{
    Q_OBJECT
public:
    explicit SlippyMapWidget(QWidget *parent = nullptr);
    virtual ~SlippyMapWidget();
    QString latLonToString(double lat, double lon);

public slots:
    void setCenter(double latitude, double longitude);
    void setZoomLevel(int zoom);
    void increaseZoomLevel();
    void decreaseZoomLevel();
    void setTextLocation(QString location);

protected slots:
    void searchBarReturnPressed();

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void enterEvent(QEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

signals:
    void centerChanged(double latitude, double longitude);
    void zoomLevelChanged(int zoom);
    void tileRequestInitiated();
    void tileRequestFinished();
    void cursorPositionChanged(double latitude, double longitude);
    void cursorLeft();
    void cursorEntered();

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

    void remap();

    bool m_dragging = false;
    QPoint m_dragStart;
    int m_zoomLevel = 0;
    int m_tileX = 0;
    int m_tileY = 0;
    int m_maxZoom = 18;
    int m_minZoom = 0;
    double m_lat;
    double m_lon;
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
};

#endif // SLIPPYMAPWIDGET_H
