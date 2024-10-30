//
// Created by Robin on 10/29/2024.
//

#include "WeatherForecastWindow.h"

#include <QTabWidget>
#include <QVBoxLayout>
#include <QScrollArea>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>

#include "Weather/NationalWeatherServiceInterface.h"
#include "Weather/ForecastPeriodWidget.h"

using namespace QtCharts;


WeatherForecastWindow::WeatherForecastWindow(const QPointF& location) :
    QWidget()
{
    m_nationalWeatherService = new NationalWeatherServiceInterface(this);
    connect(m_nationalWeatherService,
            &NationalWeatherServiceInterface::forecastReady,
            this,
            &WeatherForecastWindow::onNationalWeatherService_forecastReady);
    connect(m_nationalWeatherService,
            &NationalWeatherServiceInterface::hourlyForecastReady,
            this,
            &WeatherForecastWindow::onNationalWeatherService_hourlyForecastReady);

    m_tabWidget = new QTabWidget();
    m_tempForecastChart = new QChartView();
    m_location = location;

    setupUi();

    m_nationalWeatherService->getForecast(location);
}

void WeatherForecastWindow::setupUi()
{
    setMinimumWidth(800);
    setMinimumHeight(1000);

    m_forecastLayout = new QVBoxLayout();
    auto *forecastWidget = new QWidget();
    forecastWidget->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Maximum);
    forecastWidget->setLayout(m_forecastLayout);
    auto *forecastScrollArea = new QScrollArea();
    forecastScrollArea->setWidget(forecastWidget);
    forecastScrollArea->setWidgetResizable(true);
    forecastScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

    /*
     * Forecast Tab
     */
    m_tabWidget->addTab(forecastScrollArea, tr("Forecast (12-hour)"));
    m_tabWidget->addTab(m_tempForecastChart, tr("Temperature Trends"));

    auto *vLayout = new QVBoxLayout();
    vLayout->addWidget(m_tabWidget);
    setLayout(vLayout);
}

void WeatherForecastWindow::onNationalWeatherService_forecastReady(
        const NationalWeatherServiceInterface::Forecast12Hr& forecast)
{
    setWindowTitle(tr("Forecast for %1, %2")
        .arg(forecast.city)
        .arg(forecast.state));

    int row = 0;
    for (const auto& period : forecast.periods) {
        auto *widget = new ForecastPeriodWidget(period);
        m_forecastLayout->addWidget(widget);
    }
}

void WeatherForecastWindow::onNationalWeatherService_hourlyForecastReady(
        const NationalWeatherServiceInterface::Forecast12Hr &forecast)
{
    m_tempLineSeries = new QLineSeries();
    int hour = 0;
    for (const auto& period : forecast.periods) {
        m_tempLineSeries->append((qreal)hour, period.temperature);
        hour++;
    }

    auto *chart = new QChart();
    chart->legend()->hide();
    chart->addSeries(m_tempLineSeries);
    chart->createDefaultAxes();
    chart->setTitle("Temperatures");
    m_tempForecastChart->setChart(chart);
    m_tempForecastChart->setRenderHint(QPainter::Antialiasing);
}
