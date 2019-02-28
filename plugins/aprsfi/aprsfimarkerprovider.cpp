#include "aprsfimarkerprovider.h"

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

AprsFiMarkerProvider::AprsFiMarkerProvider(QObject *parent) :
    SlippyMapWidgetMarkerProvider (parent),
    m_net (new QNetworkAccessManager())
{
    connect(&m_requestTimer, &QTimer::timeout, this, &AprsFiMarkerProvider::update);
    connect(m_net, &QNetworkAccessManager::finished, this, &AprsFiMarkerProvider::onNetworkRequestFinished);
}

void AprsFiMarkerProvider::setUpdateInterval(int interval)
{
    m_updateInterval = interval;
}

void AprsFiMarkerProvider::setApiUrl(QString url)
{
    m_apiUrl = url;
}

void AprsFiMarkerProvider::setApiKey(QString key)
{
    m_apiKey = key;
}

void AprsFiMarkerProvider::setCallsigns(QStringList callsigns)
{
    m_callsigns = callsigns;

    for (QString ident : m_markers.keys()) {
        if (!callsigns.contains(ident)) {
            emit markerRemoved(m_markers[ident]);
            m_markers.remove(ident);
        }
    }
}

int AprsFiMarkerProvider::updateInterval()
{
    return m_updateInterval;
}

QString AprsFiMarkerProvider::apiUrl()
{
    return m_apiUrl;
}

QString AprsFiMarkerProvider::apiKey()
{
    return m_apiKey;
}

QStringList AprsFiMarkerProvider::callsigns()
{
    return m_callsigns;
}

void AprsFiMarkerProvider::start()
{
    m_requestTimer.setInterval(m_updateInterval * 60 * 1000);
    m_requestTimer.setSingleShot(false);
    m_requestTimer.start();
}

void AprsFiMarkerProvider::stop()
{
    m_requestTimer.stop();
}

void AprsFiMarkerProvider::update()
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

void AprsFiMarkerProvider::onNetworkRequestFinished(QNetworkReply *reply)
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

    qDebug() << "Processing" << entries.count() << "records:";

    for (int i = 0; i < entries.count(); i++) {
        QJsonObject entry = entries[i].toObject();
        double lat = entry["lat"].toString().toDouble();
        double lon = entry["lng"].toString().toDouble();
        QPointF point(lon, lat);
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

        QStringList info;

        for (QString key : meta.keys()) {
            info << QString("%1: %2").arg(key).arg(meta[key].toString());
        }

        QString infoString = info.join("<br/>");

        SlippyMapWidgetMarker *marker;
        if (m_markers.contains(ident)) {
            qDebug() << "Updating marker...";
            marker = m_markers[ident];
            marker->setPosition(point);
            marker->setLabel(ident);
            marker->setInformation(infoString);
        }
        else {
            qDebug() << "Creating marker...";
            marker = new SlippyMapWidgetMarker(point);
            marker->setLabel(ident);
            marker->setInformation(infoString);
            marker->setEditable(false);
            marker->setMarkerColor(Qt::red);
            marker->setMovable(false);
            m_markers[ident] = marker;
            emit markerCreated(marker);
        }
    }
}
