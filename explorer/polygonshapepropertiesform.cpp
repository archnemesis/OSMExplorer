#include "polygonshapepropertiesform.h"
#include "ui_polygonshapepropertiesform.h"

PolygonShapePropertiesForm::PolygonShapePropertiesForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PolygonShapePropertiesForm)
{
    ui->setupUi(this);
}

PolygonShapePropertiesForm::~PolygonShapePropertiesForm()
{
    delete ui;
}
