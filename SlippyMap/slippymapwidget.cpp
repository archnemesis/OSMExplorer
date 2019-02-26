#include "slippymapwidget.h"
#include "slippymapwidgetmarker.h"
#include "slippymapwidgetlayer.h"
#include "slippymapwidgetmarkermodel.h"
#include "defaults.h"

#define _USE_MATH_DEFINES
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
#include <QFile>
#include <QDir>

#include <QRegularExpressionMatch>
#include <QMenu>
#include <QAction>
#include <QCompleter>
#include <QMessageBox>
#include <QPixmap>
#include <QPoint>
#include <QPointF>
#include <QPushButton>
#include <QLineEdit>
#include <QPainterPath>
#include <QFontMetrics>
#include <QInputDialog>
#include <QClipboard>
#include <QApplication>
#include <QCryptographicHash>
#include <QListWidget>
#include <QListWidgetItem>

SlippyMapWidget::SlippyMapWidget(QWidget *parent) : QWidget(parent)
{
    setMouseTracking(true);

    m_tileServer = DEFAULT_TILE_SERVER;
    m_net = new QNetworkAccessManager(this);
    m_zoomLevel = DEFAULT_ZOOM;
    m_clipboard = QApplication::clipboard();

    m_lat = DEFAULT_LATITUDE;
    m_lon = DEFAULT_LONGITUDE;
    emit centerChanged(m_lat, m_lon);

    m_zoomInButton = new QPushButton(this);
    m_zoomInButton->setText("");
    m_zoomInButton->move(10, 10);
    m_zoomInButton->setMaximumWidth(35);
    m_zoomInButton->setMinimumWidth(35);
    m_zoomInButton->setMaximumHeight(35);
    m_zoomInButton->setMinimumHeight(35);
    m_zoomInButton->setIcon(QIcon(":/icons/zoomin.png"));
    m_zoomInButton->setIconSize(QSize(28,28));

    m_zoomOutButton = new QPushButton(this);
    m_zoomOutButton->setText("");
    m_zoomOutButton->move(10, 45);
    m_zoomOutButton->setMaximumWidth(35);
    m_zoomOutButton->setMinimumWidth(35);
    m_zoomOutButton->setMaximumHeight(35);
    m_zoomOutButton->setMinimumHeight(35);
    m_zoomOutButton->setIcon(QIcon(":/icons/zoomout.png"));
    m_zoomOutButton->setIconSize(QSize(28,28));

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

    m_searchSuggestionsList = new QListWidget(this);
    m_searchSuggestionsList->move(55, 45);
    m_searchSuggestionsList->setFixedSize(searchBarWidth, 100);

    m_searchSuggestionsList->addItem("This is a test");
    m_searchSuggestionsList->addItem("This is a test");
    m_searchSuggestionsList->addItem("This is a test");
    m_searchSuggestionsList->addItem("This is a test");
    m_searchSuggestionsList->addItem("This is a test");
    m_searchSuggestionsList->addItem("This is a test");

    connect(m_zoomInButton, &QPushButton::pressed, this, &SlippyMapWidget::increaseZoomLevel);
    connect(m_zoomOutButton, &QPushButton::pressed, this, &SlippyMapWidget::decreaseZoomLevel);
    connect(m_searchBar, &QLineEdit::returnPressed, this, &SlippyMapWidget::searchBarReturnPressed);
    connect(m_searchBar, &QLineEdit::textEdited, this, &SlippyMapWidget::searchBarTextEdited);

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

    m_coordAction = new QAction();
    m_coordAction->setEnabled(false);

    m_addMarkerAction = new QAction();
    m_addMarkerAction->setText(tr("Add Marker"));
    m_deleteMarkerAction = new QAction();
    m_deleteMarkerAction->setText(tr("Delete Marker"));
    m_setMarkerLabelAction = new QAction();
    m_setMarkerLabelAction->setText(tr("Properties..."));
    m_centerMapAction = new QAction();
    m_centerMapAction->setText(tr("Center Here"));
    m_zoomInHereMapAction = new QAction();
    m_zoomInHereMapAction->setText(tr("Zoom In"));
    m_zoomOutHereMapAction = new QAction();
    m_zoomOutHereMapAction->setText(tr("Zoom Out"));
    m_copyCoordinatesAction = new QAction();
    m_copyCoordinatesAction->setText(tr("Copy Coordinates"));
    m_copyLatitudeAction = new QAction();
    m_copyLatitudeAction->setText(tr("Copy Latitude"));
    m_copyLongitudeAction = new QAction();
    m_copyLongitudeAction->setText(tr("Copy Longitude"));

    m_contextMenu = new QMenu();
    setupContextMenu();

    connect(m_addMarkerAction, &QAction::triggered, this, &SlippyMapWidget::addMarkerActionTriggered);
    connect(m_deleteMarkerAction, &QAction::triggered, this, &SlippyMapWidget::deleteMarkerActionTriggered);
    connect(m_setMarkerLabelAction, &QAction::triggered, this, &SlippyMapWidget::setMarkerLabelActionTriggered);
    connect(m_zoomInHereMapAction, &QAction::triggered, this, &SlippyMapWidget::zoomInHereActionTriggered);
    connect(m_zoomOutHereMapAction, &QAction::triggered, this, &SlippyMapWidget::zoomOutHereActionTriggered);
    connect(m_centerMapAction, &QAction::triggered, this, &SlippyMapWidget::centerMapActionTriggered);
    connect(m_copyCoordinatesAction, &QAction::triggered, this, &SlippyMapWidget::copyCoordinatesActionTriggered);
    connect(m_copyLatitudeAction, &QAction::triggered, this, &SlippyMapWidget::copyLatitudeActionTriggered);
    connect(m_copyLongitudeAction, &QAction::triggered, this, &SlippyMapWidget::copyLongitudeActionTriggered);
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
            .arg(fabs(lat), 8, 'f', 4, '0')
            .arg(dir_lat)
            .arg(fabs(lon), 8, 'f', 4, '0')
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

QList<SlippyMapWidgetMarker *> SlippyMapWidget::markerList()
{
    return m_markers;
}

SlippyMapWidgetMarker *SlippyMapWidget::addMarker(double latitude, double longitude)
{
    SlippyMapWidgetMarker *marker =
            new SlippyMapWidgetMarker(QPointF(longitude, latitude));
    m_markers.append(marker);
    update();
    emit markerAdded(marker);
    return marker;
}

SlippyMapWidgetMarker *SlippyMapWidget::addMarker(double latitude, double longitude, QString label)
{
    SlippyMapWidgetMarker *marker =
            new SlippyMapWidgetMarker(QPointF(longitude, latitude), label);
    m_markers.append(marker);
    update();
    emit markerAdded(marker);
    return marker;
}

SlippyMapWidgetMarker *SlippyMapWidget::addMarker(QPointF location)
{
    return addMarker(location.x(), location.y());
}

SlippyMapWidgetMarker *SlippyMapWidget::addMarker(QPointF location, QString label)
{
    return addMarker(location.x(), location.y(), label);
}

void SlippyMapWidget::addMarker(SlippyMapWidgetMarker *marker)
{
    m_markers.append(marker);
    connect(marker, &SlippyMapWidgetMarker::changed, this, &SlippyMapWidget::onMarkerChanged);
    emit markerAdded(marker);
}

void SlippyMapWidget::deleteMarker(SlippyMapWidgetMarker *marker)
{
    m_markers.removeOne(marker);
}

void SlippyMapWidget::addLineSet(SlippyMapWidget::LineSet *lineSet)
{
    QVector<QLineF> lines;

    for (int i = 0; i < (lineSet->segments()->length() - 1); i++) {
        QPointF p1 = lineSet->segments()->at(i);
        QPointF p2 = lineSet->segments()->at(i+1);
        lines.append(QLineF(p1, p2));
    }

    m_lineSetPaths[lineSet] = lines;
    m_lineSets.append(lineSet);
}

void SlippyMapWidget::removeLineSet(SlippyMapWidget::LineSet *lineSet)
{
    m_lineSets.removeOne(lineSet);
    m_lineSetPaths.remove(lineSet);
}

void SlippyMapWidget::addLayer(SlippyMapWidgetLayer *layer)
{
    m_layers.append(layer);
    m_layerTileMaps[layer] = QList<Tile*>();
    connect(
                layer,
                &SlippyMapWidgetLayer::updated,
                this,
                static_cast<void (SlippyMapWidget::*)()>(&SlippyMapWidget::remap));
    remap();
}

QList<SlippyMapWidgetLayer *> SlippyMapWidget::layers()
{
    return m_layers;
}

void SlippyMapWidget::takeLayer(SlippyMapWidgetLayer *layer)
{
    m_layers.removeOne(layer);
    remap();
}

void SlippyMapWidget::addContextMenuAction(QAction *action)
{
    m_contextMenuActions.append(action);
    setupContextMenu();
}

void SlippyMapWidget::removeContextMenuAction(QAction *action)
{
    m_contextMenuActions.removeOne(action);
    setupContextMenu();
}

void SlippyMapWidget::setCenterOnCursorWhileZooming(bool enable)
{
    m_centerOnCursorWhileZooming = enable;
}

bool SlippyMapWidget::centerOnCursorWhileZooming()
{
    return m_centerOnCursorWhileZooming;
}

void SlippyMapWidget::setSearchBarVisible(bool visible)
{
    m_searchBarVisible = visible;
    m_searchBar->setVisible(false);
    update();
}

void SlippyMapWidget::setZoomButtonsVisible(bool visible)
{
    m_zoomButtonsVisible = visible;
    m_zoomInButton->setVisible(false);
    m_zoomOutButton->setVisible(false);
    update();
}

void SlippyMapWidget::setLocationButtonVisible(bool visible)
{
    m_locationButtonVisible = visible;
    m_currentLocationButton->setVisible(visible);
    update();
}

void SlippyMapWidget::setZoomSliderVisible(bool visible)
{
    m_zoomSliderVisible = visible;
    update();
}

void SlippyMapWidget::setTileCachingEnabled(bool enabled)
{
    m_cacheTiles = enabled;
}

void SlippyMapWidget::setTileCacheDir(QString dir)
{
    m_cacheTileDir = dir;
    QDir cacheDir(dir);
    cacheDir.mkpath(".");
}

void SlippyMapWidget::setModel(SlippyMapWidgetMarkerModel *model)
{
    m_markerModel = model;
    connect(m_markerModel, &SlippyMapWidgetMarkerModel::markerAdded, this, &SlippyMapWidget::onMarkerModelMarkerAdded);
    connect(m_markerModel, &SlippyMapWidgetMarkerModel::markerRemoved, this, &SlippyMapWidget::onMarkerModelMarkerRemoved);
    update();
}

void SlippyMapWidget::setCenter(double latitude, double longitude)
{
    m_lat = latitude;
    m_lon = longitude;

    remap();
}

void SlippyMapWidget::setCenter(QPointF position)
{
    m_lat = position.y();
    m_lon = position.x();
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
        lat = match_lat.toDouble(&ok);

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
        update();
        emit markerDeleted(m_activeMarker);
        m_activeMarker = nullptr;
    }
}

void SlippyMapWidget::setMarkerLabelActionTriggered()
{
    if (m_activeMarker != nullptr) {
        emit markerEditRequested(m_activeMarker);
    }
}

void SlippyMapWidget::centerMapActionTriggered()
{
    double lat = widgetY2lat(m_contextMenuLocation.y());
    double lon = widgetX2long(m_contextMenuLocation.x());
    setCenter(lat, lon);
}

void SlippyMapWidget::zoomInHereActionTriggered()
{
    double lat = widgetY2lat(m_contextMenuLocation.y());
    double lon = widgetX2long(m_contextMenuLocation.x());
    setCenter(lat, lon);
    increaseZoomLevel();
}

void SlippyMapWidget::zoomOutHereActionTriggered()
{
    double lat = widgetY2lat(m_contextMenuLocation.y());
    double lon = widgetX2long(m_contextMenuLocation.x());
    setCenter(lat, lon);
    decreaseZoomLevel();
}

void SlippyMapWidget::copyCoordinatesActionTriggered()
{
    double lat = widgetY2lat(m_contextMenuLocation.y());
    double lon = widgetX2long(m_contextMenuLocation.x());
    QString str = latLonToString(lat, lon);
    m_clipboard->setText(str);
}

void SlippyMapWidget::copyLatitudeActionTriggered()
{
    double lat = widgetY2lat(m_contextMenuLocation.y());
    QString str = QString("%1").arg(lat, 8, 'f', 4, '0');
    m_clipboard->setText(str);
}

void SlippyMapWidget::copyLongitudeActionTriggered()
{
    double lon = widgetX2long(m_contextMenuLocation.x());
    QString str = QString("%1").arg(lon, 8, 'f', 4, '0');
    m_clipboard->setText(str);
}

void SlippyMapWidget::onMarkerChanged()
{
    update();
}

void SlippyMapWidget::searchBarTextEdited(const QString &text)
{
    emit searchTextChanged(text);
}

void SlippyMapWidget::paintEvent(QPaintEvent *event)
{
    (void)event;

    QPainter painter(this);
    painter.setRenderHints(QPainter::TextAntialiasing | QPainter::Antialiasing);

    for (SlippyMapWidgetLayer *layer : m_layers) {
        for (int i = 0; i < m_layerTileMaps[layer].size(); i++) {
            Tile *t = m_layerTileMaps[layer].at(i);
            if (t->isLoaded()) {
                painter.drawPixmap(t->point(), t->pixmap());
            }
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

    qint32 pixlen = static_cast<qint32>(len / S);
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
    m_searchSuggestionsList->setFixedSize(searchBarWidth, 100);

    if (m_lineSets.length() > 0) {
        for (LineSet *lineSet : m_lineSets) {
            QPen linePen;
            linePen.setStyle(Qt::SolidLine);
            linePen.setColor(lineSet->color());
            linePen.setWidth(lineSet->width());
            painter.setPen(linePen);
            QBrush lineBrush;
            lineBrush.setStyle(Qt::SolidPattern);
            lineBrush.setColor(QColor(Qt::black));
            painter.setBrush(lineBrush);

            for (int i = 0; i < m_lineSetPaths[lineSet].size(); i++) {
                QLineF line = m_lineSetPaths[lineSet].at(i);
                QPointF p1 = line.p1();
                QPointF p2 = line.p2();
                p1.setX(long2widgetX(p1.x()));
                p1.setY(lat2widgety(p1.y()));
                p2.setX(long2widgetX(p2.x()));
                p2.setY(lat2widgety(p2.y()));
                painter.drawLine(p1, p2);
            }
        }
    }

    if (m_markerModel != nullptr) {
        QList<SlippyMapWidgetMarker *> markers = m_markerModel->markersForRect(boundingBoxLatLon());

        for (SlippyMapWidgetMarker *marker : markers) {
            qint32 x = long2widgetX(marker->position().x());
            qint32 y = lat2widgety(marker->position().y());
            marker->drawMarker(&painter, QPoint(x, y));
        }
    }

    if (m_markers.length() > 0) {
        QRectF bbox = boundingBoxLatLon();

        for (SlippyMapWidgetMarker *marker : m_markers) {
            if (bbox.contains(marker->position())) {
                qint32 x = long2widgetX(marker->position().x());
                qint32 y = lat2widgety(marker->position().y());
                marker->drawMarker(&painter, QPoint(x, y));
            }
        }
    }
}

void SlippyMapWidget::mousePressEvent(QMouseEvent *event)
{
    setFocus(Qt::MouseFocusReason);
    m_dragging = true;
    m_dragStart = event->pos();
    m_dragRealStart = event->pos();
    m_dragButton = event->button();

    if (event->button() == Qt::LeftButton) {
        if (m_markerModel != nullptr) {
            QList<SlippyMapWidgetMarker *> markers = m_markerModel->markersForRect(boundingBoxLatLon());

            for (SlippyMapWidgetMarker *marker : markers) {
                qint32 marker_x = long2widgetX(marker->longitude());
                qint32 marker_y = lat2widgety(marker->latitude());
                QRect marker_box(
                            (marker_x - 5),
                            (marker_y - 5),
                            10, 10);
                if (marker_box.contains(event->pos())) {
                    m_dragMarker = marker;
                    return;
                }
            }
        }

        for (SlippyMapWidgetMarker *marker : m_markers) {
            if (marker->isMovable()) {
                qint32 marker_x = long2widgetX(marker->longitude());
                qint32 marker_y = lat2widgety(marker->latitude());
                QRect marker_box(
                            (marker_x - 5),
                            (marker_y - 5),
                            10, 10);

                if (marker_box.contains(event->pos())) {
                    m_dragMarker = marker;
                    return;
                }
            }
        }
    }
}

void SlippyMapWidget::mouseReleaseEvent(QMouseEvent *event)
{
    m_dragging = false;
    m_dragMarker = nullptr;

    if (event->button() == Qt::LeftButton && event->pos() == m_dragRealStart) {
        if (m_markerModel != nullptr) {
            QList<SlippyMapWidgetMarker *> markers = m_markerModel->markersForRect(boundingBoxLatLon());

            for (SlippyMapWidgetMarker *marker : markers) {
                qint32 marker_x = long2widgetX(marker->longitude());
                qint32 marker_y = lat2widgety(marker->latitude());
                QRect marker_box(
                            (marker_x - 5),
                            (marker_y - 5),
                            10, 10);
                if (marker_box.contains(event->pos())) {
                    m_activeMarker = marker;
                    emit markerActivated(marker);
                    update();
                    return;
                }
            }
        }

        for (SlippyMapWidgetMarker *marker : m_markers) {
            qint32 marker_x = long2widgetX(marker->longitude());
            qint32 marker_y = lat2widgety(marker->latitude());
            QRect marker_box(
                        (marker_x - 5),
                        (marker_y - 5),
                        10, 10);

            if (marker_box.contains(event->pos())) {
                m_activeMarker = marker;
                emit markerActivated(marker);
                update();
                return;
            }
        }

        if (m_activeMarker != nullptr) {
            emit markerDeactivated(m_activeMarker);
            m_activeMarker = nullptr;
            update();
        }
    }
}

void SlippyMapWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (m_dragButton != Qt::LeftButton) return;

    double scale_factor = 1 / cos(m_lat * (M_PI / 180.0));
    double deg_per_pixel = (360.0 / pow(2.0, m_zoomLevel)) / 256.0;
    double deg_per_pixel_y = deg_per_pixel / scale_factor;
    QPoint pos = event->pos();
    QPoint diff = pos - m_dragStart;

    if (m_dragging && m_dragMarker != nullptr) {
        QPointF markerPoint = m_dragMarker->position();
        m_dragStart = pos;
        markerPoint.setY(markerPoint.y() - (deg_per_pixel_y * diff.y()));
        markerPoint.setX(markerPoint.x() + (deg_per_pixel * diff.x()));
        m_dragMarker->setPosition(markerPoint);
        update();
    }
    else if (m_dragging) {
        m_dragStart = pos;
        m_lat = m_lat + (deg_per_pixel_y * diff.y());
        m_lon = m_lon - (deg_per_pixel * diff.x());

        emit centerChanged(m_lat, m_lon);
        remap();
    }

    emit cursorPositionChanged(widgetY2lat(event->pos().y()), widgetX2long(event->pos().x()));
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

            if (m_centerOnCursorWhileZooming) {
                setCenter(widgetY2lat(event->y()), widgetX2long(event->x()));
            }

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

    if (m_markerModel != nullptr) {
        QList<SlippyMapWidgetMarker *> markers = m_markerModel->markersForRect(boundingBoxLatLon());

        for (SlippyMapWidgetMarker *marker : markers) {
            qint32 marker_x = long2widgetX(marker->longitude());
            qint32 marker_y = lat2widgety(marker->latitude());
            QRect marker_box(
                        (marker_x - 5),
                        (marker_y - 5),
                        10, 10);
            if (marker_box.contains(event->pos())) {
                m_addMarkerAction->setVisible(false);
                m_setMarkerLabelAction->setVisible(true);
                m_deleteMarkerAction->setVisible(true);
                m_activeMarker = marker;
                break;
            }
        }
    }

    for (SlippyMapWidgetMarker *marker : m_markers) {
        qint32 marker_x = long2widgetX(marker->longitude());
        qint32 marker_y = lat2widgety(marker->latitude());
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
    emit contextMenuActivated(widgetY2lat(event->y()), widgetX2long(event->x()));
    m_contextMenu->exec(event->globalPos());
}

qint32 SlippyMapWidget::long2tilex(double lon, int z)
{
    return static_cast<qint32>(floor((lon + 180.0) / 360.0 * pow(2.0, z)));
}

qint32 SlippyMapWidget::lat2tiley(double lat, int z)
{
    return static_cast<qint32>(floor((1.0 - log( tan(lat * M_PI/180.0) + 1.0 / cos(lat * M_PI/180.0)) / M_PI) / 2.0 * pow(2.0, z)));
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
    double deg_per_pixel = (360.0 / pow(2.0, m_zoomLevel)) / 256.0;
    double width_deg = deg_per_pixel * width();
    double left_deg = m_lon - (width_deg / 2);
    return static_cast<qint32>((lon - left_deg) / deg_per_pixel);
}

qint32 SlippyMapWidget::lat2widgety(double lat)
{
    double scale_factor = 1 / cos(m_lat * (M_PI / 180.0));
    double deg_per_pixel = (360.0 / pow(2.0, m_zoomLevel)) / 256.0;
    double deg_per_pixel_y = deg_per_pixel / scale_factor;
    double height_deg = deg_per_pixel_y * height();
    double top_deg = m_lat - (height_deg / 2);
    return height() - static_cast<qint32>((lat - top_deg) / deg_per_pixel_y);
}

double SlippyMapWidget::widgetX2long(qint32 x)
{
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

double SlippyMapWidget::degPerPixelX()
{
    return (360.0 / pow(2.0, m_zoomLevel)) / 256.0;
}

double SlippyMapWidget::degPerPixelY()
{
    double scale_factor = 1 / cos(m_lat * (M_PI / 180.0));
    double deg_per_pixel = (360.0 / pow(2.0, m_zoomLevel)) / 256.0;
    return deg_per_pixel / scale_factor;
}

QRectF SlippyMapWidget::boundingBoxLatLon()
{
    double width_deg = degPerPixelX() * width();
    double left_deg = m_lon - (width_deg / 2);

    double height_deg = degPerPixelY() * height();
    double top_deg = m_lat + (height_deg / 2);

    return QRectF(left_deg, top_deg, width_deg, -height_deg);
}

void SlippyMapWidget::remap()
{
    qint32 tiles_wide = static_cast<qint32>(ceil(static_cast<double>(width()) / 256.0));
    if ((tiles_wide % 2) == 0) tiles_wide++;

    qint32 tiles_high = static_cast<qint32>(ceil(static_cast<double>(height()) / 256.0));
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

    qint32 diff_pix_x = 255 + static_cast<qint32>(diff_lon / deg_per_pixel) - 128;
    qint32 diff_pix_y = static_cast<qint32>(diff_lat / deg_per_pixel_y) - 128;

    qint32 centerX = width() / 2;
    qint32 centerY = height() / 2;

    qint32 startX = centerX - 128 - ((tiles_wide / 2) * 256) + diff_pix_x;
    qint32 startY = centerY - 128 - ((tiles_high / 2) * 256) - diff_pix_y;

    for (SlippyMapWidgetLayer *layer : m_layers) {
        QList<Tile*> deleteList(m_layerTileMaps[layer]);

        if (layer->isVisible()) {
            for (int y = 0; y < tiles_high; y++) {
                for (int x = 0; x < tiles_wide; x++) {
                    qint32 this_x = tile_x_start + x;
                    qint32 this_y = tile_y_start + y;
                    QPoint point(startX + (x * 256), startY + (y * 256));

                    Tile *tile = nullptr;
                    for (int i = 0; i < m_layerTileMaps[layer].length(); i++) {
                        if (m_layerTileMaps[layer].at(i)->x() == this_x && m_layerTileMaps[layer].at(i)->y() == this_y) {
                            tile = m_layerTileMaps[layer].at(i);
                        }
                    }

                    if (tile == nullptr) {
                        tile = new Tile(this_x, this_y, point);
                        m_layerTileMaps[layer].append(tile);

                        QString tileUrl = layer->tileUrl()
                                .arg(m_zoomLevel)
                                .arg(this_x)
                                .arg(this_y);
                        QString fileExt = QUrl(tileUrl).fileName().split(".").last();
                        QString fileName = QString("%1/%2/%3.%4")
                                .arg(m_zoomLevel)
                                .arg(this_x)
                                .arg(this_y)
                                .arg(fileExt);
                        QString cachedFileName = QString("%1/%2/%3")
                                .arg(m_cacheTileDir)
                                .arg(layer->tileUrlHash())
                                .arg(fileName);

                        if (m_cacheTiles == true && QFile::exists(cachedFileName)) {
                            QPixmap pixmap(cachedFileName);
                            tile->setPixmap(pixmap);
                            update();
                            qDebug() << "Cache hit!" << layer->name();
                        }
                        else {
                            QNetworkRequest req(tileUrl);
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

                                    if (m_cacheTiles) {
                                        QFile cacheFile(cachedFileName);
                                        QFileInfo cacheFileInfo(cacheFile);
                                        cacheFileInfo.dir().mkpath(".");
                                        if (cacheFile.open(QIODevice::ReadWrite)) {
                                            cacheFile.write(data);
                                            cacheFile.close();
                                        }
                                    }

                                    QPixmap pixmap;
                                    pixmap.loadFromData(data);
                                    tile->setPixmap(pixmap);
                                    tile->setPendingReply(nullptr);
                                    update();
                                }

                                reply->deleteLater();
                            });
                        }

                    }
                    else {
                        QPoint old = tile->point();
                        tile->setPoint(point);
                        deleteList.removeOne(tile);
                        if (old != point) update();
                    }
                }
            }
        }

        while (deleteList.length() > 0) {
            Tile *todelete = deleteList.takeFirst();

            if (todelete->pendingReply() != nullptr && todelete->pendingReply()->isRunning()) {
                m_layerTileMaps[layer].removeOne(todelete);
                todelete->discard();
                todelete->pendingReply()->abort();
            }
            else {
                m_layerTileMaps[layer].removeOne(todelete);
                delete todelete;
            }
        }
    }
}

void SlippyMapWidget::onMarkerModelMarkerAdded(SlippyMapWidgetMarker *marker)
{
    QRectF currentViewport = boundingBoxLatLon();
    if (currentViewport.contains(marker->position())) {
        update();
    }
}

void SlippyMapWidget::onMarkerModelMarkerRemoved(SlippyMapWidgetMarker *marker)
{
    QRectF currentViewport = boundingBoxLatLon();
    if (currentViewport.contains(marker->position())) {
        update();
    }
}

void SlippyMapWidget::setupContextMenu()
{
    m_contextMenu->clear();
    m_contextMenu->addAction(m_coordAction);
    m_contextMenu->addSeparator();
    m_contextMenu->addAction(m_addMarkerAction);
    m_contextMenu->addAction(m_deleteMarkerAction);
    m_contextMenu->addAction(m_setMarkerLabelAction);
    m_contextMenu->addSeparator();
    m_contextMenu->addAction(m_centerMapAction);
    m_contextMenu->addAction(m_zoomInHereMapAction);
    m_contextMenu->addAction(m_zoomOutHereMapAction);
    m_contextMenu->addSeparator();
    m_contextMenu->addAction(m_copyCoordinatesAction);
    m_contextMenu->addAction(m_copyLatitudeAction);
    m_contextMenu->addAction(m_copyLongitudeAction);

    if (m_contextMenuActions.length() > 0) {
        m_contextMenu->addSeparator();
        for (QAction *action : m_contextMenuActions) {
            m_contextMenu->addAction(action);
        }
    }
}
