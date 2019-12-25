#ifndef SHAPEPROPERTYPAGEWIDGET_H
#define SHAPEPROPERTYPAGEWIDGET_H

#include <QWidget>
#include "slippymaplayerobject.h"

class ShapePropertyPageWidget : public QWidget
{
    Q_OBJECT
public:
    ShapePropertyPageWidget(QWidget *parent = nullptr);
    ShapePropertyPageWidget(SlippyMapLayerObject *object, QWidget *parent = nullptr);
    virtual ~ShapePropertyPageWidget();
protected:
    SlippyMapLayerObject *m_object;
};

#endif // SHAPEPROPERTYPAGEWIDGET_H
