#include "gpxtracklink.h"

GPXTrackLink::GPXTrackLink(QObject *parent) :
    QObject(parent),
    m_href(""),
    m_text(""),
    m_type("")
{

}

GPXTrackLink::GPXTrackLink(QString href, QString text, QString type, QObject *parent) :
    QObject(parent),
    m_href(href),
    m_text(text),
    m_type(type)
{

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
