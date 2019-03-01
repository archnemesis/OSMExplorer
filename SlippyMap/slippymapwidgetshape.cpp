#include "slippymapwidgetshape.h"
#include <QObject>
#include <QColor>

SlippyMapWidgetShape::SlippyMapWidgetShape(QObject *parent) : QObject(parent)
{
    m_selectionHandlePen.setColor(Qt::black);
    m_selectionHandlePen.setStyle(Qt::SolidLine);
    m_selectionHandlePen.setWidth(1);

    m_selectionHandleBrush.setColor(Qt::white);
    m_selectionHandleBrush.setStyle(Qt::SolidPattern);
}

SlippyMapWidgetShape::~SlippyMapWidgetShape()
{

}

void SlippyMapWidgetShape::setBrush(QBrush brush)
{
    m_brush = brush;
}

void SlippyMapWidgetShape::setPen(QPen pen)
{
    m_pen = pen;
}

void SlippyMapWidgetShape::drawResizeHandle(QPainter *painter, QPoint point)
{
    painter->setBrush(m_selectionHandleBrush);
    painter->setPen(m_selectionHandlePen);
    painter->drawRect(
                (point.x() - (m_resizeHandleWidth / 2)),
                (point.y() - (m_resizeHandleWidth / 2)),
                (point.x() + (m_resizeHandleWidth / 2)),
                (point.y() + (m_resizeHandleWidth / 2)));
}


