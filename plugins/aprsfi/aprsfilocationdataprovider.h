#ifndef APRSFILOCATIONDATAPROVIDER_H
#define APRSFILOCATIONDATAPROVIDER_H

#include "locationdataprovider.h"
#include <QTimer>

class QNetworkAccessManager;
class QNetworkReply;

class AprsFiLocationDataProvider : public LocationDataProvider
{
    Q_OBJECT
public:
    AprsFiLocationDataProvider(QObject *parent = nullptr);
    AprsFiLocationDataProvider(QObject *parent, QString apiUrl, QString apiKey);
    void setApiUrl(QString url);
    void setApiKey(QString key);
    void setCallsigns(QStringList callsigns);
    QString apiUrl();
    QString apiKey();
    QStringList callsigns();
protected slots:
    void onNetworkRequestFinished(QNetworkReply *reply);
public slots:
    void start() override;
    void stop() override;
    void updateLocations();
protected:
    QString m_apiUrl;
    QString m_apiKey;
    QStringList m_callsigns;
    QNetworkAccessManager *m_net;
    QNetworkReply *m_reply;
    QTimer m_requestTimer;
};

#endif // APRSFILOCATIONDATAPROVIDER_H
