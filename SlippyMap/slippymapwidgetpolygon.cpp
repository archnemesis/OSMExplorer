#include "slippymapwidgetpolygon.h"
#define NOMINMAX
#include <algorithm>

using namespace std;

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
    QPointF a, b;
    int count = 0;

    for (int i = 1; i < m_points.length(); i++) {
        a = m_points.at(i - 1);
        b = m_points.at(i);
        if (test_point(a, b, point)) {
            count++;
        }
    }

    return count % 2 != 0;
}

bool SlippyMapWidgetPolygon::test_point(QPointF a, QPointF b, QPointF p)
{
    const qreal epsilon = numeric_limits<qreal>().epsilon();
    const numeric_limits<qreal> DOUBLE;
    const qreal MIN = DOUBLE.min();
    const qreal MAX = DOUBLE.max();

    if (a.y() > b.y()) return test_point(b, a, p);
    if (p.y() == a.y() || p.y() == b.y()) return test_point(a, b, QPointF(p.x(), p.y() + epsilon));
    if (p.y() > b.y() || p.y() < a.y() || p.x() > max(a.x(), b.x())) return false;
    if (p.x() < min(a.x(), b.x())) return true;
    double blue = abs(a.x() - p.x()) > MIN ? (p.y() - a.y()) / (p.x() - a.x()) : MAX;
    double red  = abs(a.x() - b.x()) > MIN ? (b.y() - a.y()) / (b.x() - a.x()) : MAX;
    return blue >= red;
}
