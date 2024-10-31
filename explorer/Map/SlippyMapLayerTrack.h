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
    void draw(QPainter *painter, const QTransform &transform, ObjectState state) override;
    bool isIntersectedBy(const QRectF& rect) const override;
    bool contains(const QPointF& point, int zoom) const override;
    bool isMovable() override;
    const QPointF position() const override;
    void setPosition(const QPointF& position) override;
    const QSizeF size() const override;
    const GPXTrack& track();
    SlippyMapLayerObjectPropertyPage* propertyPage() const;
    QDataStream& serialize(QDataStream& stream) const override;
    void unserialize(QDataStream& stream) override;

private:
    void initStyle();

    QBrush m_dotBrush;
    QBrush m_lineBrush;
    QPen m_dotPen;
    QPen m_linePen;

    QColor m_trackLineColor;
    QColor m_trackLineStrokeColor;
    int m_trackLineWidth{};
    int m_trackLineStrokeWidth{};

    GPXTrack m_track;

protected:
    QVector<QPointF> m_points;
};

#endif // SLIPPYMAPLAYERROUTE_H
