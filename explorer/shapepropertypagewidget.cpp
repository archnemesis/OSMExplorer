#include "shapepropertypagewidget.h"

ShapePropertyPageWidget::ShapePropertyPageWidget(QWidget *parent) :
    QWidget (parent)
{

}

ShapePropertyPageWidget::ShapePropertyPageWidget(SlippyMapWidgetShape *shape, QWidget *parent) :
    QWidget (parent),
    m_shape (shape)
{

}

ShapePropertyPageWidget::~ShapePropertyPageWidget()
{

}
