#include "slippymapwidgetellipse.h"

SlippyMapWidgetEllipse::SlippyMapWidgetEllipse(QObject *parent) :
    SlippyMapWidgetShape (parent)
{

}

void SlippyMapWidgetEllipse::draw(QPainter *painter, const QTransform &transform, ShapeState state)
{
    (void)painter;
}

void SlippyMapWidgetEllipse::setRect(const QRectF &rect)
{
    m_rect = rect;
}
