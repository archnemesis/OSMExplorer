//
// Created by robin on 10/29/24.
//

#include "NationalWeatherServiceInterface.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QNetworkRequest>
#include <QNetworkReply>

NationalWeatherServiceInterface::NationalWeatherServiceInterface(QObject* parent)
{
    connect(&m_networkManager,
        &QNetworkAccessManager::finished,
        this,
        &NationalWeatherServiceInterface::networkManager_onRequestFinished);
}

void NationalWeatherServiceInterface::getForecast(const QPointF& location)
{
    m_requestState = Forecast;

    QString url = QString("https://api.weather.gov/points/%1,%2")
                  .arg(location.y(), 0, 'f', 4)
                  .arg(location.x(), 0, 'f', 4);
    qDebug() << "Getting forecast data from" << url;
    QNetworkRequest request = QNetworkRequest(url);
    m_networkManager.get(request);
}

void NationalWeatherServiceInterface::getLatestObservation(const QString& stationId) {
    m_requestState = ObservationLatest;
    QString url = QString("https://api.weather.gov/stations/%1/observations/latest").arg(stationId);
    QNetworkRequest request = QNetworkRequest(url);
    m_networkManager.get(request);
}

const NationalWeatherServiceInterface::Observation& NationalWeatherServiceInterface::latestObservation() {
    return m_latestObservation;
}

const QString& NationalWeatherServiceInterface::gridId() const
{
    return m_gridId;
}

const QString& NationalWeatherServiceInterface::city() const
{
    return m_city;
}

const QString& NationalWeatherServiceInterface::state() const
{
    return m_state;
}

const QVector<QPointF>& NationalWeatherServiceInterface::zonePolygonPoints() const
{
    return m_forecastZone;
}

const QList<NationalWeatherServiceInterface::WeatherStation>& NationalWeatherServiceInterface::stations() const {
    return m_stations;
}

double NationalWeatherServiceInterface::latitude() const
{
    return m_latitude;
}

double NationalWeatherServiceInterface::longitude() const
{
    return m_longitude;
}

void NationalWeatherServiceInterface::networkManager_onRequestFinished(QNetworkReply* reply)
{

    if (reply->error() != QNetworkReply::NoError) {
        qWarning() << "Weather request failed:" << reply->errorString();
        reply->deleteLater();
        return;
    }

    qDebug() << "Processing response...";

    QByteArray replyData = reply->readAll();
    QJsonDocument document = QJsonDocument::fromJson(replyData);
    QJsonObject root = document.object();

    switch (m_requestState) {
    case Forecast: {
        if (!root.contains("properties")) {
            qCritical() << "Properties element missing in response";
            break;
        }

        QJsonObject properties = document["properties"].toObject();
        QJsonObject relativeLocation = properties["relativeLocation"].toObject();
        QJsonObject relativeLocationGeometry = relativeLocation["geometry"].toObject();
        QJsonObject relativeLocationProperties = relativeLocation["relativeLocation"].toObject();
        QJsonArray relativeLocationGeometryCoords = relativeLocationGeometry["coordinates"].toArray();

        // save for later
        m_observationStationsUrl = properties["observationStations"].toString();

        m_gridId = properties["gridId"].toString();
        m_city = relativeLocationProperties["city"].toString();
        m_state = relativeLocationProperties["state"].toString();

        if (relativeLocationGeometryCoords.count() < 2) {
            qCritical() << "Invalid coordinates";
            break;
        }

        m_latitude = relativeLocationGeometryCoords[1].toDouble();
        m_longitude = relativeLocationGeometryCoords[0].toDouble();

        QString forecastZoneUrl = properties["forecastZone"].toString();
        m_requestState = ForecastZone;
        QNetworkRequest request = QNetworkRequest(QUrl(forecastZoneUrl));
        m_networkManager.get(request);

        break;
    }
    case ForecastZone: {
        QJsonObject geometry = document["geometry"].toObject();
        QJsonArray coordinates = geometry["coordinates"].toArray();
        QJsonArray polygon = coordinates[0].toArray();

        m_forecastZone.clear();
        m_forecastZone.reserve(polygon.size());

        for (const auto& point : polygon) {
            QJsonArray pointArray = point.toArray();
            double latitude = pointArray[1].toDouble();
            double longitude = pointArray[0].toDouble();
            m_forecastZone.append(QPointF(longitude, latitude));
        }

        qDebug() << "Document" << document;

        m_requestState = ObservationStations;
        QNetworkRequest request = QNetworkRequest(QUrl(m_observationStationsUrl));
        m_networkManager.get(request);

        break;
    }
    case ObservationStations: {
        m_stations.clear();

        QJsonArray features = document["features"].toArray();
        for (const auto& feature : features) {
            QJsonObject featureObject = feature.toObject();
            QJsonObject featureGeometry = featureObject["geometry"].toObject();
            QJsonObject featureProperties = featureObject["properties"].toObject();
            QJsonArray coordinates = featureGeometry["coordinates"].toArray();

            double longitude = coordinates[0].toDouble();
            double latitude = coordinates[1].toDouble();

            WeatherStation station;
            station.stationId = featureProperties["stationIdentifier"].toString();
            station.latitude = latitude;
            station.longitude = longitude;
            m_stations.append(station);
        }

        emit forecastReady();
        break;
    }
    case ObservationLatest: {
        QJsonObject properties = document["properties"].toObject();
        QJsonObject presentWeather = properties["presentWeather"].toObject();
        QJsonObject temperature = properties["temperature"].toObject();
        QJsonObject relativeHumidity = properties["relativeHumidity"].toObject();
        QJsonObject windDirection = properties["windDirection"].toObject();
        QJsonObject windSpeed = properties["windSpeed"].toObject();
        QJsonObject dewpoint = properties["dewpoint"].toObject();

        double temp_C = temperature["value"].toDouble();
        QString timestamp = properties["timestamp"].toString();
        QDateTime timestampDate = QDateTime::fromString(timestamp, Qt::ISODate);

        Observation obs{};
        obs.temperature = temp_C;
        obs.relativeHumidity = relativeHumidity["value"].toDouble();
        obs.dewPoint = dewpoint["value"].toDouble();
        obs.windDirection = windDirection["value"].toDouble();
        obs.windSpeed = windSpeed["value"].toDouble();
        obs.timestamp = timestampDate;

        m_latestObservation = obs;

        emit latestObservationReady();
        break;
    }
    }

}
