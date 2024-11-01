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
    explicit SlippyMapLayerTrack(const GPXTrack& track);
    ~SlippyMapLayerTrack() override;
    QDataStream& serialize(QDataStream& stream) const override;
    QList<SlippyMapLayerObjectPropertyPage*> propertyPages() const;
    bool contains(const QPointF& point, int zoom) const override;
    bool isIntersectedBy(const QRectF& rect) const override;
    bool isMovable() override;
    const GPXTrack& track();
    const QPointF position() const override;
    const QSizeF size() const override;
    void draw(QPainter *painter, const QTransform &transform, ObjectState state) override;
    void setPosition(const QPointF& position) override;
    void unserialize(QDataStream& stream) override;
    void setTrackLineWidth(int width);
    void setTrackLineColor(const QColor& color);
    void setTrackLineStrokeWidth(int width);
    void setTrackLineStrokeColor(const QColor& color);
    void setWaypointColor(const QColor& color);
    void setWaypointRadius(int radius);
    int trackLineWidth() const;
    const QColor& trackLineColor() const;
    int trackLineStrokeWidth() const;
    const QColor& trackLineStrokeColor() const;
    const QColor& waypointColor() const;
    int waypointRadius() const;

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

    GPXTrack m_track;

protected:
    QVector<QPointF> m_points;
};

#endif // SLIPPYMAPLAYERROUTE_H
