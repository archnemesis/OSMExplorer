#ifndef SLIPPYMAPWIDGETPOLYGON_H
#define SLIPPYMAPWIDGETPOLYGON_H

#include "slippymap_global.h"
#include "slippymaplayerobject.h"

#include <QObject>
#include <QVector>
#include <QPen>
#include <QBrush>
#include <QPainter>

class SlippyMapLayerObjectPropertyPage;

class SLIPPYMAPSHARED_EXPORT SlippyMapLayerPolygon : public SlippyMapLayerObject
{
    Q_OBJECT
public:
    explicit SlippyMapLayerPolygon(QVector<QPointF> points, QObject *parent = nullptr);
    ~SlippyMapLayerPolygon();
    void draw(QPainter *painter, const QTransform &transform, ObjectState state = NormalState);
    QVector<QPointF> points();
    bool isIntersectedBy(QRectF rect);
    bool contains(QPointF point, int zoom);
    QPointF position();
    QSizeF size();
    SlippyMapLayerObjectPropertyPage *propertyPage(QWidget *parent = nullptr);

protected:
    QVector<QPointF> m_points;

    bool test_point(QPointF a, QPointF b, QPointF p);
};

#endif // SLIPPYMAPWIDGETPOLYGON_H
