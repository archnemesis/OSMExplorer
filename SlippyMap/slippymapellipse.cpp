#include "slippymapellipse.h"

SlippyMapEllipse::SlippyMapEllipse(QObject *parent) :
    SlippyMapLayerObject (parent)
{

}

void SlippyMapEllipse::draw(QPainter *painter, const QTransform &transform, ObjectState state)
{
    (void)painter;
    (void)transform;
    (void)state;
}

void SlippyMapEllipse::setRect(const QRectF &rect)
{
    m_rect = rect;
}
