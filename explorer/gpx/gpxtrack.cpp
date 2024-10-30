#include "gpxtrack.h"
#include "gpxtracklink.h"
#include "gpxtracksegment.h"

GPXTrack::GPXTrack(QObject* parent) {
}

GPXTrack::GPXTrack(const GPXTrack& other) {
    setName(other.name());
    setDescription(other.description());
    setType(other.type());
    setComment(other.comment());
    setNumber(other.number());
    setSource(other.source());
    for (const auto& segment : other.segments())
        addSegment(segment);
}

GPXTrack& GPXTrack::operator=(const GPXTrack& other) {
    setName(other.name());
    setDescription(other.description());
    setType(other.type());
    setComment(other.comment());
    setNumber(other.number());
    setSource(other.source());
    for (const auto& segment : other.segments())
        addSegment(segment);
    return *this;
}

GPXTrack::~GPXTrack()
{
    m_links.clear();
    m_segments.clear();
}

const QString& GPXTrack::name() const {
    return m_name;
}

const QString& GPXTrack::comment() const {
    return m_comment;
}

const QString& GPXTrack::description() const {
    return m_description;
}

const QString& GPXTrack::source() const {
    return m_source;
}

int GPXTrack::number() const {
    return m_number;
}

const QString& GPXTrack::type() const {
    return m_type;
}

const QList<GPXTrackLink>& GPXTrack::links() const {
    return m_links;
}

const QList<GPXTrackSegment>& GPXTrack::segments() const
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

void GPXTrack::addLink(const GPXTrackLink& link)
{
    m_links.append(link);
}

void GPXTrack::addSegment(const GPXTrackSegment& segment)
{
    m_segments.append(segment);
}
