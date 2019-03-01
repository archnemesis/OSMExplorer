#include "slippymapwidgetpolygon.h"

SlippyMapWidgetPolygon::SlippyMapWidgetPolygon(QVector<QPointF> points, QObject *parent) :
    SlippyMapWidgetShape(parent),
    m_points(points)
{

}

SlippyMapWidgetPolygon::~SlippyMapWidgetPolygon()
{

}

void SlippyMapWidgetPolygon::draw(QPainter *painter, const QTransform &transform, ShapeState state)
{
    painter->setPen(m_pen);
    painter->setBrush(m_brush);

    painter->save();
    painter->setWorldTransform(transform);
    painter->drawConvexPolygon(m_points.data(), m_points.length());
    painter->restore();

    if (state == SelectedState) {
        /* resize handles */
        for (QPointF point : m_points) {
            QPointF mapped = transform.map(point);
            drawResizeHandle(painter, QPoint(static_cast<int>(mapped.x()), static_cast<int>(mapped.y())));
        }
    }
}

QVector<QPointF> SlippyMapWidgetPolygon::points()
{
    return m_points;
}

bool SlippyMapWidgetPolygon::isIntersectedBy(QRectF rect)
{
    for (QPointF point : m_points) {
        if (rect.contains(point)) {
            return true;
        }
    }

    return false;
}

bool SlippyMapWidgetPolygon::contains(QPointF point)
{
    QPointF a = m_points.at(0);
    QPointF b = m_points.at(1);


}
