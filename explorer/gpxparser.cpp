#include "gpxparser.h"
#include "gpxtrack.h"
#include "gpxtracksegment.h"
#include "gpxtracklink.h"
#include "gpxwaypoint.h"
#include "gpxmetadata.h"
#include "gpxperson.h"
#include "gpxcopyright.h"

#include <QDateTime>

GPXParser::GPXParser(QObject *parent) : QObject(parent)
{

}

bool GPXParser::read(QIODevice *device)
{
    m_xml.setDevice(device);

    if (m_xml.readNextStartElement()) {
        if (m_xml.name() == QLatin1String("gpx")
                && m_xml.attributes().value(versionAttribute()) == QLatin1String("1.1")) {
            readGPX();
        }
        else {
            m_xml.raiseError(tr("The file is not a valid GPX 1.1 file."));
        }
    }

    return !m_xml.error();
}

GPXMetadata *GPXParser::metadata()
{
    return m_metadata;
}

QList<GPXTrack *> GPXParser::tracks()
{
    return m_tracks;
}

QList<GPXWaypoint *> GPXParser::waypoints()
{
    return m_waypoints;
}

void GPXParser::readGPX()
{
    Q_ASSERT(m_xml.isStartElement() && m_xml.name() == QLatin1String("gpx"));

    while (m_xml.readNextStartElement()) {
        if (m_xml.name() == QLatin1String("trk")) {
            m_tracks.append(readTrack());
        }
        else if (m_xml.name() == QLatin1String("wpt")) {
            m_waypoints.append(readTrackPoint());
        }
        else if (m_xml.name() == QLatin1String("metadata")) {
            m_metadata = readMetadata();
        }
        else {
            m_xml.skipCurrentElement();
        }
    }
}

GPXMetadata *GPXParser::readMetadata()
{
    Q_ASSERT(m_xml.isStartElement() && m_xml.name() == QLatin1String("metadata"));

    GPXMetadata *metadata = new GPXMetadata();

    while (m_xml.readNextStartElement()) {
        if (m_xml.name() == QLatin1String("name")) {
            metadata->setName(m_xml.readElementText());
        }
        else if (m_xml.name() == QLatin1String("desc")) {
            metadata->setDescription(m_xml.readElementText());
        }
        else if (m_xml.name() == QLatin1String("author")) {
            metadata->setAuthor(readAuthor());
        }
        else if (m_xml.name() == QLatin1String("copyright")) {
            metadata->setCopyright(readCopyright());
        }
        else if (m_xml.name() == QLatin1String("link")) {
            metadata->addLink(readTrackPointLink());
        }
        else if (m_xml.name() == QLatin1String("time")) {
            QDateTime time = QDateTime::fromString(m_xml.readElementText(), Qt::ISODate);
            metadata->setTime(time);
        }
        else if (m_xml.name() == QLatin1String("keywords")) {
            metadata->setKeywords(m_xml.readElementText());
        }
        else {
            m_xml.skipCurrentElement();
        }
    }

    return metadata;
}

GPXTrack *GPXParser::readTrack()
{
    Q_ASSERT(m_xml.isStartElement() && m_xml.name() == QLatin1String("trk"));

    GPXTrack *track = new GPXTrack();

    while (m_xml.readNextStartElement()) {
        if (m_xml.name() == QLatin1String("name")) {
            track->setName(m_xml.readElementText());
        }
        else if (m_xml.name() == QLatin1String("cmt")) {
            track->setComment(m_xml.readElementText());
        }
        else if (m_xml.name() == QLatin1String("desc")) {
            track->setDescription(m_xml.readElementText());
        }
        else if (m_xml.name() == QLatin1String("link")) {
            track->addLink(readTrackPointLink());
        }
        else if (m_xml.name() == QLatin1String("number")) {
            track->setNumber(m_xml.readElementText().toInt());
        }
        else if (m_xml.name() == QLatin1String("type")) {
            track->setType(m_xml.readElementText());
        }
        else if (m_xml.name() == QLatin1String("trkseg")) {
            track->addSegment(readTrackSegment());
        }
        else {
            m_xml.skipCurrentElement();
        }
    }

    return track;
}

GPXTrackSegment *GPXParser::readTrackSegment()
{
    Q_ASSERT(m_xml.isStartElement() && m_xml.name() == QLatin1String("trkseg"));

    GPXTrackSegment *segment = new GPXTrackSegment();

    while (m_xml.readNextStartElement()) {
        if (m_xml.name() == QLatin1String("trkpt")) {
            segment->addPoint(readTrackPoint());
        }
        else {
            m_xml.skipCurrentElement();
        }
    }

    return segment;
}

GPXWaypoint *GPXParser::readTrackPoint()
{
    Q_ASSERT(m_xml.isStartElement() &&
             (m_xml.name() == QLatin1String("trkpt") || m_xml.name() == QLatin1String("wpt")));

    GPXWaypoint *point = new GPXWaypoint();

    point->setLatitude(m_xml.attributes().value(QLatin1String("lat")).toDouble());
    point->setLongitude(m_xml.attributes().value(QLatin1String("lon")).toDouble());

    while (m_xml.readNextStartElement()) {
        if (m_xml.name() == QLatin1String("ele")) {
            point->setElevation(m_xml.readElementText().toDouble());
        }
        else if (m_xml.name() == QLatin1String("time")) {
            QDateTime time = QDateTime::fromString(m_xml.readElementText(), Qt::ISODate);
            point->setTime(time);
        }
        else if (m_xml.name() == QLatin1String("magvar")) {
            point->setMagneticVariation(m_xml.readElementText().toDouble());
        }
        else if (m_xml.name() == QLatin1String("geoidheight")) {
            point->setGeoidHeight(m_xml.readElementText().toDouble());
        }
        else if (m_xml.name() == QLatin1String("name")) {
            point->setName(m_xml.readElementText());
        }
        else if (m_xml.name() == QLatin1String("cmt")) {
            point->setComment(m_xml.readElementText());
        }
        else if (m_xml.name() == QLatin1String("desc")) {
            point->setDescription(m_xml.readElementText());
        }
        else if (m_xml.name() == QLatin1String("src")) {
            point->setSource(m_xml.readElementText());
        }
        else if (m_xml.name() == QLatin1String("link")) {
            point->addLink(readTrackPointLink());
        }
        else if (m_xml.name() == QLatin1String("sym")) {
            point->setSymbol(m_xml.readElementText());
        }
        else if (m_xml.name() == QLatin1String("type")) {
            point->setType(m_xml.readElementText());
        }
        else if (m_xml.name() == QLatin1String("fix")) {
            point->setFixType(m_xml.readElementText());
        }
        else if (m_xml.name() == QLatin1String("sat")) {
            point->setNumSatellites(m_xml.readElementText().toInt());
        }
        else if (m_xml.name() == QLatin1String("hdop")) {
            point->setHdop(m_xml.readElementText().toDouble());
        }
        else if (m_xml.name() == QLatin1String("vdop")) {
            point->setVdop(m_xml.readElementText().toDouble());
        }
        else if (m_xml.name() == QLatin1String("pdop")) {
            point->setPdop(m_xml.readElementText().toDouble());
        }
        else if (m_xml.name() == QLatin1String("ageofgpsdata")) {
            point->setAgeOfGpsData(m_xml.readElementText().toDouble());
        }
        else if (m_xml.name() == QLatin1String("dgpsid")) {
            point->setDgpsid(m_xml.readElementText());
        }
        else {
            m_xml.skipCurrentElement();
        }
    }

    return point;
}

GPXTrackLink *GPXParser::readTrackPointLink()
{
    Q_ASSERT(m_xml.isStartElement() && m_xml.name() == QLatin1String("link"));

    GPXTrackLink *link = new GPXTrackLink();

    link->setHref(m_xml.attributes().value(QLatin1String("href")).toString());

    while (m_xml.readNextStartElement()) {
        if (m_xml.name() == QLatin1String("text")) {
            link->setText(m_xml.readElementText());
        }
        else if (m_xml.name() == QLatin1String("type")) {
            link->setType(m_xml.readElementText());
        }
        else {
            m_xml.skipCurrentElement();
        }
    }

    return link;
}

GPXPerson *GPXParser::readAuthor()
{
    Q_ASSERT(m_xml.isStartElement() && m_xml.name() == QLatin1String("author"));

    GPXPerson *author = new GPXPerson();

    while (m_xml.readNextStartElement()) {
        if (m_xml.name() == QLatin1String("name")) {
            author->setName(m_xml.readElementText());
        }
        else if (m_xml.name() == QLatin1String("email")) {
            author->setEmail(readEmail());
        }
        else if (m_xml.name() == QLatin1String("link")) {
            author->setLink(readTrackPointLink());
        }
        else {
            m_xml.skipCurrentElement();
        }
    }

    return author;
}

GPXCopyright *GPXParser::readCopyright()
{
    GPXCopyright *copyright = new GPXCopyright();

    copyright->setAuthor(m_xml.attributes().value("author").toString());

    while (m_xml.readNextStartElement()) {
        if (m_xml.name() == QLatin1String("year")) {
            copyright->setYear(m_xml.readElementText());
        }
        else if (m_xml.name() == QLatin1String("license")) {
            copyright->setLicense(m_xml.readElementText());
        }
        else {
            m_xml.skipCurrentElement();
        }
    }

    return copyright;
}

QString GPXParser::readEmail()
{
    Q_ASSERT(m_xml.isStartElement() && m_xml.name() == QLatin1String("email"));

    QString email;

    while (m_xml.readNextStartElement()) {
        if (m_xml.name() == QLatin1String("id")) {
            email.append(m_xml.readElementText());
            email.append("@");
        }
        else if (m_xml.name() == QLatin1String("domain")) {
            email.append(m_xml.readElementText());
        }
        else {
            m_xml.skipCurrentElement();
        }
    }

    return email;
}
