//
// Created by robin on 10/29/24.
//

#ifndef WEATHERSTATIONPROPERTYPAGE_H
#define WEATHERSTATIONPROPERTYPAGE_H

#include "PropertyPage/SlippyMapLayerObjectPropertyPage.h"
#include "NationalWeatherServiceInterface.h"
#include "WeatherStationMarker.h"

class QLabel;
class QTabWidget;

class WeatherStationPropertyPage : public SlippyMapLayerObjectPropertyPage {
    Q_OBJECT
public:
    explicit WeatherStationPropertyPage(WeatherStationMarker *object);
    QString tabTitle() override;

protected slots:
    void nationalWeatherServiceInterface_onLatestObservationReady();

protected:
    void setupUi();

    QString m_stationId;
    NationalWeatherServiceInterface *m_nationalWeatherServiceInterface;
    QTabWidget *m_tabWidget;
    QLabel *m_temperatureLabel;
    QLabel *m_relativeHumidityLabel;
    QLabel *m_windSpeedLabel;
    QLabel *m_windDirectionLabel;
    QLabel *m_dewPointLabel;
    QLabel *m_timestampLabel;
};



#endif //WEATHERSTATIONPROPERTYPAGE_H
