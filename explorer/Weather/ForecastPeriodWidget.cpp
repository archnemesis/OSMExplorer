//
// Created by Robin on 10/29/2024.
//

#include "ForecastPeriodWidget.h"

#include <QLabel>
#include <QVBoxLayout>
#include <QApplication>

ForecastPeriodWidget::ForecastPeriodWidget(const NationalWeatherServiceInterface::ForecastPeriod &period)
{
    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Maximum);

    QFont largeFont = qApp->font();
    largeFont.setBold(true);
    largeFont.setPointSize(18);

    m_temperatureLabel = new QLabel();
    m_temperatureLabel->setFont(largeFont);
    m_temperatureLabel->setText(QString("%1 %2")
        .arg(period.temperature, 0, 'f', 1)
        .arg(period.temperatureUnit));

    auto *tempLabel = new QLabel(tr("Temperature"));
    auto *tempLayout = new QVBoxLayout();
    tempLayout->addWidget(m_temperatureLabel);
    tempLayout->addWidget(tempLabel);

    m_windSpeedLabel = new QLabel();
    m_windSpeedLabel->setFont(largeFont);
    m_windSpeedLabel->setText(QString("%1 %2")
        .arg(period.windSpeed)
        .arg(period.windDirection));

    auto *windSpeedLabel = new QLabel(tr("Wind Speed"));
    auto *windSpeedLayout = new QVBoxLayout();
    windSpeedLayout->addWidget(m_windSpeedLabel);
    windSpeedLayout->addWidget(windSpeedLabel);

    m_detailLabel = new QLabel();
    m_detailLabel->setText(period.detailedForecast);
    m_detailLabel->setWordWrap(true);

    m_chanceRainLabel = new QLabel();
    m_chanceRainLabel->setText(tr("Chance of rain %1%").arg(period.probabilityOfPrecipitation));

    QFont periodNameFont = qApp->font();
    periodNameFont.setBold(true);
    periodNameFont.setPointSize(12);
    m_periodNameLabel = new QLabel();
    m_periodNameLabel->setFont(periodNameFont);
    m_periodNameLabel->setText(period.name);

    auto *vLayout = new QVBoxLayout();
    vLayout->addWidget(m_periodNameLabel);

    auto *hLayout = new QHBoxLayout();
    hLayout->addLayout(tempLayout);
    hLayout->addSpacing(20);
    hLayout->addLayout(windSpeedLayout);
    hLayout->addStretch();
    vLayout->addLayout(hLayout);
    vLayout->addSpacing(20);

    if (period.probabilityOfPrecipitation > 0)
        vLayout->addWidget(m_chanceRainLabel);

    vLayout->addWidget(m_detailLabel);

    setLayout(vLayout);
}
