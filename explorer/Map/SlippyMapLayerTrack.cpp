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

SlippyMapLayerTrack::SlippyMapLayerTrack(QObject *parent) : SlippyMapLayerObject(parent),
                                                            m_trackLineWidth(5),
                                                            m_trackLineStrokeWidth(1),
                                                            m_trackLineColor(Qt::green),
                                                            m_trackLineStrokeColor(Qt::darkGreen),
                                                            m_waypointRadius(5),
                                                            m_waypointColor(Qt::darkYellow)
{

}

SlippyMapLayerTrack::SlippyMapLayerTrack(const GPXTrack& track) : SlippyMapLayerTrack()
{
    m_track = track;
    initStyle();

    for (const GPXTrackSegment& segment : track.segments()) {
        for (const GPXWaypoint& waypoint : segment.points()) {
            m_points.append(QPointF(waypoint.longitude(), waypoint.latitude()));
        }
    }
}

QDataStream &SlippyMapLayerTrack::serialize(QDataStream &stream) const
{
    stream << label();
    stream << description();
    stream << trackLineWidth();
    stream << trackLineColor();
    stream << trackLineStrokeWidth();
    stream << trackLineStrokeColor();
    stream << waypointRadius();
    stream << waypointColor();
    stream << points();
    return stream;
}

void SlippyMapLayerTrack::unserialize(QDataStream &stream)
{
    QString label;
    QString description;
    int trackLineWidth;
    QColor trackLineColor;
    int trackLineStrokeWidth;
    QColor trackLineStrokeColor;
    int waypointRadius;
    QColor waypointColor;
    QVector<QPointF> points;

    stream >> label;
    stream >> description;
    stream >> trackLineWidth;
    stream >> trackLineColor;
    stream >> trackLineStrokeWidth;
    stream >> trackLineStrokeColor;
    stream >> waypointRadius;
    stream >> waypointColor;
    stream >> points;

    setLabel(label);
    setDescription(description);
    setTrackLineWidth(trackLineWidth);
    setTrackLineColor(trackLineColor);
    setTrackLineStrokeWidth(trackLineStrokeWidth);
    setTrackLineStrokeColor(trackLineStrokeColor);
    setWaypointRadius(waypointRadius);
    setWaypointColor(waypointColor);
    setPoints(points);
    initStyle();
}

SlippyMapLayerTrack::SlippyMapLayerTrack(const SlippyMapLayerTrack &other) : SlippyMapLayerTrack()
{
    setLabel(other.label());
    setDescription(other.description());
    setTrackLineWidth(other.trackLineWidth());
    setTrackLineColor(other.trackLineColor());
    setTrackLineStrokeWidth(other.trackLineStrokeWidth());
    setTrackLineStrokeColor(other.trackLineStrokeColor());
    setWaypointRadius(other.waypointRadius());
    setWaypointColor(other.waypointColor());
    setPoints(other.points());
}

void SlippyMapLayerTrack::copy(SlippyMapLayerObject *other)
{
    auto *track = dynamic_cast<SlippyMapLayerTrack*>(other);
    setLabel(track->label());
    setDescription(track->description());
    setTrackLineWidth(track->trackLineWidth());
    setTrackLineColor(track->trackLineColor());
    setTrackLineStrokeWidth(track->trackLineStrokeWidth());
    setTrackLineStrokeColor(track->trackLineStrokeColor());
    setWaypointRadius(track->waypointRadius());
    setWaypointColor(track->waypointColor());
    setPoints(track->points());
}

SlippyMapLayerTrack *SlippyMapLayerTrack::clone() const
{
    return new SlippyMapLayerTrack(*this);
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
    double deg_lineWidth = deg_per_pixel * m_trackLineStrokeWidth;

    for (int i = 0; i < m_points.length() - 1; i++) {
        QPointF p1 = m_points.at(i);
        QPointF p2 = m_points.at(i+1);

        //
        // the point should lie within the bounding rect of the line
        //
        QRectF bbox = {
                p1.x(),
                p1.y(),
                p2.x() - p1.x(),
                p2.y() - p1.y()};

        if (!bbox.contains(point)) continue;

        //
        // vertical line, so if point.x is within +/- deg_lineWidth of
        // the line, then it's a hit
        //
        if ((p2.x() - p1.x()) == 0) {
            if ((p1.x() - (deg_lineWidth / 2)) < point.x() && point.x() < (p1.x() + (deg_lineWidth / 2))) {
                return true;
            }
        }
            //
            // same for horizontal line
            //
        else if ((p2.y() - p1.y()) == 0) {
            if ((p1.y() - (deg_lineWidth / 2)) < point.y() && point.y() < (p1.y() + (deg_lineWidth / 2))) {
                return true;
            }
        }

        double slope = (p2.y() - p1.y()) / (p2.x() - p1.x());
        double intercept = p1.y() - slope * p1.x();

        double intersect_y = (slope * point.x()) + intercept;
        double intersect_x = (intersect_y - intercept) / slope;

        QRectF deg_rect(
                intersect_x - (deg_lineWidth),
                intersect_y - (deg_lineWidth),
                deg_lineWidth * 2,
                deg_lineWidth * 2);

        if (deg_rect.contains(point)) {
            return true;
        }
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
    m_linePen.setCapStyle(Qt::RoundCap);

    m_strokePen.setStyle(Qt::SolidLine);
    m_strokePen.setColor(m_trackLineStrokeColor);
    m_strokePen.setWidth(m_trackLineWidth + (m_trackLineStrokeWidth * 2));
    m_strokePen.setCapStyle(Qt::RoundCap);
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

const QVector<QPointF> &SlippyMapLayerTrack::points() const
{
    return m_points;
}

void SlippyMapLayerTrack::setPoints(const QVector<QPointF> &points)
{
    m_points = points;
}
