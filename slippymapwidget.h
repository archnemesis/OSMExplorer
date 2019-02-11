#ifndef SLIPPYMAPWIDGET_H
#define SLIPPYMAPWIDGET_H

#include <QWidget>
#include <QPoint>
#include <QMap>
#include <QMutex>

class QNetworkAccessManager;
class QNetworkReply;
class QPaintEvent;
class QPixmap;
class QMouseEvent;

class SlippyMapWidget : public QWidget
{
    Q_OBJECT
public:
    explicit SlippyMapWidget(QWidget *parent = nullptr);
    virtual ~SlippyMapWidget();

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

signals:
    void centerChanged(double latitude, double longitude);

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
    private:
        qint32 m_x;
        qint32 m_y;
        bool m_loaded = false;
        bool m_discarded = false;
        QPoint m_point;
        QPixmap m_pixmap;
        QMutex m_pixmapMutex;
    };

    qint32 long2tilex(double lon, int z);
    qint32 lat2tiley(double lat, int z);
    double tilex2long(qint32 x, qint32 z);
    double tiley2lat(qint32 y, qint32 z);

    void remap();

    bool m_dragging = false;
    QPoint m_dragStart;
    int m_zoomLevel = 0;
    double m_lat;
    double m_lon;
    QMutex m_tileMutex;
    QNetworkAccessManager *m_net;
    QList<Tile*> *m_tileSet;
};

#endif // SLIPPYMAPWIDGET_H
