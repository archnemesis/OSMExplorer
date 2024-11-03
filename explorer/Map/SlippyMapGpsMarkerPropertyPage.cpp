//
// Created by Robin on 11/1/2024.
//

#include "SlippyMapGpsMarkerPropertyPage.h"
#include "SlippyMapGpsMarker.h"

#include <QDebug>
#include <QLabel>
#include <QProgressBar>
#include <QFormLayout>

SlippyMapGpsMarkerPropertyPage::SlippyMapGpsMarkerPropertyPage(SlippyMapLayerObject *object)
        : SlippyMapLayerObjectPropertyPage(object)
{

}

QString SlippyMapGpsMarkerPropertyPage::tabTitle()
{
    return tr("GPS");
}

void SlippyMapGpsMarkerPropertyPage::setupUi()
{
    m_numSatsLabel = new QLabel();
    m_gpsTimeLabel = new QLabel();

    auto *satelliteLayout = new QHBoxLayout();
    for (int i = 0; i < 12; i++) {
        auto *pgs = new QProgressBar();
        pgs->setMinimum(0);
        pgs->setMaximum(40);
        pgs->setOrientation(Qt::Vertical);
        m_satellites.append(pgs);
        satelliteLayout->addWidget(pgs);
    }

    auto *formLayout = new QFormLayout();
    formLayout->addRow(tr("GPS Time"), m_gpsTimeLabel);
    formLayout->addRow(tr("Satellites"), m_numSatsLabel);

    auto *vLayout = new QVBoxLayout();
    vLayout->addLayout(formLayout);
    vLayout->addLayout(satelliteLayout);

    updateUi();

    setLayout(vLayout);
}

void SlippyMapGpsMarkerPropertyPage::save()
{

}

void SlippyMapGpsMarkerPropertyPage::updateUi()
{
    SlippyMapGpsMarker *marker = dynamic_cast<SlippyMapGpsMarker*>(m_object);
    int index = 0;
    for (auto satellite: marker->satellites()) {
        m_satellites.at(index)->setValue(satellite.snr());
        index++;
    }
}
