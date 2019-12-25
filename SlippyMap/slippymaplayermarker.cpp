#include "slippymaplayermarker.h"

#include <QPainter>
#include <QPoint>
#include <QPointF>
#include <QBrush>
#include <QPen>
#include <QColor>
#include <QFont>
#include <QFontMetrics>
#include <QPalette>
#include <QGuiApplication>

SlippyMapLayerMarker::SlippyMapLayerMarker(const QPointF &position, QObject *parent) :
    SlippyMapLayerObject (parent)
{
    m_position = position;
    m_radius = 5;
}

void SlippyMapLayerMarker::draw(QPainter *painter, const QTransform &transform, SlippyMapLayerObject::ObjectState state)
{
    QPointF pos = transform.map(m_position);

    if (state == SelectedState) {
        painter->setBrush(m_activeDotBrush);
        painter->setPen(m_activeDotPen);
        painter->drawEllipse(pos, m_radius + 2, m_radius + 2);
    }

    painter->setBrush(m_dotBrush);
    painter->setPen(m_dotPen);
    painter->drawEllipse(pos, m_radius, m_radius);

    QFontMetrics metrics(painter->font());
    if (m_label.length() > 0) {
        qint32 label_w = metrics.width(m_label);
        qint32 label_h = metrics.height();
        qint32 label_x = static_cast<qint32>(pos.x()) - (label_w / 2);
        qint32 label_y = static_cast<qint32>(pos.y()) - (label_h + 15);
        painter->setBrush(m_labelBrush);
        painter->setPen(m_labelPen);
        painter->drawRoundRect(
                    label_x - 5,
                    label_y - 5,
                    label_w + 10,
                    label_h + 10,
                    5, 5);
        painter->setBrush(m_labelTextBrush);
        painter->setPen(m_labelTextPen);
        painter->drawText(
                    label_x,
                    label_y + (label_h / 2) + 5,
                    m_label);
    }
}

bool SlippyMapLayerMarker::isIntersectedBy(QRectF rect)
{
    return rect.contains(m_position);
}

bool SlippyMapLayerMarker::contains(QPointF point, int zoom)
{
    double deg_per_pixel = (360.0 / pow(2.0, zoom)) / 256.0;
    double deg_radius = deg_per_pixel * m_radius;
    QRectF deg_rect(
                m_position.x() - deg_radius,
                m_position.y() - deg_radius,
                deg_radius * 2,
                deg_radius * 2);
    return deg_rect.contains(point);
}

void SlippyMapLayerMarker::setLabel(QString label)
{
    m_label = label;
}

void SlippyMapLayerMarker::setDescription(QString description)
{
    m_description = description;
}

void SlippyMapLayerMarker::setPosition(QPointF position)
{
    m_position = position;
}

void SlippyMapLayerMarker::setColor(QColor color)
{
    m_markerColor = color;
}

void SlippyMapLayerMarker::setEditable(bool editable)
{
    m_editable = editable;
}

bool SlippyMapLayerMarker::isMovable()
{
    return m_movable;
}

QPointF SlippyMapLayerMarker::position()
{
    return m_position;
}

QSizeF SlippyMapLayerMarker::size()
{
    return QSizeF();
}

SlippyMapLayerObjectPropertyPage *SlippyMapLayerMarker::propertyPage(QWidget *parent)
{
    return nullptr;
}

void SlippyMapLayerMarker::initStyle()
{
    QPalette systemPalette = QGuiApplication::palette();

    m_dotBrush.setStyle(Qt::SolidPattern);
    m_dotBrush.setColor(systemPalette.highlight().color());
    m_dotPen.setStyle(Qt::NoPen);

    m_activeDotBrush.setStyle(Qt::NoBrush);
    m_activeDotPen.setColor(systemPalette.highlight().color());
    m_activeDotPen.setWidth(2);

    m_labelBrush.setStyle(Qt::SolidPattern);
    m_labelBrush.setColor(systemPalette.background().color());
    m_labelPen.setStyle(Qt::SolidLine);
    m_labelPen.setColor(systemPalette.dark().color());

    m_labelTextBrush.setStyle(Qt::NoBrush);
    m_labelTextPen.setStyle(Qt::SolidLine);
    m_labelTextPen.setColor(systemPalette.text().color());
}
