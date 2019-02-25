#include "markerlistitemwidget.h"
#include "ui_markerlistitemwidget.h"
#include "slippymapwidget.h"

MarkerListItemWidget::MarkerListItemWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MarkerListItemWidget)
{
    ui->setupUi(this);
    connect(ui->btnCenterMap, &QPushButton::pressed, this, &MarkerListItemWidget::centerMapButtonPressed);
}

MarkerListItemWidget::~MarkerListItemWidget()
{
    delete ui;
}

void MarkerListItemWidget::setName(QString name)
{
    m_name = name;
    ui->lblName->setText(name);
}

void MarkerListItemWidget::setDescription(QString description)
{
    m_description = description;
    ui->lblDescription->setText(description);
}

void MarkerListItemWidget::setLatitude(double latitude)
{
    m_latitude = latitude;
    ui->btnCenterMap->setText(SlippyMapWidget::latLonToString(m_latitude, m_longitude));
}

void MarkerListItemWidget::setLongitude(double longitude)
{
    m_longitude = longitude;
    ui->btnCenterMap->setText(SlippyMapWidget::latLonToString(m_latitude, m_longitude));
}

void MarkerListItemWidget::centerMapButtonPressed()
{
    emit markerMapButtonPressed(m_latitude, m_longitude);
}
