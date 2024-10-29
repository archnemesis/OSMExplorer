//
// Created by robin on 10/29/24.
//

#include "WeatherStationPropertyPage.h"

#include <QLabel>
#include <QFormLayout>

WeatherStationPropertyPage::WeatherStationPropertyPage(WeatherStationMarker *object) :
    SlippyMapLayerObjectPropertyPage(object) {
    setupUi();

    m_stationId = object->stationId();
    m_nationalWeatherServiceInterface = new NationalWeatherServiceInterface(this);
    connect(m_nationalWeatherServiceInterface,
        &NationalWeatherServiceInterface::latestObservationReady,
        this,
        &WeatherStationPropertyPage::nationalWeatherServiceInterface_onLatestObservationReady);
    m_nationalWeatherServiceInterface->getLatestObservation(m_stationId);
}

QString WeatherStationPropertyPage::tabTitle() {
    return tr("Weather Station");
}

void WeatherStationPropertyPage::nationalWeatherServiceInterface_onLatestObservationReady() {
    NationalWeatherServiceInterface::Observation observation \
        = m_nationalWeatherServiceInterface->latestObservation();

    m_temperatureLabel->setText(tr("%1 C").arg(observation.temperature));
    m_relativeHumidityLabel->setText(tr("%1 %%").arg(observation.relativeHumidity));
    m_windSpeedLabel->setText(tr("%1").arg(observation.windSpeed));
    m_windDirectionLabel->setText(tr("%1").arg(observation.windDirection));
    m_dewPointLabel->setText(tr("%1 C").arg(observation.dewPoint));
}

void WeatherStationPropertyPage::setupUi() {
    m_temperatureLabel = new QLabel(this);
    m_temperatureLabel->setText("0.0");

    m_relativeHumidityLabel = new QLabel(this);
    m_relativeHumidityLabel->setText("0.0");

    m_windSpeedLabel = new QLabel(this);
    m_windSpeedLabel->setText("0.0");

    m_windDirectionLabel = new QLabel(this);
    m_windDirectionLabel->setText("0.0");

    m_dewPointLabel = new QLabel(this);
    m_dewPointLabel->setText("0.0");

    m_timestampLabel = new QLabel(this);
    m_timestampLabel->setText("0000-00-00T00:00:00.0");

    auto *layout = new QFormLayout(this);
    layout->addRow(tr("Temperature:"), m_temperatureLabel);
    layout->addRow(tr("Relative Humidity:"), m_relativeHumidityLabel);
    layout->addRow(tr("Wind Speed:"), m_windSpeedLabel);
    layout->addRow(tr("Wind Direction:"), m_windDirectionLabel);
    layout->addRow(tr("Dew Point:"), m_dewPointLabel);
    layout->addRow(tr("Timestamp"), m_timestampLabel);

    setLayout(layout);
}
