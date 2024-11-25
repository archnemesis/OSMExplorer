//
// Created by Robin on 10/29/2024.
//

#ifndef OSMEXPLORER_WEATHERFORECASTWINDOW_H
#define OSMEXPLORER_WEATHERFORECASTWINDOW_H

#include <QWidget>

#include "Weather/NationalWeatherServiceInterface.h"


class QTableWidget;
class QTabWidget;
class QVBoxLayout;
class QMessageBox;
class QTimer;

namespace QtCharts {
    class QChartView;
    class QLineSeries;
}

class WeatherForecastWindow : public QWidget
{
    Q_OBJECT
public:
    explicit WeatherForecastWindow(const QPointF& location);

protected:
    void resizeEvent(QResizeEvent *event) override;

protected slots:
    void onNationalWeatherService_forecastReady(
            const NationalWeatherServiceInterface::Forecast12Hr& forecast);
    void onNationalWeatherService_hourlyForecastReady(
            const NationalWeatherServiceInterface::Forecast12Hr& forecast);
    void onNationalWeatherService_requestError(
            QString errorMessage
            );
    void onWindowResizeTimer_timeout();

private:
    void setupUi();
    void saveWindowSize();
    void loadSettings();

    QPointF m_location;
    QString m_stationId;
    QTimer *m_windowResizeTimer;
    NationalWeatherServiceInterface *m_nationalWeatherService;
    QTableWidget *m_forecastTable;
    QTabWidget *m_tabWidget;
    QVBoxLayout *m_forecastLayout;
    QMessageBox *m_loadingMessageBox;
    QtCharts::QLineSeries *m_tempLineSeries;
    QtCharts::QChartView *m_tempForecastChart;
};


#endif //OSMEXPLORER_WEATHERFORECASTWINDOW_H
