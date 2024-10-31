//
// Created by Robin on 10/29/2024.
//

#include "WeatherForecastWindow.h"

#include <QTabWidget>
#include <QVBoxLayout>
#include <QScrollArea>
#include <QMessageBox>
#include <QSettings>
#include <QTimer>
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
    connect(m_nationalWeatherService,
            &NationalWeatherServiceInterface::requestError,
            this,
            &WeatherForecastWindow::onNationalWeatherService_requestError);

    m_tabWidget = new QTabWidget();
    m_tempForecastChart = new QChartView();
    m_location = location;

    m_windowResizeTimer = new QTimer();
    m_windowResizeTimer->setSingleShot(true);
    m_windowResizeTimer->setInterval(100);
    connect(m_windowResizeTimer,
            &QTimer::timeout,
            this,
            &WeatherForecastWindow::onWindowResizeTimer_timeout);

    setupUi();
    loadSettings();

    m_loadingMessageBox = new QMessageBox(this);
    m_loadingMessageBox->setText(tr("Loading forecast data..."));
    m_loadingMessageBox->setWindowTitle(tr("Loading"));
    m_loadingMessageBox->show();
    m_nationalWeatherService->getForecast(location);
}

void WeatherForecastWindow::setupUi()
{
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

    m_loadingMessageBox->hide();
}

void WeatherForecastWindow::onNationalWeatherService_requestError(QString errorMessage)
{
    m_loadingMessageBox->hide();
    QMessageBox::critical(
            this,
            tr("Forecast Error"),
            tr("There was an error getting the forecast: %1").arg(errorMessage)
            );
}

void WeatherForecastWindow::saveWindowSize()
{
    QSettings settings;
    settings.setValue("weatherForecastWindow/windowWidth", width());
    settings.setValue("weatherForecastWindow/windowHeight", height());
}

void WeatherForecastWindow::onWindowResizeTimer_timeout()
{
    saveWindowSize();
}

void WeatherForecastWindow::resizeEvent(QResizeEvent *event)
{
    m_windowResizeTimer->stop();
    m_windowResizeTimer->start();
}

void WeatherForecastWindow::loadSettings()
{
    QSettings settings;

    if (settings.contains("weatherForecastWindow/windowWidth") \
        && settings.contains("weatherForecastWindow/windowHeight")) {
        int width = settings.value("weatherForecastWindow/windowWidth").toInt();
        int height = settings.value("weatherForecastWindow/windowHeight").toInt();
        resize(width, height);
    }
}
