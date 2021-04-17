#ifndef APRSFIMARKERPROVIDER_H
#define APRSFIMARKERPROVIDER_H

#include "slippymaplayerobjectprovider.h"

#include <QMap>
#include <QString>
#include <QStringList>
#include <QTimer>
#include <QNetworkAccessManager>
#include <QNetworkReply>

class AprsFiMarkerProvider : public SlippyMapLayerObjectProvider
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

protected slots:
    void onNetworkRequestFinished(QNetworkReply *reply);

private:
    int m_updateInterval;
    QString m_apiUrl;
    QString m_apiKey;
    QStringList m_callsigns;
    QNetworkAccessManager *m_net;
    QNetworkReply *m_reply;
    QTimer m_requestTimer;
    QMap<QString,SlippyMapLayerObject*> m_objects;
};

#endif // APRSFIMARKERPROVIDER_H
