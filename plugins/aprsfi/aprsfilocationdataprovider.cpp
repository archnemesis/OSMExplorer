#include "aprsfilocationdataprovider.h"

#include <QDebug>

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrl>
#include <QByteArray>

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonParseError>

#include <QPointF>

AprsFiLocationDataProvider::AprsFiLocationDataProvider(QObject *parent) :
    LocationDataProvider(parent),
    m_net(new QNetworkAccessManager())
{
    connect(m_net, &QNetworkAccessManager::finished, this, &AprsFiLocationDataProvider::onNetworkRequestFinished);
}

AprsFiLocationDataProvider::AprsFiLocationDataProvider(QObject *parent, QString apiUrl, QString apiKey) :
    LocationDataProvider(parent),
    m_apiUrl(apiUrl),
    m_apiKey(apiKey),
    m_net(new QNetworkAccessManager)
{
    connect(m_net, &QNetworkAccessManager::finished, this, &AprsFiLocationDataProvider::onNetworkRequestFinished);
}

void AprsFiLocationDataProvider::setApiUrl(QString url)
{
    m_apiUrl = url;
}

void AprsFiLocationDataProvider::setApiKey(QString key)
{
    m_apiKey = key;
}

void AprsFiLocationDataProvider::setCallsigns(QStringList callsigns)
{
    m_callsigns = callsigns;
}

QString AprsFiLocationDataProvider::apiUrl()
{
    return m_apiUrl;
}

QString AprsFiLocationDataProvider::apiKey()
{
    return m_apiKey;
}

QStringList AprsFiLocationDataProvider::callsigns()
{
    return m_callsigns;
}

void AprsFiLocationDataProvider::updateLocations()
{
    if (m_callsigns.size() == 0) return;

    QString callsignList = m_callsigns.join(",");
    QString url = QString("%1/get?name=%2&what=loc&apikey=%3&format=json")
            .arg(m_apiUrl)
            .arg(callsignList)
            .arg(m_apiKey);
    QNetworkRequest req(url);
    req.setHeader(QNetworkRequest::UserAgentHeader, "OSMExplorer/1.0.0alpha1 (+https://github.com/archnemesis/OSMExplorer)");
    qDebug() << "Requesting data from:" << url;
    m_net->get(req);
}

void AprsFiLocationDataProvider::onNetworkRequestFinished(QNetworkReply *reply)
{
    if (reply->error() != QNetworkReply::NoError) {
        qCritical() << "Unable to fetch APRS data!";
        reply->deleteLater();
        return;
    }

    QByteArray data = reply->readAll();

    qDebug() << "Got response:" << data;

    QJsonParseError error;
    QJsonDocument document = QJsonDocument::fromJson(data, &error);

    if (error.error != QJsonParseError::NoError) {
        qCritical() << "JSON Parse Error:" << error.errorString();
        return;
    }

    QJsonObject root = document.object();

    if (!root.contains("result") || root["result"].toString() != "ok") {
        qCritical() << "Request Error!";
        return;
    }

    QJsonArray entries = root["entries"].toArray();
    for (int i = 0; i < entries.count(); i++) {
        QJsonObject entry = entries[i].toObject();
        double lat = entry["lat"].toString().toDouble();
        double lon = entry["lng"].toString().toDouble();
        QString ident = entry["name"].toString();

        QHash<QString,QVariant> meta;

        if (entry.contains("class"))
            meta["aprs.fi_class"] = entry["class"].toVariant();
        if (entry.contains("type"))
            meta["aprs.fi_type"] = entry["type"].toVariant();
        if (entry.contains("time"))
            meta["aprs.fi_time"] = entry["time"].toVariant();
        if (entry.contains("lasttime"))
            meta["aprs.fi_lasttime"] = entry["lasttime"].toVariant();
        if (entry.contains("symbol"))
            meta["aprs.fi_symbol"] = entry["symbol"].toVariant();
        if (entry.contains("srccall"))
            meta["aprs.fi_srccall"] = entry["srccall"].toVariant();
        if (entry.contains("dstcall"))
            meta["aprs.fi_dstcall"] = entry["dstcall"].toVariant();
        if (entry.contains("phg"))
            meta["aprs.fi_phg"] = entry["phg"].toVariant();
        if (entry.contains("comment"))
            meta["aprs.fi_comment"] = entry["comment"].toVariant();
        if (entry.contains("path"))
            meta["aprs.fi_path"] = entry["path"].toVariant();

        emit positionUpdated(ident, QPointF(lon,lat), meta);
    }
}

void AprsFiLocationDataProvider::start()
{
    m_requestTimer.setInterval(m_updateInterval * 60 * 1000);
    m_requestTimer.setSingleShot(false);
    connect(&m_requestTimer, &QTimer::timeout, this, &AprsFiLocationDataProvider::updateLocations);
    m_requestTimer.start();
    updateLocations();
}

void AprsFiLocationDataProvider::stop()
{
    m_requestTimer.stop();
}

