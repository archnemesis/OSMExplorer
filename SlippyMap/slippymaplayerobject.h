#ifndef SLIPPYMAPLAYEROBJECT_H
#define SLIPPYMAPLAYEROBJECT_H

#include "slippymap_global.h"

#include <QObject>
#include <QPainter>
#include <QBrush>
#include <QPen>
#include <QTransform>
#include <QPointF>
#include <QSizeF>

class SlippyMapLayerObjectPropertyPage;

class SLIPPYMAPSHARED_EXPORT SlippyMapLayerObject : public QObject
{
    Q_OBJECT
public:
    enum ObjectState {
        NormalState,
        SelectedState,
        DisabledState
    };

    explicit SlippyMapLayerObject(QObject *parent = nullptr);
    virtual void draw(QPainter *painter, const QTransform &transform, ObjectState state = NormalState) = 0;
    virtual void setMovable(bool movable);
    void setBrush(QBrush brush);
    void setPen(QPen pen);
    virtual bool contains(QPointF point, int zoom) = 0;
    virtual bool isIntersectedBy(QRectF rect) = 0;
    virtual bool isMovable();
    virtual QPointF position() = 0;
    virtual QSizeF size() = 0;
    virtual SlippyMapLayerObjectPropertyPage *propertyPage(QWidget *parent = nullptr) = 0;
    QString name();
    void setName(const QString& name);
    void setDescription(const QString& description);
    void setVisible(bool visible);
    bool isVisible();

signals:
    void somethingHappened();

protected:
    QString m_name;
    QString m_description;
    QBrush m_brush;
    QBrush m_activeBrush;
    QBrush m_selectedBrush;
    QBrush m_selectionHandleBrush;
    QPen m_pen;
    QPen m_activePen;
    QPen m_selectedPen;
    QPen m_selectionHandlePen;
    int m_resizeHandleWidth = 6;
    bool m_visible = true;
    bool m_movable = true;
    void drawResizeHandle(QPainter *painter, QPoint point);
};

#endif // SLIPPYMAPLAYEROBJECT_H
