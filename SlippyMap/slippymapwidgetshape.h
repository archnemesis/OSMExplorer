#ifndef SLIPPYMAPWIDGETSHAPE_H
#define SLIPPYMAPWIDGETSHAPE_H

#include "slippymap_global.h"

#include <QObject>
#include <QPainter>
#include <QBrush>
#include <QPen>
#include <QTransform>

class SLIPPYMAPSHARED_EXPORT SlippyMapWidgetShape : public QObject
{
    Q_OBJECT
public:
    enum ShapeState {
        NormalState,
        SelectedState,
        DisabledState
    };

    SlippyMapWidgetShape(QObject *parent = nullptr);
    ~SlippyMapWidgetShape();
    virtual void draw(QPainter *painter, const QTransform &transform, ShapeState state = NormalState) = 0;
    void setBrush(QBrush brush);
    void setPen(QPen pen);
    virtual bool contains(QPointF point) = 0;
    virtual bool isIntersectedBy(QRectF rect) = 0;

signals:
    void somethingHappened();

protected:
    QBrush m_brush;
    QBrush m_activeBrush;
    QBrush m_selectedBrush;
    QBrush m_selectionHandleBrush;
    QPen m_pen;
    QPen m_activePen;
    QPen m_selectedPen;
    QPen m_selectionHandlePen;
    int m_resizeHandleWidth = 6;

    void drawResizeHandle(QPainter *painter, QPoint point);
};

#endif // SLIPPYMAPWIDGETSHAPE_H
