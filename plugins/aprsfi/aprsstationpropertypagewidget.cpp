#include "aprsstationpropertypagewidget.h"
#include "ui_aprsstationpropertypagewidget.h"

AprsStationPropertyPageWidget::AprsStationPropertyPageWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::AprsStationPropertyPageWidget)
{
    ui->setupUi(this);

    m_stationNameItem = new QListWidgetItem(tr("Name"));
}

AprsStationPropertyPageWidget::~AprsStationPropertyPageWidget()
{
    delete ui;
}
