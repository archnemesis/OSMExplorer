#ifndef APRSFIMARKERPROVIDER_H
#define APRSFIMARKERPROVIDER_H

#include "slippymapwidgetmarkerprovider.h"

#include <QString>
#include <QStringList>
#include <QTimer>
#include <QNetworkAccessManager>
#include <QNetworkReply>

class AprsFiMarkerProvider : public SlippyMapWidgetMarkerProvider
{
    Q_OBJECT

public:
    AprsFiMarkerProvider(QObject *parent = nullptr);
    void setUpdateInterval(int interval);
    void setApiUrl(QString url);
    void setApiKey(QString key);
    void setCallsigns(QStringList callsigns);

    int updateInterval();
    QString apiUrl();
    QString apiKey();
    QStringList callsigns();

    void start();
    void stop();

public slots:
    void update();

private:
    int m_updateInterval;
    QString m_apiUrl;
    QString m_apiKey;
    QStringList m_callsigns;
    QNetworkAccessManager *m_net;
    QNetworkReply *m_reply;
    QTimer m_requestTimer;
};

#endif // APRSFIMARKERPROVIDER_H
