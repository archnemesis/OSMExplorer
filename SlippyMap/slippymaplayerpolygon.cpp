#include "slippymaplayerpolygon.h"
#include "slippymaplayerpolygonpropertypage.h"

#define NOMINMAX
#include <algorithm>
#include <QDebug>

using namespace std;

SlippyMapLayerPolygon::SlippyMapLayerPolygon(QVector<QPointF> points, QObject *parent) :
    SlippyMapLayerObject(parent),
    m_points(points)
{

}

SlippyMapLayerPolygon::~SlippyMapLayerPolygon()
{

}

void SlippyMapLayerPolygon::draw(QPainter *painter, const QTransform &transform, ObjectState state)
{
    switch (state) {
    case NormalState:
        painter->setPen(m_pen);
        painter->setBrush(m_brush);

        painter->save();
        painter->setWorldTransform(transform);
        painter->drawConvexPolygon(m_points.data(), m_points.length());
        painter->restore();

        break;
    case SelectedState:

        painter->save();

        painter->setPen(QPen(Qt::NoPen));
        painter->setBrush(m_selectedBrush);
        painter->setWorldTransform(transform);
        painter->drawConvexPolygon(m_points.data(), m_points.length());

        painter->setPen(m_selectedPen);
        painter->setBrush(QBrush(Qt::NoBrush));
        painter->drawConvexPolygon(m_points.data(), m_points.length());

        painter->restore();


        /* resize handles */
        for (QPointF point : m_points) {
            QPointF mapped = transform.map(point);
            drawResizeHandle(painter, QPoint(static_cast<int>(mapped.x()), static_cast<int>(mapped.y())));
        }
        break;
    default:
        break;
    }
}

QVector<QPointF> SlippyMapLayerPolygon::points()
{
    return m_points;
}

bool SlippyMapLayerPolygon::isIntersectedBy(QRectF rect)
{
    for (QPointF point : m_points) {
        if (rect.contains(point)) {
            return true;
        }
    }

    return false;
}

bool SlippyMapLayerPolygon::contains(QPointF point, int zoom)
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

    if (test_point(m_points.last(), m_points.first(), point)) {
        count++;
    }

    return (count % 2) != 0;
}

QPointF SlippyMapLayerPolygon::position()
{
    const qreal MIN = numeric_limits<qreal>().min();
    const qreal MAX = numeric_limits<qreal>().max();
    qreal x = MAX;
    qreal y = MIN;

    for (QPointF point : m_points) {
        if (point.x() < x) {
            x = point.x();
        }

        if (point.y() > y) {
            y = point.y();
        }
    }

    return QPointF(x, y);
}

QSizeF SlippyMapLayerPolygon::size()
{
    QPointF pos = position();

    const qreal MIN = numeric_limits<qreal>().min();
    const qreal MAX = numeric_limits<qreal>().max();
    qreal x = -MAX;
    qreal y = MAX;

    for (QPointF point : m_points) {
        qDebug() << "Point:" << point;

        if (point.x() > x) {
            x = point.x();
        }

        if (point.y() < y) {
            y = point.y();
        }
    }

    qDebug() << "Got X:" << x << "and Y:" << y;

    qreal width = x - pos.x();
    qreal height = pos.y() - y;
    return QSizeF(width, height);
}

SlippyMapLayerObjectPropertyPage *SlippyMapLayerPolygon::propertyPage(QWidget *parent)
{
    SlippyMapLayerPolygonPropertyPage *ppage =
            new SlippyMapLayerPolygonPropertyPage(this, parent);
    return ppage;
}

bool SlippyMapLayerPolygon::test_point(QPointF a, QPointF b, QPointF p)
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
