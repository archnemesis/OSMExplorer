//
// Created by robin on 10/29/24.
//

#ifndef NATIONALWEATHERSERVICEINTERFACE_H
#define NATIONALWEATHERSERVICEINTERFACE_H

#include <qjsonvalue.h>
#include <QObject>
#include <QVector>
#include <QNetworkAccessManager>
#include <QPointF>
#include <QDateTime>


class NationalWeatherServiceInterface : public QObject {
    Q_OBJECT
public:
    struct WeatherStation
    {
        QString stationId;
        double latitude;
        double longitude;
    };

    struct Observation
    {
        double temperature;
        double relativeHumidity;
        double dewPoint;
        double windDirection;
        double windSpeed;
        QDateTime timestamp;
    };

    struct ForecastPeriod
    {
        int number;
        QString name;
        QDateTime startTime;
        QDateTime endTime;
        bool isDaytime;
        double temperature;
        QString temperatureUnit;
        QString temperatureTrend;
        double probabilityOfPrecipitation;
        QString windSpeed;
        QString windDirection;
        QString icon;
        QString shortForecast;
        QString detailedForecast;
    };

    struct Forecast12Hr
    {
        QString city;
        QString state;
        QDateTime generatedAt;
        QDateTime updatedAt;
        double elevation;
        QList<ForecastPeriod> periods;
    };

    explicit NationalWeatherServiceInterface(QObject *parent = nullptr);
    void getWeatherStationList(const QPointF& location);
    void getLatestObservation(const QString& stationId);
    void getForecast(const QPointF& location);
    void getRelativeLocation(const QPointF& location);

    const Forecast12Hr& forecast();
    const Observation& latestObservation();
    const QString& gridId() const;
    const QString& city() const;
    const QString& state() const;
    const QVector<QPointF>& zonePolygonPoints() const;
    const QList<WeatherStation>& stations() const;
    double latitude() const;
    double longitude() const;

signals:
    void stationListReady(const QList<WeatherStation>& stations);
    void forecastReady(const Forecast12Hr& forecast);
    void hourlyForecastReady(const Forecast12Hr& forecast);
    void latestObservationReady(const Observation& observation);
    void receivedGridId(QString gridId);
    void requestError(QString errorString);

private slots:
    void networkManager_onRequestFinished(QNetworkReply *reply);

private:
    void handleError(QString message);

    enum RequestState
    {
        Forecast,
        StationList,
        Forecast2,
        Forecast3,
        ForecastZone,
        StationList2,
        ObservationLatest,
        RelativeLocation,
        RelativeLocation2
    };

    Observation m_latestObservation;
    Forecast12Hr m_forecast;
    Forecast12Hr m_forecastHourly;
    QList<Observation> m_observations;
    QString m_gridId;
    QString m_city;
    QString m_state;
    QString m_observationStationsUrl;
    QString m_forecastUrl;
    QString m_hourlyForecastUrl;
    QVector<QPointF> m_forecastZone;
    QList<WeatherStation> m_stations;
    QNetworkAccessManager m_networkManager;
    double m_latitude;
    double m_longitude;
    RequestState m_requestState;
    RequestState m_nextRequestState;
};



#endif //NATIONALWEATHERSERVICEINTERFACE_H
