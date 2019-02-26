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
    SlippyMapWidgetMarkerProvider (parent)
{
    connect(&m_requestTimer, &QTimer::timeout, this, &AprsFiMarkerProvider::update);
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
    m_requestTimer.setInterval(m_updateInterval);
    m_requestTimer.setSingleShot(false);
    m_requestTimer.start();
}

void AprsFiMarkerProvider::stop()
{
    m_requestTimer.stop();
}

void AprsFiMarkerProvider::update()
{

}
