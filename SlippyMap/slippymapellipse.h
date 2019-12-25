#ifndef SLIPPYMAPWIDGETELLIPSE_H
#define SLIPPYMAPWIDGETELLIPSE_H

#include "slippymap_global.h"
#include "slippymaplayerobject.h"

#include <QObject>
#include <QPainter>
#include <QRectF>

class SLIPPYMAPSHARED_EXPORT SlippyMapEllipse : public SlippyMapLayerObject
{
    Q_OBJECT
public:
    SlippyMapEllipse(QObject *parent = nullptr);

    void draw(QPainter *painter, const QTransform &transform, ObjectState state = NormalState);
    void setRect(const QRectF &rect);

private:
    QRectF m_rect;
};

#endif // SLIPPYMAPWIDGETELLIPSE_H
