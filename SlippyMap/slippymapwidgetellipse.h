#ifndef SLIPPYMAPWIDGETELLIPSE_H
#define SLIPPYMAPWIDGETELLIPSE_H

#include "slippymap_global.h"
#include "slippymapwidgetshape.h"

#include <QObject>
#include <QPainter>
#include <QRectF>

class SLIPPYMAPSHARED_EXPORT SlippyMapWidgetEllipse : public SlippyMapWidgetShape
{
    Q_OBJECT
public:
    SlippyMapWidgetEllipse(QObject *parent = nullptr);

    void draw(QPainter *painter, const QTransform &transform, ShapeState state = NormalState);
    void setRect(const QRectF &rect);

private:
    QRectF m_rect;
};

#endif // SLIPPYMAPWIDGETELLIPSE_H
