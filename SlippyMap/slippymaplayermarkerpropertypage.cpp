#include "slippymaplayermarkerpropertypage.h"
#include "slippymaplayerobject.h"

#include <QLabel>
#include <QLineEdit>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QPointF>
#include <QPlainTextEdit>

SlippyMapLayerMarkerPropertyPage::SlippyMapLayerMarkerPropertyPage(QWidget *parent)
{
    m_lneTitle = new QLineEdit(this);
    m_lneX = new QLineEdit(this);
    m_lneY = new QLineEdit(this);
    m_pteDescription = new QPlainTextEdit(this);

    QFormLayout *layout = new QFormLayout();
    layout->addRow(tr("Title"), m_lneTitle);
    layout->addRow(tr("Description"), m_pteDescription);

    QGroupBox *grpMarker = new QGroupBox();
    grpMarker->setTitle(tr("Marker"));
    grpMarker->setLayout(layout);

    layout = new QFormLayout();
    layout->addRow(tr("Latitude"), m_lneY);
    layout->addRow(tr("Longitude"), m_lneX);

    QGroupBox *grpPosition = new QGroupBox();
    grpPosition->setTitle(tr("Position"));
    grpPosition->setLayout(layout);

    QVBoxLayout *vbox = new QVBoxLayout();
    vbox->addWidget(grpMarker);
    vbox->addWidget(grpPosition);
    vbox->addStretch();

    setLayout(vbox);
}

SlippyMapLayerMarkerPropertyPage::SlippyMapLayerMarkerPropertyPage(SlippyMapLayerObject *object, QWidget *parent) :
    SlippyMapLayerMarkerPropertyPage (parent)
{

}

QString SlippyMapLayerMarkerPropertyPage::tabTitle()
{
    return tr("Marker");
}
