#ifndef SLIPPYMAPLAYERROUTE_H
#define SLIPPYMAPLAYERROUTE_H

#include "slippymaplayerobject.h"

#include <QVector>

class SlippyMapLayerObjectPropertyPage;

class SlippyMapLayerTrack : public SlippyMapLayerObject
{
public:
    SlippyMapLayerTrack(const QVector<QPointF> &points);
    void draw(QPainter *painter, const QTransform &transform, ObjectState state = NormalState);
    bool isIntersectedBy(QRectF rect);
    bool contains(QPointF point, int zoom);
    bool isMovable();
    QPointF position();
    QSizeF size();
    SlippyMapLayerObjectPropertyPage *propertyPage(QWidget *parent = nullptr);

private:
    void initStyle();

    QBrush m_dotBrush;
    QBrush m_lineBrush;
    QPen m_dotPen;
    QPen m_linePen;

    QColor m_trackLineColor;
    QColor m_trackLineStrokeColor;
    int m_trackLineWidth;
    int m_trackLineStrokeWidth;

    SlippyMapLayerObjectPropertyPage *m_propertyPage = nullptr;

protected:
    QVector<QPointF> m_points;
};

#endif // SLIPPYMAPLAYERROUTE_H
