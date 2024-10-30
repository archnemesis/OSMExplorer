#ifndef GPXTRACKSEGMENT_H
#define GPXTRACKSEGMENT_H

#include <QObject>

class GPXWaypoint;

class GPXTrackSegment : public QObject
{
    Q_OBJECT
public:
    explicit GPXTrackSegment(QObject *parent = nullptr);
    GPXTrackSegment(const GPXTrackSegment& other);
    GPXTrackSegment& operator=(const GPXTrackSegment& other);
    ~GPXTrackSegment();
    void addPoint(const GPXWaypoint& point);
    void clearPoints();
    const QList<GPXWaypoint>& points() const;
private:
    QList<GPXWaypoint> m_points;
};

#endif // GPXTRACKSEGMENT_H
