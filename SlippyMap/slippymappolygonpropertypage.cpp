#include "slippymappolygonpropertypage.h"

#include <QLineEdit>
#include <QLabel>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QPointF>

SlippyMapPolygonPropertyPage::SlippyMapPolygonPropertyPage(QWidget *parent)
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

    QVBoxLayout *vbox = new QVBoxLayout();
    vbox->addWidget(grpPosition);
    vbox->addWidget(grpSize);
    vbox->addStretch();

    setLayout(vbox);
}

SlippyMapPolygonPropertyPage::SlippyMapPolygonPropertyPage(SlippyMapWidgetShape *shape, QWidget *parent) :
    SlippyMapPolygonPropertyPage (parent)
{
    m_polygon = qobject_cast<SlippyMapWidgetPolygon*>(shape);

    QPointF pos = shape->position();
    QSizeF size = shape->size();

    m_lneX->setText(QString("%1").arg(pos.x()));
    m_lneY->setText(QString("%1").arg(pos.y()));
    m_lneWidth->setText(QString("%1").arg(size.width()));
    m_lneHeight->setText(QString("%1").arg(size.height()));
}

QString SlippyMapPolygonPropertyPage::tabTitle()
{
    return tr("Polygon");
}
