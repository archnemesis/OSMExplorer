//
// Created by robin on 11/15/2024.
//

#ifndef RADARGEOCODINGINTERFACE_H
#define RADARGEOCODINGINTERFACE_H

#include "GeoCodingInterface.h"


class QNetworkAccessManager;
class QNetworkReply;


class RadarGeoCodingInterface : public GeoCodingInterface {
    Q_OBJECT
public:
    explicit RadarGeoCodingInterface(QObject *parent = nullptr);
    void submitQuery(const QString& query) override;

protected slots:
    void replyFinished(QNetworkReply *reply);

protected:
    QNetworkAccessManager *m_networkManager;
};



#endif //RADARGEOCODINGINTERFACE_H
