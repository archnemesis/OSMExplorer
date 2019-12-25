#include "shapepropertypagewidget.h"

ShapePropertyPageWidget::ShapePropertyPageWidget(QWidget *parent) :
    QWidget (parent)
{

}

ShapePropertyPageWidget::ShapePropertyPageWidget(SlippyMapLayerObject *object, QWidget *parent) :
    QWidget (parent),
    m_object (object)
{

}

ShapePropertyPageWidget::~ShapePropertyPageWidget()
{

}
