#ifndef SHAPEPROPERTYPAGEWIDGET_H
#define SHAPEPROPERTYPAGEWIDGET_H

#include <QWidget>
#include "slippymapwidgetshape.h"

class ShapePropertyPageWidget : public QWidget
{
    Q_OBJECT
public:
    ShapePropertyPageWidget(QWidget *parent = nullptr);
    ShapePropertyPageWidget(SlippyMapWidgetShape *shape, QWidget *parent = nullptr);
    virtual ~ShapePropertyPageWidget();
protected:
    SlippyMapWidgetShape *m_shape;
};

#endif // SHAPEPROPERTYPAGEWIDGET_H
