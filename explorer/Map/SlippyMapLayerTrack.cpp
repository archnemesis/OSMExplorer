#include "SlippyMapLayerTrack.h"
#include "SlippyMapLayerTrackPropertyPage.h"

#include <cmath>

#include <QDebug>

#include <QPainter>
#include <QPointF>
#include <QBrush>
#include <QPolygonF>

#include "gpx/gpxwaypoint.h"
#include "gpx/gpxtracksegment.h"

SlippyMapLayerTrack::SlippyMapLayerTrack(const GPXTrack& track) :
    m_trackLineWidth(1),
    m_trackLineStrokeWidth(1)
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
    for (int i = 0; i < m_points.length(); i++) {
        QPointF thisPoint = transform.map(m_points.at(i));

        if ((i + 1) < m_points.length()) {
            QPointF nextPoint = transform.map(m_points.at(i + 1));
            painter->setPen(m_linePen);
            painter->setBrush(m_lineBrush);
            painter->drawLine(thisPoint, nextPoint);
        }

        painter->setPen(m_dotPen);
        painter->setBrush(m_dotBrush);
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
    m_dotBrush.setColor(Qt::black);
    m_dotBrush.setStyle(Qt::SolidPattern);
    m_dotPen.setColor(Qt::green);
    m_dotPen.setStyle(Qt::SolidLine);
    m_dotPen.setWidth(8);

    m_linePen.setStyle(Qt::SolidLine);
    m_linePen.setColor(Qt::darkGray);
    m_linePen.setWidth(8);
}

QDataStream &SlippyMapLayerTrack::serialize(QDataStream &stream) const
{
    return stream;
}

void SlippyMapLayerTrack::unserialize(QDataStream &stream)
{

}

SlippyMapLayerObjectPropertyPage *SlippyMapLayerTrack::propertyPage() const
{
    return new SlippyMapLayerTrackPropertyPage((SlippyMapLayerObject *) this);
}
