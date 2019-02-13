#include "directionlistitemwidget.h"
#include "ui_directionlistitemwidget.h"

DirectionListItemWidget::DirectionListItemWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::DirectionListItemWidget)
{
    ui->setupUi(this);
}

DirectionListItemWidget::~DirectionListItemWidget()
{
    delete ui;
}

void DirectionListItemWidget::setInstruction(QString instruction)
{
    m_instruction = instruction;
    ui->lblInstruction->setText(m_instruction);
}

void DirectionListItemWidget::setDuration(double duration)
{
    m_duration = duration;
    ui->lblDuration->setText(QString("%1 s").arg(duration));
}

void DirectionListItemWidget::setDistance(double distance)
{
    m_distance = distance;
    ui->lblDistance->setText(QString("%1 m").arg(distance));
}

QString DirectionListItemWidget::instruction()
{
    return m_instruction;
}

double DirectionListItemWidget::duration()
{
    return m_duration;
}

double DirectionListItemWidget::distance()
{
    return m_distance;
}
