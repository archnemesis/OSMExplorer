#include "gpxwaypoint.h"
#include "gpxtracklink.h"

GPXWaypoint::GPXWaypoint(QObject *parent) : QObject(parent)
{

}

GPXWaypoint::~GPXWaypoint()
{
    for (GPXTrackLink *link : m_links) {
        delete link;
    }
    m_links.clear();

}

qreal GPXWaypoint::latitude() const
{
    return m_latitude;
}

void GPXWaypoint::setLatitude(const qreal &latitude)
{
    m_latitude = latitude;
}

qreal GPXWaypoint::longitude() const
{
    return m_longitude;
}

void GPXWaypoint::setLongitude(const qreal &longitude)
{
    m_longitude = longitude;
}

qreal GPXWaypoint::elevation() const
{
    return m_elevation;
}

void GPXWaypoint::setElevation(const qreal &elevation)
{
    m_elevation = elevation;
}

QDateTime GPXWaypoint::time() const
{
    return m_time;
}

void GPXWaypoint::setTime(const QDateTime &time)
{
    m_time = time;
}

qreal GPXWaypoint::magneticVariation() const
{
    return m_magvar;
}

void GPXWaypoint::setMagneticVariation(const qreal &magvar)
{
    m_magvar = magvar;
}

qreal GPXWaypoint::geoidHeight() const
{
    return m_geoidHeight;
}

void GPXWaypoint::setGeoidHeight(const qreal &geoidHeight)
{
    m_geoidHeight = geoidHeight;
}

QString GPXWaypoint::name() const
{
    return m_name;
}

void GPXWaypoint::setName(const QString &name)
{
    m_name = name;
}

QString GPXWaypoint::comment() const
{
    return m_comment;
}

void GPXWaypoint::setComment(const QString &comment)
{
    m_comment = comment;
}

QString GPXWaypoint::description() const
{
    return m_description;
}

void GPXWaypoint::setDescription(const QString &description)
{
    m_description = description;
}

QString GPXWaypoint::source() const
{
    return m_source;
}

void GPXWaypoint::setSource(const QString &source)
{
    m_source = source;
}

QString GPXWaypoint::symbol() const
{
    return m_symbol;
}

void GPXWaypoint::setSymbol(const QString &symbol)
{
    m_symbol = symbol;
}

QString GPXWaypoint::type() const
{
    return m_type;
}

void GPXWaypoint::setType(const QString &type)
{
    m_type = type;
}

QString GPXWaypoint::fixType() const
{
    return m_fixType;
}

void GPXWaypoint::setFixType(const QString &fixType)
{
    m_fixType = fixType;
}

int GPXWaypoint::numSatellites() const
{
    return m_numSatellites;
}

void GPXWaypoint::setNumSatellites(int numSatellites)
{
    m_numSatellites = numSatellites;
}

qreal GPXWaypoint::hdop() const
{
    return m_hdop;
}

void GPXWaypoint::setHdop(const qreal &hdop)
{
    m_hdop = hdop;
}

qreal GPXWaypoint::vdop() const
{
    return m_vdop;
}

void GPXWaypoint::setVdop(const qreal &vdop)
{
    m_vdop = vdop;
}

qreal GPXWaypoint::pdop() const
{
    return m_pdop;
}

void GPXWaypoint::setPdop(const qreal &pdop)
{
    m_pdop = pdop;
}

qreal GPXWaypoint::ageOfGpsData() const
{
    return m_ageOfGpsData;
}

void GPXWaypoint::setAgeOfGpsData(const qreal &ageOfGpsData)
{
    m_ageOfGpsData = ageOfGpsData;
}

QString GPXWaypoint::dgpsid() const
{
    return m_dgpsid;
}

void GPXWaypoint::setDgpsid(const QString &dgpsid)
{
    m_dgpsid = dgpsid;
}

void GPXWaypoint::addLink(GPXTrackLink *link)
{
    m_links.append(link);
}
