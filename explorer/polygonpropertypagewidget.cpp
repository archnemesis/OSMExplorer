#include "polygonpropertypagewidget.h"
#include <QLineEdit>
#include <QLabel>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QPointF>

PolygonPropertyPageWidget::PolygonPropertyPageWidget(QWidget *parent) :
    ShapePropertyPageWidget (parent)
{
    m_lneX = new QLineEdit();
    m_lneY = new QLineEdit();
    m_lneWidth = new QLineEdit();
    m_lneHeight = new QLineEdit();

    QFormLayout *layout = new QFormLayout();
    layout->addRow(tr("Latitude"), m_lneY);
    layout->addRow(tr("Longitude"), m_lneX);

    QGroupBox *grpPosition = new QGroupBox();
    grpPosition->setTitle(tr("Position"));
    grpPosition->setLayout(layout);

    layout = new QFormLayout();
    layout->addRow(tr("Width"), m_lneWidth);
    layout->addRow(tr("Height"), m_lneHeight);

    QGroupBox *grpSize = new QGroupBox();
    grpSize->setTitle(tr("Size"));
    grpSize->setLayout(layout);

    QHBoxLayout *hbox = new QHBoxLayout();
    hbox->addWidget(grpPosition);
    hbox->addWidget(grpSize);
    hbox->addStretch();

    setLayout(hbox);
}

PolygonPropertyPageWidget::PolygonPropertyPageWidget(SlippyMapWidgetShape *shape, QWidget *parent) :
    PolygonPropertyPageWidget (parent)
{
    m_polygon = qobject_cast<SlippyMapWidgetPolygon*>(shape);

    QPointF pos = shape->position();
    QSizeF size = shape->size();

    m_lneX->setText(QString("%1").arg(pos.x()));
    m_lneY->setText(QString("%1").arg(pos.y()));
    m_lneWidth->setText(QString("%1").arg(size.width()));
    m_lneHeight->setText(QString("%1").arg(size.height()));
}
