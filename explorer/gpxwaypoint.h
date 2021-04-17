#ifndef GPXTRACKPOINT_H
#define GPXTRACKPOINT_H

#include <QObject>
#include <QDateTime>

class GPXTrackLink;

class GPXWaypoint : public QObject
{
    Q_OBJECT
public:
    explicit GPXWaypoint(QObject *parent = nullptr);
    ~GPXWaypoint();

    qreal latitude() const;
    void setLatitude(const qreal &latitude);

    qreal longitude() const;
    void setLongitude(const qreal &longitude);

    qreal elevation() const;
    void setElevation(const qreal &elevation);

    QDateTime time() const;
    void setTime(const QDateTime &time);

    qreal magneticVariation() const;
    void setMagneticVariation(const qreal &magvar);

    qreal geoidHeight() const;
    void setGeoidHeight(const qreal &geoidHeight);

    QString name() const;
    void setName(const QString &name);

    QString comment() const;
    void setComment(const QString &comment);

    QString description() const;
    void setDescription(const QString &description);

    QString source() const;
    void setSource(const QString &source);

    QString symbol() const;
    void setSymbol(const QString &symbol);

    QString type() const;
    void setType(const QString &type);

    QString fixType() const;
    void setFixType(const QString &fixType);

    int numSatellites() const;
    void setNumSatellites(int numSatellites);

    qreal hdop() const;
    void setHdop(const qreal &hdop);

    qreal vdop() const;
    void setVdop(const qreal &vdop);

    qreal pdop() const;
    void setPdop(const qreal &pdop);

    qreal ageOfGpsData() const;
    void setAgeOfGpsData(const qreal &ageOfGpsData);

    QString dgpsid() const;
    void setDgpsid(const QString &dgpsid);

    void addLink(GPXTrackLink *link);

private:
    qreal m_latitude;
    qreal m_longitude;
    qreal m_elevation;
    QDateTime m_time;
    qreal m_magvar;
    qreal m_geoidHeight;
    QString m_name;
    QString m_comment;
    QString m_description;
    QString m_source;
    QList<GPXTrackLink*> m_links;
    QString m_symbol;
    QString m_type;
    QString m_fixType;
    int m_numSatellites;
    qreal m_hdop;
    qreal m_vdop;
    qreal m_pdop;
    qreal m_ageOfGpsData;
    QString m_dgpsid;
};

#endif // GPXTRACKPOINT_H
