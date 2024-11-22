#ifndef SLIPPYMAPLAYERROUTE_H
#define SLIPPYMAPLAYERROUTE_H

#include <QVector>
#include <SlippyMap/SlippyMapLayerObject.h>
#include "gpx/gpxtrack.h"


using namespace SlippyMap;

class SlippyMapLayerObjectPropertyPage;

class SlippyMapLayerTrack : public SlippyMapLayerObject
{
    Q_OBJECT
public:
    typedef QSharedPointer<SlippyMapLayerTrack> Ptr;
    explicit Q_INVOKABLE SlippyMapLayerTrack(QObject *parent = nullptr);
    explicit SlippyMapLayerTrack(const GPXTrack& track);
    Q_INVOKABLE SlippyMapLayerTrack(const SlippyMapLayerTrack& other);

    virtual SlippyMapLayerTrack* clone() const;
    virtual void copy(SlippyMapLayerObject *other);
    void hydrateFromDatabase(const QJsonObject& json, const QString& geometry) override;
    void packageObjectData(QJsonObject& json, QString& geometry) override;
    QDataStream& serialize(QDataStream& stream) const override;
    QList<SlippyMapLayerObjectPropertyPage*> propertyPages(SlippyMapLayerObject::Ptr object) const;
    bool contains(const QPointF& point, int zoom) const override;
    bool isIntersectedBy(const QRectF& rect) const override;
    bool isMovable() override;
    const GPXTrack& track();
    const QPointF position() const override;
    const QSizeF size() const override;
    const QVector<QPointF>& points() const;
    void setPoints(const QVector<QPointF>& points);
    void draw(QPainter *painter, const QTransform &transform, ObjectState state) override;
    void setPosition(const QPointF& position) override;
    void unserialize(QDataStream& stream) override;
    void setLineWidth(int width);
    void setLineColor(const QColor& color);
    void setStrokeWidth(int width);
    void setStrokeColor(const QColor& color);
    void setWaypointColor(const QColor& color);
    void setWaypointRadius(int radius);
    void setWaypointsVisible(bool visible);
    int lineWidth() const;
    const QColor& lineColor() const;
    int strokeWidth() const;
    const QColor& strokeColor() const;
    const QColor& waypointColor() const;
    int waypointRadius() const;
    bool waypointsVisible() const;
    void appendPoint(const QPointF& point);

private:
    void initStyle();

    QBrush m_strokeBrush;
    QPen m_dotPen;
    QPen m_linePen;
    QPen m_strokePen;

    QColor m_waypointColor;
    QColor m_trackLineColor;
    QColor m_trackLineStrokeColor;
    int m_trackLineWidth;
    int m_trackLineStrokeWidth;
    int m_waypointRadius;
    bool m_waypointsVisible;

    GPXTrack m_track;

protected:
    QVector<QPointF> m_points;
};

#endif // SLIPPYMAPLAYERROUTE_H
