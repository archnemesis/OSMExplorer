#ifndef GPXTRACKSEGMENT_H
#define GPXTRACKSEGMENT_H

#include <QObject>

class GPXWaypoint;

class GPXTrackSegment : public QObject
{
    Q_OBJECT
public:
    explicit GPXTrackSegment(QObject *parent = nullptr);
    ~GPXTrackSegment();
    void addPoint(GPXWaypoint *point);
    void clearPoints();
    QList<GPXWaypoint*> points();
private:
    QList<GPXWaypoint*> m_points;
};

#endif // GPXTRACKSEGMENT_H
