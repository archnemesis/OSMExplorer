#ifndef SLIPPYMAPWIDGETPOLYGON_H
#define SLIPPYMAPWIDGETPOLYGON_H

#include "slippymap_global.h"
#include "slippymapwidgetshape.h"
#include "slippymapwidgetmarker.h"

#include <QObject>
#include <QVector>
#include <QPen>
#include <QBrush>
#include <QPainter>

class SLIPPYMAPSHARED_EXPORT SlippyMapWidgetPolygon : public SlippyMapWidgetShape
{
    Q_OBJECT
public:
    explicit SlippyMapWidgetPolygon(QVector<QPointF> points, QObject *parent = nullptr);
    ~SlippyMapWidgetPolygon();
    void draw(QPainter *painter, const QTransform &transform, ShapeState state = NormalState);
    QVector<QPointF> points();
    bool isIntersectedBy(QRectF rect);
    bool contains(QPointF point);

protected:
    QVector<QPointF> m_points;
};

#endif // SLIPPYMAPWIDGETPOLYGON_H