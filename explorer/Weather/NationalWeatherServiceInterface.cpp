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

void NationalWeatherServiceInterface::getWeatherStationList(const QPointF& location)
{
    m_requestState = StationList;

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

void NationalWeatherServiceInterface::getRelativeLocation(const QPointF &location)
{
    m_requestState = RelativeLocation;

    QString url = QString("https://api.weather.gov/points/%1,%2")
            .arg(location.y(), 0, 'f', 4)
            .arg(location.x(), 0, 'f', 4);
    qDebug() << "Getting forecast data from" << url;

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
        emit requestError(reply->errorString());
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
            emit requestError(tr("Invalid response from API"));
            break;
        }

        QJsonObject properties = document["properties"].toObject();
        QString forecastUrl = properties["forecast"].toString();
        QString hourlyForecastUrl = properties["forecastHourly"].toString();

        QJsonObject relativeLocation = properties["relativeLocation"].toObject();
        QJsonObject relativeLocationProps = relativeLocation["properties"].toObject();
        QString city = relativeLocationProps["city"].toString();
        QString state = relativeLocationProps["state"].toString();

        m_city = city;
        m_state = state;

        m_requestState = Forecast2;
        m_hourlyForecastUrl = hourlyForecastUrl;
        QNetworkRequest request = QNetworkRequest(QUrl(forecastUrl));
        m_networkManager.get(request);

        break;
    }
    case Forecast2:
    case Forecast3: {
        if (!root.contains("properties")) {
            qCritical() << "Properties element missing in response";
            emit requestError(tr("Invalid response from API"));
            break;
        }

        QJsonObject properties = root["properties"].toObject();
        QJsonArray periods = properties["periods"].toArray();

        Forecast12Hr forecast;
        forecast.city = m_city;
        forecast.state = m_state;

        for (const auto& period : periods) {
            QJsonObject periodObject = period.toObject();
            ForecastPeriod fp;
            fp.number = periodObject["number"].toInt();
            fp.name = periodObject["name"].toString();
            fp.startTime = QDateTime::fromString(periodObject["startTime"].toString());
            fp.endTime = QDateTime::fromString(periodObject["endTime"].toString());
            fp.isDaytime = periodObject["isDaytime"].toBool();
            fp.temperature = periodObject["temperature"].toDouble();
            fp.temperatureUnit = periodObject["temperatureUnit"].toString();
            fp.temperatureTrend = periodObject["temperatureTrend"].toString();
            fp.windSpeed = periodObject["windSpeed"].toString();
            fp.windDirection = periodObject["windDirection"].toString();
            fp.icon = periodObject["icon"].toString();
            fp.shortForecast = periodObject["shortForecast"].toString();
            fp.detailedForecast = periodObject["detailedForecast"].toString();

            QJsonObject precip = periodObject["probabilityOfPrecipitation"].toObject();
            fp.probabilityOfPrecipitation = precip["value"].toDouble();

            forecast.periods.append(fp);
        }

        if (m_requestState == Forecast2) {
            m_forecast = forecast;
            emit forecastReady(m_forecast);
            m_requestState = Forecast3;
            QNetworkRequest request = QNetworkRequest(QUrl(m_hourlyForecastUrl));
            m_networkManager.get(request);
        }
        else if (m_requestState == Forecast3) {
            m_forecastHourly = forecast;
            emit hourlyForecastReady(m_forecastHourly);
        }

        break;
    }
    case StationList: {
        if (!root.contains("properties")) {
            qCritical() << "Properties element missing in response";
            emit requestError(tr("Invalid response from API"));
            break;
        }

        QJsonObject properties = document["properties"].toObject();
        QJsonObject relativeLocation = properties["relativeLocation"].toObject();
        QJsonObject relativeLocationGeometry = relativeLocation["geometry"].toObject();
        QJsonObject relativeLocationProperties = relativeLocation["relativeLocation"].toObject();
        QJsonArray relativeLocationGeometryCoords = relativeLocationGeometry["coordinates"].toArray();

        m_gridId = properties["gridId"].toString();
        m_city = relativeLocationProperties["city"].toString();
        m_state = relativeLocationProperties["state"].toString();

        if (relativeLocationGeometryCoords.count() < 2) {
            qCritical() << "Invalid coordinates";
            emit requestError(tr("Invalid response from API"));
            break;
        }

        m_latitude = relativeLocationGeometryCoords[1].toDouble();
        m_longitude = relativeLocationGeometryCoords[0].toDouble();

        m_requestState = StationList2;
        QString url = properties["observationStations"].toString();
        QNetworkRequest request = QNetworkRequest(QUrl(url));
        m_networkManager.get(request);

        break;
    }
    case StationList2: {
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

        emit stationListReady(m_stations);
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

        m_requestState = StationList2;
        QNetworkRequest request = QNetworkRequest(QUrl(m_observationStationsUrl));
        m_networkManager.get(request);

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

        emit latestObservationReady(m_latestObservation);
        break;
    }
    }

}

const NationalWeatherServiceInterface::Forecast12Hr &NationalWeatherServiceInterface::forecast()
{
    return m_forecast;
}

void NationalWeatherServiceInterface::handleError(QString message)
{
    emit requestError(message);
}
