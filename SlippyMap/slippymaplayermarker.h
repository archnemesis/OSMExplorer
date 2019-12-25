#ifndef SLIPPYMAPLAYERMARKER_H
#define SLIPPYMAPLAYERMARKER_H

#include "slippymaplayerobject.h"

class SLIPPYMAPSHARED_EXPORT SlippyMapLayerMarker : public SlippyMapLayerObject
{
public:
    SlippyMapLayerMarker(const QPointF &position, QObject *parent = nullptr);
    void draw(QPainter *painter, const QTransform &transform, ObjectState state = NormalState);
    bool isIntersectedBy(QRectF rect);
    bool contains(QPointF point, int zoom);
    void setLabel(QString label);
    void setDescription(QString description);
    void setPosition(QPointF position);
    void setColor(QColor color);
    void setEditable(bool editable);
    virtual bool isMovable();
    QPointF position();
    QSizeF size();
    SlippyMapLayerObjectPropertyPage *propertyPage(QWidget *parent = nullptr);
protected:
    void initStyle();

    qint32 m_radius;
    QHash<QString,QVariant> m_metadata;
    QPointF m_position;
    QString m_label;
    QString m_information;
    QColor m_markerColor;
    QBrush m_activeDotBrush;
    QBrush m_dotBrush;
    QBrush m_labelBrush;
    QBrush m_labelTextBrush;
    QPen m_activeDotPen;
    QPen m_dotPen;
    QPen m_labelPen;
    QPen m_labelTextPen;
    bool m_active = false;
    bool m_movable = true;
    bool m_editable = true;
};

#endif // SLIPPYMAPLAYERMARKER_H
