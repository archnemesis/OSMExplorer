#include "SlippyMapLayerTrack.h"
#include "SlippyMapLayerTrackPropertyPage.h"
#include "SlippyMapLayerTrackStylePropertyPage.h"

#include <cmath>

#include <QDebug>

#include <QPainter>
#include <QPointF>
#include <QBrush>
#include <QPolygonF>

#include "SlippyMapLayerTrackStylePropertyPage.h"
#include "gpx/gpxwaypoint.h"
#include "gpx/gpxtracksegment.h"

SlippyMapLayerTrack::SlippyMapLayerTrack(const GPXTrack& track) :
    m_trackLineWidth(5),
    m_trackLineStrokeWidth(1),
    m_trackLineColor(Qt::green),
    m_trackLineStrokeColor(Qt::darkGreen),
    m_waypointRadius(5),
    m_waypointColor(Qt::darkYellow)
{
    m_track = track;
    initStyle();

    for (const GPXTrackSegment& segment : track.segments()) {
        for (const GPXWaypoint& waypoint : segment.points()) {
            m_points.append(QPointF(waypoint.longitude(), waypoint.latitude()));
        }
    }
}

SlippyMapLayerTrack::~SlippyMapLayerTrack() {
}

void SlippyMapLayerTrack::draw(QPainter *painter, const QTransform &transform, SlippyMapLayerObject::ObjectState state)
{
    if (m_trackLineStrokeWidth > 0) {
        for (int i = 0; i < m_points.length(); i++) {
            QPointF thisPoint = transform.map(m_points.at(i));

            if ((i + 1) < m_points.length()) {
                QPointF nextPoint = transform.map(m_points.at(i + 1));
                painter->setPen(m_strokePen);
                painter->setBrush(Qt::NoBrush);
                painter->drawLine(thisPoint, nextPoint);
            }
        }
    }
    for (int i = 0; i < m_points.length(); i++) {
        QPointF thisPoint = transform.map(m_points.at(i));

        if ((i + 1) < m_points.length()) {
            QPointF nextPoint = transform.map(m_points.at(i + 1));
            painter->setPen(m_linePen);
            painter->setBrush(Qt::NoBrush);
            painter->drawLine(thisPoint, nextPoint);
        }

        painter->setPen(m_dotPen);
        painter->setBrush(Qt::NoBrush);
        painter->drawPoint(thisPoint);
    }
}

bool SlippyMapLayerTrack::isIntersectedBy(const QRectF& rect) const
{
    for (QPointF point : m_points) {
        if (rect.contains(point)) {
            return true;
        }
    }

    return false;
}

bool SlippyMapLayerTrack::contains(const QPointF& point, int zoom) const
{
    double deg_per_pixel = (360.0 / pow(2.0, zoom)) / 256.0;
    double deg_radius = deg_per_pixel * 10;

    for (int i = 0; i < m_points.length(); i++) {
        QRectF deg_rect(
                    m_points.at(i).x() - deg_radius,
                    m_points.at(i).y() - deg_radius,
                    deg_radius * 2,
                    deg_radius * 2);

        if (deg_rect.contains(point)) {
            return true;
        }

//        if ((i + 1) < m_points.length()) {
//            QPointF thisPoint = m_points.at(i);
//            QPointF nextPoint = m_points.at(i + 1);

//            qreal a = (nextPoint.y() - thisPoint.y()) / (nextPoint.x() - thisPoint.x());
//            qreal b = thisPoint.y() - (a * thisPoint.x());

//            //
//            // check if point lies within line up/down
//            // then check if point lies within x on either side of where it interects line
//            //

//        }
    }

    return false;
}

bool SlippyMapLayerTrack::isMovable()
{
    return false;
}

const QPointF SlippyMapLayerTrack::position() const
{
    return m_points.at(0);
}

void SlippyMapLayerTrack::setPosition(const QPointF &position) {

}

const QSizeF SlippyMapLayerTrack::size() const
{
    QPolygonF poly(m_points);
    QRectF boundingRect = poly.boundingRect();
    return boundingRect.size();
}

const GPXTrack& SlippyMapLayerTrack::track() {
    return m_track;
}

void SlippyMapLayerTrack::initStyle()
{
    m_dotPen.setColor(m_waypointColor);
    m_dotPen.setStyle(Qt::SolidLine);
    m_dotPen.setWidth(m_waypointRadius);

    m_linePen.setStyle(Qt::SolidLine);
    m_linePen.setColor(m_trackLineColor);
    m_linePen.setWidth(m_trackLineWidth);

    m_strokePen.setStyle(Qt::SolidLine);
    m_strokePen.setColor(m_trackLineStrokeColor);
    m_strokePen.setWidth(m_trackLineWidth + (m_trackLineStrokeWidth * 2));
}

QDataStream &SlippyMapLayerTrack::serialize(QDataStream &stream) const
{
    return stream;
}

void SlippyMapLayerTrack::unserialize(QDataStream &stream)
{

}

void SlippyMapLayerTrack::setTrackLineWidth(int width)
{
    m_trackLineWidth = width;
    initStyle();
    emit updated();
}

void SlippyMapLayerTrack::setTrackLineColor(const QColor &color)
{
    m_trackLineColor = color;
    initStyle();
    emit updated();
}

void SlippyMapLayerTrack::setTrackLineStrokeWidth(int width)
{
    m_trackLineStrokeWidth = width;
    initStyle();
    emit updated();
}

void SlippyMapLayerTrack::setTrackLineStrokeColor(const QColor &color)
{
    m_trackLineStrokeColor = color;
    initStyle();
    emit updated();
}

void SlippyMapLayerTrack::setWaypointColor(const QColor &color)
{
    m_waypointColor = color;
    initStyle();
    emit updated();
}

void SlippyMapLayerTrack::setWaypointRadius(int radius)
{
    m_waypointRadius = radius;
    initStyle();
    emit updated();
}

int SlippyMapLayerTrack::trackLineWidth() const
{
    return m_trackLineWidth;
}

const QColor & SlippyMapLayerTrack::trackLineColor() const
{
    return m_trackLineColor;
}

int SlippyMapLayerTrack::trackLineStrokeWidth() const
{
    return m_trackLineStrokeWidth;
}

const QColor & SlippyMapLayerTrack::trackLineStrokeColor() const
{
    return m_trackLineStrokeColor;
}

const QColor & SlippyMapLayerTrack::waypointColor() const
{
    return m_waypointColor;
}

int SlippyMapLayerTrack::waypointRadius() const
{
    return m_waypointRadius;
}

QList<SlippyMapLayerObjectPropertyPage*> SlippyMapLayerTrack::propertyPages() const
{
    return {
        new SlippyMapLayerTrackPropertyPage((SlippyMapLayerObject *) this),
        new SlippyMapLayerTrackStylePropertyPage((SlippyMapLayerObject *)this)
    };
}
