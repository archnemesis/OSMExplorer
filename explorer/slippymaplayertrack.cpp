#include "slippymaplayertrack.h"
#include "slippymaplayertrackpropertypageform.h"

#include <cmath>

#include <QPainter>
#include <QPointF>
#include <QBrush>
#include <QPen>
#include <QColor>
#include <QFont>
#include <QPolygonF>

SlippyMapLayerTrack::SlippyMapLayerTrack(const QVector<QPointF> &points)
{
    m_points = points;
    initStyle();
}

void SlippyMapLayerTrack::draw(QPainter *painter, const QTransform &transform, SlippyMapLayerObject::ObjectState state)
{
    for (int i = 0; i < m_points.length(); i++) {
        QPointF thisPoint = transform.map(m_points.at(i));

        if ((i + 1) < m_points.length()) {
            QPointF nextPoint = transform.map(m_points.at(i + 1));
            painter->setPen(m_linePen);
            painter->setBrush(m_lineBrush);
            painter->drawLine(thisPoint, nextPoint);
        }

        painter->setPen(m_dotPen);
        painter->setBrush(m_dotBrush);
        painter->drawPoint(thisPoint);
    }
}

bool SlippyMapLayerTrack::isIntersectedBy(QRectF rect)
{
    for (QPointF point : m_points) {
        if (rect.contains(point)) {
            return true;
        }
    }

    return false;
}

bool SlippyMapLayerTrack::contains(QPointF point, int zoom)
{
    double deg_per_pixel = (360.0 / pow(2.0, zoom)) / 256.0;
    double deg_radius = deg_per_pixel * 10;

    for (int i = 0; i < m_points.length(); i++) {
        QRectF deg_rect(
                    m_points.at(i).x() - deg_radius,
                    m_points.at(i).y() - deg_radius,
                    deg_radius * 2,
                    deg_radius * 2);

        if (deg_rect.contains(point)) {
            return true;
        }

//        if ((i + 1) < m_points.length()) {
//            QPointF thisPoint = m_points.at(i);
//            QPointF nextPoint = m_points.at(i + 1);

//            qreal a = (nextPoint.y() - thisPoint.y()) / (nextPoint.x() - thisPoint.x());
//            qreal b = thisPoint.y() - (a * thisPoint.x());

//            //
//            // check if point lies within line up/down
//            // then check if point lies within x on either side of where it interects line
//            //

//        }
    }

    return false;
}

bool SlippyMapLayerTrack::isMovable()
{
    return false;
}

QPointF SlippyMapLayerTrack::position()
{
    return m_points.at(0);
}

QSizeF SlippyMapLayerTrack::size()
{
    QPolygonF poly(m_points);
    QRectF boundingRect = poly.boundingRect();
    return boundingRect.size();
}

SlippyMapLayerObjectPropertyPage *SlippyMapLayerTrack::propertyPage(QWidget *parent)
{
    if (m_propertyPage == nullptr) {
        m_propertyPage = new SlippyMapLayerTrackPropertyPageForm(this, parent);
    }
    return m_propertyPage;
}

void SlippyMapLayerTrack::initStyle()
{
    m_dotBrush.setColor(Qt::black);
    m_dotBrush.setStyle(Qt::SolidPattern);
    m_dotPen.setColor(Qt::green);
    m_dotPen.setStyle(Qt::SolidLine);
    m_dotPen.setWidth(2);

    m_linePen.setStyle(Qt::SolidLine);
    m_linePen.setColor(Qt::white);
    m_linePen.setWidth(3);
}
