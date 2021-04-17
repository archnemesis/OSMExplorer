#include "gpxtrack.h"
#include "gpxtracklink.h"
#include "gpxtracksegment.h"

GPXTrack::GPXTrack(QObject *parent) : QObject(parent)
{

}

GPXTrack::~GPXTrack()
{
    for (GPXTrackLink *link : m_links) {
        delete link;
    }
    m_links.clear();

    for (GPXTrackSegment *segment : m_segments) {
        delete segment;
    }
    m_segments.clear();
}

QList<GPXTrackSegment *> GPXTrack::segments() const
{
    return m_segments;
}

void GPXTrack::setName(const QString &name)
{
    m_name = name;
}

void GPXTrack::setComment(const QString &comment)
{
    m_comment = comment;
}

void GPXTrack::setDescription(const QString &description)
{
    m_description = description;
}

void GPXTrack::setSource(const QString &source)
{
    m_source = source;
}

void GPXTrack::setNumber(int number)
{
    m_number = number;
}

void GPXTrack::setType(const QString &type)
{
    m_type = type;
}

void GPXTrack::addLink(GPXTrackLink *link)
{
    m_links.append(link);
}

void GPXTrack::addSegment(GPXTrackSegment *segment)
{
    m_segments.append(segment);
}
