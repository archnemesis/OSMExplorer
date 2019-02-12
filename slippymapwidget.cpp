#include "slippymapwidget.h"

#include <math.h>

#include <QDebug>
#include <QGuiApplication>
#include <QPalette>

#include <QPainter>
#include <QPaintEvent>
#include <QDragEnterEvent>
#include <QDragLeaveEvent>
#include <QDragMoveEvent>
#include <QWheelEvent>
#include <QContextMenuEvent>

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrl>

#include <QRegularExpressionMatch>
#include <QMenu>
#include <QAction>
#include <QCompleter>
#include <QMessageBox>
#include <QPixmap>
#include <QPoint>
#include <QPushButton>
#include <QLineEdit>
#include <QPainterPath>
#include <QFontMetrics>
#include <QInputDialog>

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

    //^(\-?\d+\.?\d*)\s*(N|W|E|S*)\s*\,?\s*(\-?\d*\.?\d*)\s*(N|W|E|S*)$
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

    QPalette systemPalette = QGuiApplication::palette();
    m_markerBrush.setStyle(Qt::SolidPattern);
    m_markerBrush.setColor(systemPalette.highlight().color());
    m_markerPen.setStyle(Qt::NoPen);
    m_markerLabelBrush.setStyle(Qt::SolidPattern);
    m_markerLabelBrush.setColor(systemPalette.background().color());
    m_markerLabelPen.setStyle(Qt::SolidLine);
    m_markerLabelPen.setColor(systemPalette.dark().color());
    m_markerLabelTextBrush.setStyle(Qt::NoBrush);
    m_markerLabelTextPen.setStyle(Qt::SolidLine);
    m_markerLabelTextPen.setColor(systemPalette.text().color());

    m_coordAction = new QAction();
    m_coordAction->setEnabled(false);

    m_addMarkerAction = new QAction();
    m_addMarkerAction->setText(tr("Add marker here"));
    m_deleteMarkerAction = new QAction();
    m_deleteMarkerAction->setText(tr("Delete marker"));
    m_setMarkerLabelAction = new QAction();
    m_setMarkerLabelAction->setText(tr("Set label..."));

    connect(m_addMarkerAction, &QAction::triggered, this, &SlippyMapWidget::addMarkerActionTriggered);
    connect(m_deleteMarkerAction, &QAction::triggered, this, &SlippyMapWidget::deleteMarkerActionTriggered);
    connect(m_setMarkerLabelAction, &QAction::triggered, this, &SlippyMapWidget::setMarkerLabelActionTriggered);

    m_contextMenu = new QMenu(this);
    m_contextMenu->addAction(m_coordAction);
    m_contextMenu->addSeparator();
    m_contextMenu->addAction(m_addMarkerAction);
    m_contextMenu->addAction(m_deleteMarkerAction);
    m_contextMenu->addAction(m_setMarkerLabelAction);
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

void SlippyMapWidget::addMarker(double latitude, double longitude)
{
    m_markers.append(new Marker(latitude, longitude));
    update();
}

void SlippyMapWidget::addMarker(double latitude, double longitude, QString label)
{
    m_markers.append(new Marker(latitude, longitude, label));
    update();
}

void SlippyMapWidget::addMarker(SlippyMapWidget::Marker *marker)
{
    m_markers.append(marker);
}

void SlippyMapWidget::deleteMarker(SlippyMapWidget::Marker *marker)
{
    m_markers.removeOne(marker);
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

    QString match_lat = match.captured(1);
    QString match_lat_card = match.captured(2);
    QString match_lon = match.captured(3);
    QString match_lon_card = match.captured(4);

    double lat;
    double lon;

    if (match_lat.length() > 0) {
        bool ok;
        lat = match_lat.toFloat(&ok);

        if (!ok) {
            goto parseError;
        }

        if (match_lat_card.length() == 1) {
            if (match_lat_card == "N") {
                lat = fabs(lat);
            }
            else {
                lat = -1 * fabs(lat);
            }
        }
    }
    else {
        goto parseError;
    }

    if (match_lon.length() > 0) {
        bool ok;
        lon = match_lon.toDouble(&ok);

        if (!ok) {
            goto parseError;
        }

        if (match_lon_card.length() == 1) {
            if (match_lon_card == "E") {
                lon = fabs(lon);
            }
            else {
                lon = -1 * fabs(lon);
            }
        }
    }
    else {
        goto parseError;
    }

    setCenter(lat, lon);
    m_searchBar->setText(latLonToString(lat, lon));
    addMarker(lat, lon, latLonToString(lat, lon));

    return;

parseError:
    QMessageBox::critical(
                this,
                tr("Location Error"),
                tr("Unable to parse location. Please try again."));
    return;
}

void SlippyMapWidget::searchBarReturnPressed()
{
    setTextLocation(m_searchBar->text());
}

void SlippyMapWidget::addMarkerActionTriggered()
{
    double lat = widgetY2lat(m_contextMenuLocation.y());
    double lon = widgetX2long(m_contextMenuLocation.x());
    addMarker(lat, lon, latLonToString(lat, lon));
}

void SlippyMapWidget::deleteMarkerActionTriggered()
{
    if (m_activeMarker != nullptr) {
        m_markers.removeOne(m_activeMarker);
        delete m_activeMarker;
        m_activeMarker = nullptr;
        update();
    }
}

void SlippyMapWidget::setMarkerLabelActionTriggered()
{
    if (m_activeMarker != nullptr) {
        QString label = QInputDialog::getText(
                    this,
                    latLonToString(m_activeMarker->latitude(), m_activeMarker->longitude()),
                    tr("Enter new label"),
                    QLineEdit::Normal,
                    m_activeMarker->label());
        if (label.length() > 0) {
            m_activeMarker->setLabel(label);
            update();
        }
    }
}

void SlippyMapWidget::paintEvent(QPaintEvent *event)
{
    (void)event;

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

    if (m_markers.length() > 0) {
        double scale_factor = 1 / cos(m_lat * (M_PI / 180.0));
        double deg_per_pixel = (360.0 / pow(2.0, m_zoomLevel)) / 256.0;
        double deg_per_pixel_y = deg_per_pixel / scale_factor;
        double width_deg = deg_per_pixel * width();
        double height_deg = deg_per_pixel_y * height();
        double left_deg = m_lon - (width_deg / 2);
        double top_deg = m_lat - (height_deg / 2);

        for (Marker *marker : m_markers) {
            if (marker->longitude() > left_deg && marker->longitude() < (left_deg + width_deg)) {
                if (marker->latitude() > top_deg && marker->latitude() < (top_deg + height_deg)) {
                    qint32 x = (qint32)((marker->longitude() - left_deg) / deg_per_pixel);
                    qint32 y = height() - (qint32)((marker->latitude() - top_deg) / deg_per_pixel_y);
                    qint32 rad = (m_activeMarker == marker ? 10 : 5);

                    painter.setBrush(m_markerBrush);
                    painter.setPen(m_markerPen);
                    painter.drawEllipse(QPoint(x, y), rad, rad);

                    QFontMetrics metrics(painter.font());
                    if (marker->label().length() > 0) {
                        qint32 label_w = metrics.width(marker->label());
                        qint32 label_h = metrics.height();
                        qint32 label_x = x - (label_w / 2);
                        qint32 label_y = y - (label_h + 15);
                        painter.setBrush(m_markerLabelBrush);
                        painter.setPen(m_markerLabelPen);
                        painter.drawRoundRect(
                                    label_x - 5,
                                    label_y - 5,
                                    label_w + 10,
                                    label_h + 10,
                                    5, 5);
                        painter.setBrush(m_markerLabelTextBrush);
                        painter.setPen(m_markerLabelTextPen);
                        painter.drawText(
                                    label_x,
                                    label_y + (label_h / 2) + 5,
                                    marker->label());
                    }
                }
            }
        }
    }
}

void SlippyMapWidget::mousePressEvent(QMouseEvent *event)
{
    m_dragging = true;
    m_dragStart = event->pos();
    m_dragRealStart = event->pos();
    m_dragButton = event->button();
}

void SlippyMapWidget::mouseReleaseEvent(QMouseEvent *event)
{
    m_dragging = false;

    if (event->button() == Qt::LeftButton && event->pos() == m_dragRealStart) {
        m_activeMarker = nullptr;

        for (Marker *marker : m_markers) {
            qint32 marker_x = long2widgetX(marker->longitude());
            qint32 marker_y = lat2widgety(marker->latitude());
            QRect marker_box(
                        (marker_x - 5),
                        (marker_y - 5),
                        10, 10);

            if (marker_box.contains(event->pos())) {
                m_activeMarker = marker;
                break;
            }
        }

        update();
    }
}

void SlippyMapWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (m_dragButton != Qt::LeftButton) return;

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
        remap();
    }

    double width_deg = deg_per_pixel * width();
    double height_deg = deg_per_pixel_y * height();
    double left_deg = m_lon - (width_deg / 2);
    double top_deg = m_lat - (height_deg / 2);
    double xpos = left_deg + (deg_per_pixel * event->pos().x());
    double ypos = top_deg - (deg_per_pixel_y * event->pos().y());
    emit cursorPositionChanged(widgetY2lat(event->pos().y()), xpos);
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

void SlippyMapWidget::contextMenuEvent(QContextMenuEvent *event)
{
    m_coordAction->setText(latLonToString(widgetY2lat(event->y()), widgetX2long(event->x())));
    m_contextMenuLocation = event->pos();
    m_addMarkerAction->setVisible(true);
    m_deleteMarkerAction->setVisible(false);
    m_setMarkerLabelAction->setVisible(false);
    m_activeMarker = nullptr;

    for (Marker *marker : m_markers) {
        qint32 marker_x = long2widgetX(marker->longitude());
        qint32 marker_y = lat2widgety(marker->latitude());
        qDebug() << "Marker is at:" << marker_x << marker_y;
        if (event->x() > (marker_x - 5) && event->x() < (marker_x + 5)) {
            if (event->y() > (marker_y - 5) && event->y() < (marker_y + 5)) {
                m_addMarkerAction->setVisible(false);
                m_setMarkerLabelAction->setVisible(true);
                m_deleteMarkerAction->setVisible(true);
                m_activeMarker = marker;
                break;
            }
        }
    }

    update();
    m_contextMenu->exec(event->globalPos());
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

qint32 SlippyMapWidget::long2widgetX(double lon)
{
    double scale_factor = 1 / cos(m_lat * (M_PI / 180.0));
    double deg_per_pixel = (360.0 / pow(2.0, m_zoomLevel)) / 256.0;
    double width_deg = deg_per_pixel * width();
    double left_deg = m_lon - (width_deg / 2);
    return (qint32)((lon - left_deg) / deg_per_pixel);
}

qint32 SlippyMapWidget::lat2widgety(double lat)
{
    double scale_factor = 1 / cos(m_lat * (M_PI / 180.0));
    double deg_per_pixel = (360.0 / pow(2.0, m_zoomLevel)) / 256.0;
    double deg_per_pixel_y = deg_per_pixel / scale_factor;
    double height_deg = deg_per_pixel_y * height();
    double top_deg = m_lat - (height_deg / 2);
    return height() - (qint32)((lat - top_deg) / deg_per_pixel_y);
}

double SlippyMapWidget::widgetX2long(qint32 x)
{
    double scale_factor = 1 / cos(m_lat * (M_PI / 180.0));
    double deg_per_pixel = (360.0 / pow(2.0, m_zoomLevel)) / 256.0;
    double width_deg = deg_per_pixel * width();
    double left_deg = m_lon - (width_deg / 2);
    double xpos = left_deg + (deg_per_pixel * x);
    return xpos;
}

double SlippyMapWidget::widgetY2lat(qint32 y)
{
    double scale_factor = 1 / cos(m_lat * (M_PI / 180.0));
    double deg_per_pixel = (360.0 / pow(2.0, m_zoomLevel)) / 256.0;
    double deg_per_pixel_y = deg_per_pixel / scale_factor;
    double height_deg = deg_per_pixel_y * height();
    double top_deg = m_lat + (height_deg / 2);
    double ypos = top_deg - (deg_per_pixel_y * y);
    return ypos;
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
