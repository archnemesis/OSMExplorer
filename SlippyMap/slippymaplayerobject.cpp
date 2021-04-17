#include "slippymaplayerobject.h"

SlippyMapLayerObject::SlippyMapLayerObject(QObject *parent) : QObject(parent)
{
    m_pen.setStyle(Qt::SolidLine);
    m_pen.setCosmetic(true);
    m_pen.setWidth(2);
    m_pen.setColor(QColor(153, 153, 102, 128));
    m_pen.setJoinStyle(Qt::MiterJoin);
    m_pen.setCapStyle(Qt::SquareCap);

    m_brush.setStyle(Qt::SolidPattern);
    m_brush.setColor(QColor(255, 255, 51, 128));

    m_selectionHandlePen.setColor(Qt::black);
    m_selectionHandlePen.setStyle(Qt::SolidLine);
    m_selectionHandlePen.setCosmetic(true);
    m_selectionHandlePen.setWidth(1);
    m_selectionHandlePen.setJoinStyle(Qt::MiterJoin);
    m_selectionHandlePen.setCapStyle(Qt::SquareCap);

    m_selectionHandleBrush.setColor(Qt::white);
    m_selectionHandleBrush.setStyle(Qt::SolidPattern);

    m_activePen.setStyle(Qt::DotLine);
    m_activePen.setCosmetic(true);
    m_activePen.setColor(Qt::white);
    m_activePen.setWidth(2);
    m_activeBrush.setStyle(Qt::SolidPattern);
    m_activeBrush.setColor(Qt::gray);

    m_selectedPen.setStyle(Qt::DotLine);
    m_selectedPen.setCosmetic(true);
    m_selectedPen.setColor(Qt::darkBlue);
    m_selectedPen.setWidth(2);
    m_selectedBrush.setStyle(Qt::SolidPattern);
    m_selectedBrush.setColor(Qt::lightGray);
}

void SlippyMapLayerObject::setMovable(bool movable)
{
    m_movable = true;
}

QString SlippyMapLayerObject::name()
{
    return m_name;
}

QString SlippyMapLayerObject::description()
{
    return m_description;
}

void SlippyMapLayerObject::setName(const QString &name)
{
    m_name = name;
}

void SlippyMapLayerObject::setDescription(const QString &description)
{
    m_description = description;
}

void SlippyMapLayerObject::setVisible(bool visible)
{
    m_visible = visible;
}

bool SlippyMapLayerObject::isVisible()
{
    return m_visible;
}

void SlippyMapLayerObject::setBrush(QBrush brush)
{
    m_brush = brush;
}

void SlippyMapLayerObject::setPen(QPen pen)
{
    m_pen = pen;
}

bool SlippyMapLayerObject::isMovable()
{
    return m_movable;
}

QString SlippyMapLayerObject::statusBarText()
{
    return QString("");
}

void SlippyMapLayerObject::drawResizeHandle(QPainter *painter, QPoint point)
{
    painter->setBrush(m_selectionHandleBrush);
    painter->setPen(m_selectionHandlePen);
    painter->drawRect(
                (point.x() - (m_resizeHandleWidth / 2)),
                (point.y() - (m_resizeHandleWidth / 2)),
                m_resizeHandleWidth,
                m_resizeHandleWidth);
}



