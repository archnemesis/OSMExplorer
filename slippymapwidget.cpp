#include "slippymapwidget.h"

#include <math.h>

#include <QDebug>

#include <QPainter>
#include <QPaintEvent>
#include <QDragEnterEvent>
#include <QDragLeaveEvent>
#include <QDragMoveEvent>

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrl>

#include <QMessageBox>
#include <QPixmap>
#include <QPoint>

SlippyMapWidget::SlippyMapWidget(QWidget *parent) : QWidget(parent)
{
    m_net = new QNetworkAccessManager(this);
    m_zoomLevel = 14;
    m_tileSet = new QList<Tile*>();

    m_lat = 45.541460;
    m_lon = -122.999700;
    emit centerChanged(m_lat, m_lon);
}

SlippyMapWidget::~SlippyMapWidget()
{

}

void SlippyMapWidget::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);

    for (int i = 0; i < m_tileSet->size(); i++) {
        Tile *t = m_tileSet->at(i);
        if (t->isLoaded()) {
            painter.drawPixmap(t->point(), t->pixmap());
        }
    }
}

void SlippyMapWidget::mousePressEvent(QMouseEvent *event)
{
    m_dragging = true;
    m_dragStart = event->pos();
}

void SlippyMapWidget::mouseReleaseEvent(QMouseEvent *event)
{
    m_dragging = false;
}

void SlippyMapWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (m_dragging) {
        QPoint pos = event->pos();
        QPoint diff = pos - m_dragStart;
        m_dragStart = pos;

        double deg_per_pixel = 360.0 / pow(2.0, m_zoomLevel) / 256.0;
        m_lat = m_lat + (deg_per_pixel * diff.y());
        m_lon = m_lon - (deg_per_pixel * diff.x());

        emit centerChanged(m_lat, m_lon);

        remap();
    }
}

void SlippyMapWidget::resizeEvent(QResizeEvent *event)
{
    (void)event;
    remap();
}

qint32 SlippyMapWidget::long2tilex(double lon, int z)
{
    return (qint32)(floor((lon + 180.0) / 360.0 * pow(2.0, z)));
}

qint32 SlippyMapWidget::lat2tiley(double lat, int z)
{
    return (qint32)(floor((1.0 - log( tan(lat * M_PI/180.0) + 1.0 / cos(lat * M_PI/180.0)) / M_PI) / 2.0 * pow(2.0, z)));
}

double SlippyMapWidget::tilex2long(qint32 x, qint32 z)
{
    return x / pow(2.0, z) * 360.0 - 180;
}

double SlippyMapWidget::tiley2lat(qint32 y, qint32 z)
{
    double n = M_PI - 2.0 * M_PI * y / pow(2.0, z);
        return 180.0 / M_PI * atan(0.5 * (exp(n) - exp(-n)));
}

void SlippyMapWidget::remap()
{
    qint32 tiles_wide = (qint32)ceil((double)width() / 256.0);
    if ((tiles_wide % 2) == 0) tiles_wide++;

    qint32 tiles_high = (qint32)ceil((double)height() / 256.0);
    if ((tiles_high % 2) == 0) tiles_high++;

    qint32 centerX = width() / 2;
    qint32 centerY = height() / 2;

    qint32 startX = centerX - 128 - ((tiles_wide / 2) * 256);
    qint32 startY = centerY - 128 - ((tiles_high / 2) * 256);

    qint32 tile_x = long2tilex(m_lon, m_zoomLevel);
    qint32 tile_y = lat2tiley(m_lat, m_zoomLevel);
    qint32 tile_x_start = tile_x - (tiles_wide - 1) / 2;
    qint32 tile_y_start = tile_y - (tiles_high - 1) / 2;

    QList<Tile*> deleteList(*m_tileSet);

    for (int y = 0; y < tiles_high; y++) {
        for (int x = 0; x < tiles_wide; x++) {
            qint32 this_x = tile_x_start + x;
            qint32 this_y = tile_y_start + y;
            QPoint point(startX + (x * 256), startY + (y * 256));

            Tile *tile = nullptr;
            for (int i = 0; i < m_tileSet->length(); i++) {
                if (m_tileSet->at(i)->x() == this_x && m_tileSet->at(i)->y() == this_y) {
                    tile = m_tileSet->at(i);
                }
            }

            if (tile == nullptr) {
                tile = new Tile(this_x, this_y, point);
                m_tileSet->append(tile);

                QString tile_path = QString("http://10.1.1.150/osm_tiles/%1/%2/%3.png")
                        .arg(m_zoomLevel)
                        .arg(this_x)
                        .arg(this_y);
                qDebug() << "Requesting tile:" << tile_path;
                QNetworkRequest req(tile_path);
                QNetworkReply *reply = m_net->get(req);
                tile->setPendingReply(reply);
                connect(reply, &QNetworkReply::finished, [=]() {
                  if (tile->isDiscarded()) {
                    delete tile;
                  }
                  else {
                    if (reply->error() != QNetworkReply::NoError) {
                      // handle error
                      return;
                    }

                    QByteArray data = reply->readAll();
                    QPixmap pixmap;
                    pixmap.loadFromData(data);
                    tile->setPixmap(pixmap);
                    repaint();
                  }

                  reply->deleteLater();
                });
            }
            else {
                QPoint old = tile->point();
                tile->setPoint(point);
                deleteList.removeOne(tile);
                repaint();
            }
        }
    }

    while (deleteList.length() > 0) {
        Tile *todelete = deleteList.takeFirst();

        if (todelete->pendingReply()->isRunning()) {
          todelete->discard();
          todelete->pendingReply()->abort();
        }

        m_tileSet->removeOne(todelete);
        delete todelete;
    }
}
