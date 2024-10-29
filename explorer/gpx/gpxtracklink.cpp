#include "gpxtracklink.h"

GPXTrackLink::GPXTrackLink(const GPXTrackLink& other) :
    m_href(other.href()),
    m_text(other.text()),
    m_type(other.type()) {
}

GPXTrackLink& GPXTrackLink::operator=(const GPXTrackLink& other) {
    setHref(other.href());
    setText(other.text());
    setType(other.type());
    return *this;
}

QString GPXTrackLink::href() const
{
    return m_href;
}

QString GPXTrackLink::text() const
{
    return m_text;
}

QString GPXTrackLink::type() const
{
    return m_type;
}

void GPXTrackLink::setHref(QString href)
{
    m_href = href;
}

void GPXTrackLink::setText(QString text)
{
    m_text = text;
}

void GPXTrackLink::setType(QString type)
{
    m_type = type;
}
