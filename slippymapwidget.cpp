#include "slippymapwidget.h"

#include <math.h>

#include <QDebug>

#include <QPainter>
#include <QPaintEvent>
#include <QDragEnterEvent>
#include <QDragLeaveEvent>
#include <QDragMoveEvent>
#include <QWheelEvent>

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrl>

#include <QRegularExpressionMatch>

#include <QCompleter>
#include <QMessageBox>
#include <QPixmap>
#include <QPoint>
#include <QPushButton>
#include <QLineEdit>
#include <QPainterPath>

SlippyMapWidget::SlippyMapWidget(QWidget *parent) : QWidget(parent)
{
    setMouseTracking(true);

    m_tileServer = "10.1.1.150";
    m_net = new QNetworkAccessManager(this);
    m_zoomLevel = 14;
    m_tileSet = new QList<Tile*>();

    m_lat = 45.541460;
    m_lon = -122.999700;
    emit centerChanged(m_lat, m_lon);

    m_zoomInButton = new QPushButton(this);
    m_zoomInButton->setText("➕");
    m_zoomInButton->move(10, 10);
    m_zoomInButton->setMaximumWidth(35);
    m_zoomInButton->setMinimumWidth(35);
    m_zoomInButton->setMaximumHeight(35);
    m_zoomInButton->setMinimumHeight(35);

    m_zoomOutButton = new QPushButton(this);
    m_zoomOutButton->setText("➖");
    m_zoomOutButton->move(10, 45);
    m_zoomOutButton->setMaximumWidth(35);
    m_zoomOutButton->setMinimumWidth(35);
    m_zoomOutButton->setMaximumHeight(35);
    m_zoomOutButton->setMinimumHeight(35);

    m_currentLocationButton = new QPushButton(this);
    m_currentLocationButton->setText("☼");
    m_currentLocationButton->move(10, 90);
    m_currentLocationButton->setMaximumWidth(35);
    m_currentLocationButton->setMinimumWidth(35);
    m_currentLocationButton->setMaximumHeight(35);
    m_currentLocationButton->setMinimumHeight(35);

    m_searchBar = new QLineEdit(this);
    m_searchBar->setText(latLonToString(m_lat, m_lon));
    m_searchBar->move(55, 10);
    m_searchBar->setMinimumHeight(35);
    m_searchBar->setMaximumHeight(35);

    int searchBarWidth = width() - (55 + 10);
    m_searchBar->setMinimumWidth(searchBarWidth);
    m_searchBar->setMaximumWidth(searchBarWidth);

    connect(m_zoomInButton, &QPushButton::pressed, this, &SlippyMapWidget::increaseZoomLevel);
    connect(m_zoomOutButton, &QPushButton::pressed, this, &SlippyMapWidget::decreaseZoomLevel);
    connect(m_searchBar, &QLineEdit::returnPressed, this, &SlippyMapWidget::searchBarReturnPressed);

    m_scaleBrush.setStyle(Qt::SolidPattern);
    m_scaleBrush.setColor(QColor(0,0,0,128));
    m_scalePen.setStyle(Qt::NoPen);

    m_scaleTextBrush.setStyle(Qt::SolidPattern);
    m_scaleTextBrush.setColor(Qt::black);
    m_scaleTextPen.setStyle(Qt::SolidLine);
    m_scaleTextPen.setColor(Qt::white);
    m_scaleTextPen.setWidth(1);
    m_scaleTextFont.setPixelSize(12);
    m_scaleTextFont.setBold(true);

    m_locationParser.setPattern("^(\\-?\\d*\\.?\\d*)\\s*(N|W|E|S*)\\s*\\,?\\s*(\\-?\\d*\\.?\\d*)\\s*(N|W|E|S*)$");

    QStringList testWords;
    testWords << "Africa";
    testWords << "Asia";
    testWords << "Asiaface";
    testWords << "TheMoon";
    m_locationCompleter = new QCompleter(testWords, this);
    m_locationCompleter->setCaseSensitivity(Qt::CaseInsensitive);
    m_searchBar->setCompleter(m_locationCompleter);

    QFont searchFont;
    searchFont.setPixelSize(18);
    m_searchBar->setFont(searchFont);
}

SlippyMapWidget::~SlippyMapWidget()
{

}

QString SlippyMapWidget::latLonToString(double lat, double lon)
{
    char dir_lat;
    char dir_lon;

    if (lat > 0) {
        dir_lat = 'N';
    }
    else {
        dir_lat = 'S';
    }

    if (lon > 0) {
        dir_lon = 'E';
    }
    else {
        dir_lon = 'W';
    }

    QString ret = QString("%1 %2 %3 %4")
            .arg(lat, 8, 'f', 4, '0')
            .arg(dir_lat)
            .arg(lon, 8, 'f', 4, '0')
            .arg(dir_lon);

    return ret;
}

void SlippyMapWidget::setTileServer(QString server)
{
    m_tileServer = server;
}

QString SlippyMapWidget::tileServer()
{
    return m_tileServer;
}

void SlippyMapWidget::setCenter(double latitude, double longitude)
{
    m_lat = latitude;
    m_lon = longitude;
    remap();
}

void SlippyMapWidget::setZoomLevel(int zoom)
{
    if (zoom >= m_minZoom && zoom <= m_maxZoom) {
        m_zoomLevel = zoom;
        remap();
    }
}

void SlippyMapWidget::increaseZoomLevel()
{
    if (m_zoomLevel < m_maxZoom) {
        m_zoomLevel++;
        remap();
    }
}

void SlippyMapWidget::decreaseZoomLevel()
{
    if (m_zoomLevel > m_minZoom) {
        m_zoomLevel--;
        remap();
    }
}

void SlippyMapWidget::setTextLocation(QString location)
{
    QRegularExpressionMatch match = m_locationParser.match(location);
    if (!match.hasMatch()) {
        QMessageBox::critical(
                    this,
                    tr("Location Error"),
                    tr("Unable to parse location. Please try again."));
        return;
    }

    QString lat = match.captured(1);
    QString lat_card = match.captured(2);
    QString lon = match.captured(3);
    QString lon_card = match.captured(4);

    qDebug() << "Lat:" << lat << lat_card;
    qDebug() << "Lon:" << lon << lon_card;
}

void SlippyMapWidget::searchBarReturnPressed()
{
    setTextLocation(m_searchBar->text());
}

void SlippyMapWidget::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHints(QPainter::TextAntialiasing | QPainter::Antialiasing);

    for (int i = 0; i < m_tileSet->size(); i++) {
        Tile *t = m_tileSet->at(i);
        if (t->isLoaded()) {
            painter.drawPixmap(t->point(), t->pixmap());
        }
    }

    double C = 40075016.686;
    double S = C * cos(m_lat * (M_PI/180.0)) / pow(2.0, m_zoomLevel + 8);
    double len = S * 100;

    if (len < 100.0) {
        len = floor((len + 5) / 10) * 10;
    }
    else if (len < 1000.0) {
        len = floor((len + 50) / 100) * 100;
    }
    else if (len < 10000.0) {
        len = floor((len + 500) / 1000) * 1000;
    }
    else if (len < 100000.0) {
        len = floor((len + 5000) / 10000) * 10000;
    }
    else if (len < 1000000.0) {
        len = floor((len + 50000) / 100000) * 100000;
    }

    qint32 pixlen = (qint32)(len / S);
    int left = width() - m_scaleBarMarginRight - pixlen;
    int top = height() - m_scaleBarMarginBottom - m_scaleBarHeight;

    painter.setPen(m_scalePen);
    painter.setBrush(m_scaleBrush);
    painter.drawRect(left, top, pixlen, m_scaleBarHeight);

    QPainterPath textPath;
    textPath.addText(QPoint(left, (top - 10)), m_scaleTextFont, tr("%1 m").arg(len));

    painter.setBrush(m_scaleTextBrush);
    painter.setPen(m_scaleTextPen);
    painter.drawPath(textPath);

    int searchBarWidth = width() - (55 + 10);
    m_searchBar->setMinimumWidth(searchBarWidth);
    m_searchBar->setMaximumWidth(searchBarWidth);
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
    double scale_factor = 1 / cos(m_lat * (M_PI / 180.0));
    double deg_per_pixel = (360.0 / pow(2.0, m_zoomLevel)) / 256.0;
    double deg_per_pixel_y = deg_per_pixel / scale_factor;

    if (m_dragging) {
        QPoint pos = event->pos();
        QPoint diff = pos - m_dragStart;
        m_dragStart = pos;
        m_lat = m_lat + (deg_per_pixel_y * diff.y());
        m_lon = m_lon - (deg_per_pixel * diff.x());

        emit centerChanged(m_lat, m_lon);
        m_searchBar->setText(latLonToString(m_lat, m_lon));
        remap();
    }

    double width_deg = deg_per_pixel * width();
    double height_deg = deg_per_pixel_y * height();
    double left_deg = m_lon - (width_deg / 2);
    double top_deg = m_lat - (height_deg / 2);
    double xpos = left_deg + (deg_per_pixel * event->pos().x());
    double ypos = top_deg - (deg_per_pixel_y * event->pos().y());
    emit cursorPositionChanged(ypos, xpos);

    qDebug() << "Got position:" << ypos << xpos;
}

void SlippyMapWidget::enterEvent(QEvent *event)
{
    (void)event;
    emit cursorEntered();
}

void SlippyMapWidget::leaveEvent(QEvent *event)
{
    (void)event;
    emit cursorLeft();
}

void SlippyMapWidget::wheelEvent(QWheelEvent *event)
{
    QPoint deg = event->angleDelta();

    if (deg.y() > 0) {
        if (m_zoomLevel < m_maxZoom) {
            m_zoomLevel++;
            remap();
            emit zoomLevelChanged(m_zoomLevel);
        }
    }
    else if (deg.y() < 0) {
        if (m_zoomLevel > m_minZoom) {
            m_zoomLevel--;
            remap();
            emit zoomLevelChanged(m_zoomLevel);
        }
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

    tiles_wide += 2;
    tiles_high += 2;

    qint32 tile_x = long2tilex(m_lon, m_zoomLevel);
    qint32 tile_y = lat2tiley(m_lat, m_zoomLevel);
    qint32 tile_x_start = tile_x - (tiles_wide - 1) / 2;
    qint32 tile_y_start = tile_y - (tiles_high - 1) / 2;

    m_tileX = tile_x;
    m_tileY = tile_y;

    double scale_factor = 1 / cos(m_lat * (M_PI / 180.0));
    double deg_per_pixel = (360.0 / pow(2.0, m_zoomLevel)) / 256.0;
    double deg_per_pixel_y = deg_per_pixel / scale_factor;
    double snapped_lon = tilex2long(long2tilex(m_lon, m_zoomLevel), m_zoomLevel);
    double diff_lon = snapped_lon - m_lon;
    double snapped_lat = tiley2lat(lat2tiley(m_lat, m_zoomLevel), m_zoomLevel);
    double diff_lat = snapped_lat - m_lat;

    qint32 diff_pix_x = 255 + (qint32)(diff_lon / deg_per_pixel) - 128;
    qint32 diff_pix_y = (qint32)(diff_lat / deg_per_pixel_y) - 128;

    qint32 centerX = width() / 2;
    qint32 centerY = height() / 2;

    qint32 startX = centerX - 128 - ((tiles_wide / 2) * 256) + diff_pix_x;
    qint32 startY = centerY - 128 - ((tiles_high / 2) * 256) - diff_pix_y;

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

                QString tile_path = QString("http://%1/osm_tiles/%2/%3/%4.png")
                        .arg(m_tileServer)
                        .arg(m_zoomLevel)
                        .arg(this_x)
                        .arg(this_y);
                QNetworkRequest req(tile_path);
                emit tileRequestInitiated();
                QNetworkReply *reply = m_net->get(req);
                tile->setPendingReply(reply);
                connect(reply, &QNetworkReply::finished, [=]() {
                    emit tileRequestFinished();

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
                        tile->setPendingReply(nullptr);
                        update();
                    }

                    reply->deleteLater();
                });
            }
            else {
                QPoint old = tile->point();
                tile->setPoint(point);
                deleteList.removeOne(tile);
                if (old != point) update();
            }
        }
    }

    while (deleteList.length() > 0) {
        Tile *todelete = deleteList.takeFirst();

        if (todelete->pendingReply() != nullptr && todelete->pendingReply()->isRunning()) {
            m_tileSet->removeOne(todelete);
            todelete->discard();
            todelete->pendingReply()->abort();
        }
        else {
            m_tileSet->removeOne(todelete);
            delete todelete;
        }

    }
}
