#include "gpxtracksegment.h"
#include "gpxwaypoint.h"

GPXTrackSegment::GPXTrackSegment(QObject *parent) : QObject(parent)
{

}

GPXTrackSegment::GPXTrackSegment(const GPXTrackSegment& other)
{
    for (const auto& point : other.points())
        m_points.append(point);
}

GPXTrackSegment& GPXTrackSegment::operator=(const GPXTrackSegment& other) {
    for (const auto& point : other.points())
        m_points.append(point);
    return *this;
}

GPXTrackSegment::~GPXTrackSegment()
{
    clearPoints();
}

void GPXTrackSegment::addPoint(const GPXWaypoint& point)
{
    m_points.append(point);
}

void GPXTrackSegment::clearPoints()
{
    m_points.clear();
}

const QList<GPXWaypoint>& GPXTrackSegment::points() const
{
    return m_points;
}
