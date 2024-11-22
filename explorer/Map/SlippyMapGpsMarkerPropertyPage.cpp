//
// Created by Robin on 11/1/2024.
//

#include "SlippyMapGpsMarkerPropertyPage.h"
#include "SlippyMapGpsMarker.h"

#include <QDebug>
#include <QLabel>
#include <QProgressBar>
#include <QFormLayout>

SlippyMapGpsMarkerPropertyPage::SlippyMapGpsMarkerPropertyPage(const SlippyMapLayerObject::Ptr& object)
        : SlippyMapLayerObjectPropertyPage(object)
{

}

QString SlippyMapGpsMarkerPropertyPage::tabTitle()
{
    return tr("GPS Status");
}

void SlippyMapGpsMarkerPropertyPage::setupUi()
{
    m_numSatsLabel = new QLabel();
    m_gpsTimeLabel = new QLabel();
    m_hdopLabel = new QLabel();
    m_vdopLabel = new QLabel();
    m_pdopLabel = new QLabel();
    m_gpsAltitudeLabel = new QLabel();
    m_gpsLatitudeLabel = new QLabel();
    m_gpsLongitudeLabel = new QLabel();
    m_fixTypeLabel = new QLabel();

    auto *satelliteLayout = new QHBoxLayout();
    for (int i = 0; i < 16; i++) {
        auto *pgs = new QProgressBar();
        pgs->setMinimum(0);
        pgs->setMaximum(40);
        pgs->setOrientation(Qt::Vertical);

        auto *lbl = new QLabel();
        lbl->setAlignment(Qt::AlignCenter);

        m_satelliteProgressBars.append(pgs);
        m_satellitePrnLabels.append(lbl);

        auto *layout = new QVBoxLayout();
        layout->addWidget(pgs);
        layout->addWidget(lbl);
        satelliteLayout->addLayout(layout);
    }

    auto *formLayout = new QFormLayout();
    formLayout->addRow(tr("Latitude"), m_gpsLatitudeLabel);
    formLayout->addRow(tr("Longitude"), m_gpsLongitudeLabel);
    formLayout->addRow(tr("Altitude"), m_gpsAltitudeLabel);
    formLayout->addRow(tr("GPS Time"), m_gpsTimeLabel);
    formLayout->addRow(tr("Fix"), m_fixTypeLabel);
    formLayout->addRow(tr("Satellites"), m_numSatsLabel);
    formLayout->addRow(tr("HDOP"), m_hdopLabel);
    formLayout->addRow(tr("VDOP"), m_vdopLabel);

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
    SlippyMapGpsMarker *marker = dynamic_cast<SlippyMapGpsMarker*>(m_object.get());
    int index = 0;
    int numSatellites = 0;
    for (auto satellite: marker->gpsData().satellites()) {
        m_satelliteProgressBars.at(index)->setValue(satellite.snr());
        m_satellitePrnLabels.at(index)->setText(QString::number(satellite.prn()));
        if (satellite.prn() > 0)
            numSatellites++;
        index++;
    }

    const auto& gpsData = marker->gpsData();

    switch (gpsData.fixType()) {
    case NmeaSerialLocationDataProvider::PositionData::FixNone:
        m_fixTypeLabel->setText(tr("None"));
        break;
    case NmeaSerialLocationDataProvider::PositionData::Fix2D:
        m_fixTypeLabel->setText(tr("2D"));
        break;
    case NmeaSerialLocationDataProvider::PositionData::Fix3D:
        m_fixTypeLabel->setText(tr("3D"));
        break;
    default:
        m_fixTypeLabel->setText(tr("Unknown"));
        break;
    }

    m_numSatsLabel->setText(QString::number(numSatellites));
    m_gpsTimeLabel->setText(gpsData.gpsTime().toString(Qt::ISODate));
    m_gpsLatitudeLabel->setText(QString::number(gpsData.latitude(), 'f', 7));
    m_gpsLongitudeLabel->setText(QString::number(gpsData.longitude(), 'f', 7));
    m_gpsAltitudeLabel->setText(QString::number(gpsData.altitude(), 'f', 3));
    m_hdopLabel->setText(QString::number(gpsData.hdop(), 'f', 2));
    m_vdopLabel->setText(QString::number(gpsData.vdop(), 'f', 2));
}
