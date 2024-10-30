//
// Created by Robin on 10/29/2024.
//

#ifndef OSMEXPLORER_FORECASTPERIODWIDGET_H
#define OSMEXPLORER_FORECASTPERIODWIDGET_H

#include <QWidget>
#include "Weather/NationalWeatherServiceInterface.h"


class QLabel;

class ForecastPeriodWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ForecastPeriodWidget(const NationalWeatherServiceInterface::ForecastPeriod& period);

private:
    QLabel *m_temperatureLabel;
    QLabel *m_detailLabel;
    QLabel *m_chanceRainLabel;
    QLabel *m_periodNameLabel;
    QLabel *m_windSpeedLabel;
    QLabel *m_windDirectionLabel;
};


#endif //OSMEXPLORER_FORECASTPERIODWIDGET_H
