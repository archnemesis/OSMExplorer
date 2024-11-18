//
// Created by robin on 11/15/2024.
//

#include "RadarGeoCodingInterface.h"

#include <filesystem>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>


RadarGeoCodingInterface::RadarGeoCodingInterface(QObject* parent) : GeoCodingInterface(parent)
{
    m_networkManager = new QNetworkAccessManager(this);
    connect(m_networkManager,
        &QNetworkAccessManager::finished,
        this,
        &RadarGeoCodingInterface::replyFinished);
}

void RadarGeoCodingInterface::submitQuery(const QString& query)
{
    QString encodedQuery = QUrl::toPercentEncoding(query);
    QString url = QString("https://api.radar.io/v1/geocode/forward?query=%1").arg(encodedQuery);
    QNetworkRequest request;
    request.setUrl(QUrl(url));
    request.setRawHeader("Authorization", "prj_test_pk_87ec7334c55e813708838d44bf86d636c807f06a");
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    m_networkManager->get(request);
}

void RadarGeoCodingInterface::replyFinished(QNetworkReply* reply)
{
    QList<GeoCodedAddress> results;

    if (reply->error() != QNetworkReply::NoError) {
        qCritical() << "Request failed:" << reply->errorString();
        return;
    }

    auto data = reply->readAll();
    auto doc = QJsonDocument::fromJson(data);
    auto root = doc.object();

    if (!root.contains("addresses")) {
        qWarning() << "No addresses found in response";
        return;
    }

    auto addresses = root.value("addresses").toArray();

    for (auto address : addresses) {
        auto addressObject = address.toObject();
        qDebug() << "Got result";
        qDebug() << "Coordinates:" << addressObject["latitude"] << addressObject["longitude"];

        QString confidence = addressObject["confidence"].toString();

        if (confidence == "exact") {
            double lat = addressObject["latitude"].toDouble();
            double lon = addressObject["longitude"].toDouble();

            GeoCodedAddress addr;
            addr.formatted = addressObject["formattedAddress"].toString();
            addr.street = addressObject["street"].toString();
            addr.city = addressObject["city"].toString();
            addr.state = addressObject["state"].toString();
            addr.county = addressObject["county"].toString();
            addr.country = addressObject["country"].toString();
            addr.location = QPointF(lon, lat);
            results.append(addr);
        }
    }

    emit locationsFound(results);
}
