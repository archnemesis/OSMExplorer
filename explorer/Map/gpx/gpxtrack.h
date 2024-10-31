#ifndef GPXTRACK_H
#define GPXTRACK_H

#include <QObject>

class GPXTrackSegment;
class GPXTrackLink;

class GPXTrack : public QObject
{
    Q_OBJECT

public:
    explicit GPXTrack(QObject *parent = nullptr);
    GPXTrack(const GPXTrack& other);
    GPXTrack& operator=(const GPXTrack& other);
    ~GPXTrack();
    const QString& name() const;
    const QString& comment() const;
    const QString& description() const;
    const QString& source() const;
    int number() const;
    const QString& type() const;
    const QList<GPXTrackLink>& links() const;
    const QList<GPXTrackSegment>& segments() const;

    void setName(const QString &name);
    void setComment(const QString &comment);
    void setDescription(const QString &description);
    void setSource(const QString &source);
    void setNumber(int number);
    void setType(const QString &type);
    void addLink(const GPXTrackLink& link);
    void addSegment(const GPXTrackSegment& segment);

protected:
    QString m_name;
    QString m_comment;
    QString m_description;
    QString m_source;
    int m_number;
    QString m_type;
    QList<GPXTrackLink> m_links;
    QList<GPXTrackSegment> m_segments;
};

#endif // GPXTRACK_H
