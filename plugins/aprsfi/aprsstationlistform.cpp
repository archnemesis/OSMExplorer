#include "aprsstationlistform.h"
#include "ui_aprsstationlistform.h"

AprsStationListForm::AprsStationListForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::AprsStationListForm)
{
    ui->setupUi(this);
}

AprsStationListForm::~AprsStationListForm()
{
    delete ui;
}
