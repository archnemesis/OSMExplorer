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

    explicit NationalWeatherServiceInterface(QObject *parent = nullptr);
    void getForecast(const QPointF& location);
    void getLatestObservation(const QString& stationId);

    const Observation& latestObservation();
    const QString& gridId() const;
    const QString& city() const;
    const QString& state() const;
    const QVector<QPointF>& zonePolygonPoints() const;
    const QList<WeatherStation>& stations() const;
    double latitude() const;
    double longitude() const;

signals:
    void forecastReady();
    void latestObservationReady();

private slots:
    void networkManager_onRequestFinished(QNetworkReply *reply);

private:
    enum RequestState
    {
        Forecast,
        ForecastZone,
        ObservationStations,
        ObservationLatest
    };

    Observation m_latestObservation;
    QList<Observation> m_observations;
    QString m_gridId;
    QString m_city;
    QString m_state;
    QString m_observationStationsUrl;
    QVector<QPointF> m_forecastZone;
    QList<WeatherStation> m_stations;
    QNetworkAccessManager m_networkManager;
    double m_latitude;
    double m_longitude;
    RequestState m_requestState;
};



#endif //NATIONALWEATHERSERVICEINTERFACE_H
