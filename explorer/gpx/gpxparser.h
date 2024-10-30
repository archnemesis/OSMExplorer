#ifndef GPXPARSER_H
#define GPXPARSER_H

#include <QObject>
#include <QIODevice>
#include <QXmlStreamReader>
#include <QStringLiteral>

#include "gpxmetadata.h"
#include "gpxtrack.h"
#include "gpxwaypoint.h"

class GPXParser : public QObject
{
    Q_OBJECT
public:
    explicit GPXParser(QObject *parent = nullptr);

    bool read(QIODevice *device);
    QString errorString() const;

    static inline QString versionAttribute() { return QStringLiteral("version"); }
    static inline QString creatorAttribute() { return QStringLiteral("creator"); }

    const GPXMetadata& metadata();
    const QList<GPXTrack>& tracks();
    const QList<GPXWaypoint>& waypoints();

private:
    QXmlStreamReader m_xml;

    GPXMetadata m_metadata;
    QList<GPXTrack> m_tracks;
    QList<GPXWaypoint> m_waypoints;

    void readGPX();
    GPXMetadata readMetadata();
    GPXTrack readTrack();
    GPXTrackSegment readTrackSegment();
    GPXWaypoint readTrackPoint();
    GPXTrackLink readTrackPointLink();
    GPXPerson readAuthor();
    GPXCopyright readCopyright();
    QString readEmail();
};

#endif // GPXPARSER_H
