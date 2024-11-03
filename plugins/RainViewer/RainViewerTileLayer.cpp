//
// Created by robin on 10/4/24.
//

#include "RainViewerTileLayer.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QJsonArray>

RainViewerSlippyMapTileLayer::RainViewerSlippyMapTileLayer(QObject *parent)
        : SlippyMapAnimatedLayer(parent)
        , m_ready(false)
        , m_currentFrame(0)
{
    m_nam = new QNetworkAccessManager(this);
    connect(m_nam,
            &QNetworkAccessManager::finished,
            this,
            &RainViewerSlippyMapTileLayer::nam_onFinished);

    setName("Radar");
    setDescription("Radar reflectivity in 10-minute intervals.");
    setBaseLayer(false);
    setCacheDuration(86400);
}

void RainViewerSlippyMapTileLayer::nam_onFinished(QNetworkReply *reply)
{
    if (reply->error() != QNetworkReply::NoError) {
        qCritical() << "API request failed:" << reply->errorString();
        return;
    }

    QByteArray replyData = reply->readAll();
    QJsonDocument document = QJsonDocument::fromJson(replyData);

    if (document.isEmpty()) {
        qCritical() << "No response from API";
        return;
    }

    if (!document.isObject()) {
        qCritical() << "Unexpected JSON element";
        return;
    }

    QJsonObject root = document.object();

    if (!root.contains("version") || !root.contains("host")) {
        qCritical() << "Missing version and/or host";
        return;
    }

    if (!root.contains("radar")) {
        qCritical() << "No radar information";
        return;
    }

    // API params
    QString size("256");
    QString color("1");
    QString options("1_0");

    QString host = root["host"].toString();
    QJsonObject radar = root["radar"].toObject();
    QJsonArray radarPast = radar["past"].toArray();

    for (int i = 0; i < radarPast.count(); i++) {
        const QJsonObject& radarPastLastFrame = radarPast[i].toObject();
        QString radarPath = radarPastLastFrame["path"].toString();
        QString tileUrl = QString("%1%2/%3/%4/%5/%6.png")
                .arg(host,
                     radarPath,
                     size,
                     "%1/%2/%3",
                     color,
                     options);

        m_radarFrameUrls.append(tileUrl);
    }

    m_currentFrame = 0;
    m_ready = true;
    setTileUrl(m_radarFrameUrls.at(m_currentFrame));
    emit updated();
}

void RainViewerSlippyMapTileLayer::update()
{
    m_nam->get(
            QNetworkRequest(
                    QUrl("https://api.rainviewer.com/public/weather-maps.json")));
}

bool RainViewerSlippyMapTileLayer::isVisible()
{
    return m_ready && SlippyMapWidgetLayer::isVisible();
}

void RainViewerSlippyMapTileLayer::nextFrame()
{
    if (!m_ready) return;
    if (++m_currentFrame == m_radarFrameUrls.count())
        m_currentFrame = 0;
    setTileUrl(m_radarFrameUrls.at(m_currentFrame));
    emit updated();
}

void RainViewerSlippyMapTileLayer::previousFrame()
{
    if (!m_ready) return;
    if (--m_currentFrame < 0)
        m_currentFrame = m_radarFrameUrls.count() - 1;
    setTileUrl(m_radarFrameUrls.at(m_currentFrame));
    emit updated();
}