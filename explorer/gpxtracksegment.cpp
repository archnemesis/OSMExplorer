#include "gpxtracksegment.h"
#include "gpxwaypoint.h"

GPXTrackSegment::GPXTrackSegment(QObject *parent) : QObject(parent)
{

}

GPXTrackSegment::~GPXTrackSegment()
{
    clearPoints();
}

void GPXTrackSegment::addPoint(GPXWaypoint *point)
{
    m_points.append(point);
}

void GPXTrackSegment::clearPoints()
{
    for (GPXWaypoint *point : m_points) {
        delete point;
    }

    m_points.clear();
}

QList<GPXWaypoint *> GPXTrackSegment::points()
{
    return m_points;
}
