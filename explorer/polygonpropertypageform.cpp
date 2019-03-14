#include "polygonpropertypageform.h"
#include "ui_polygonpropertypageform.h"

PolygonPropertyPageForm::PolygonPropertyPageForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PolygonPropertyPageForm)
{
    ui->setupUi(this);
}

PolygonPropertyPageForm::~PolygonPropertyPageForm()
{
    delete ui;
}
