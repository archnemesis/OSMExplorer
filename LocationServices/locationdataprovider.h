#ifndef LOCATIONDATAPROVIDER_H
#define LOCATIONDATAPROVIDER_H

#include "locationservices_global.h"

#include <QObject>
#include <QHash>
#include <QPointF>
#include <QString>
#include <QVariant>

class LOCATIONSERVICESSHARED_EXPORT LocationDataProvider : public QObject
{
    Q_OBJECT
public:
    LocationDataProvider(QObject *parent = nullptr);
    void setUpdateInterval(int interval);
    int updateInterval();

signals:
    void positionUpdated(QString identifier, QPointF position, QHash<QString,QVariant> metadata);
    void dataProviderError();

public slots:
    virtual void start() {}
    virtual void stop() {}

protected:
    int m_updateInterval;
};

#endif // LOCATIONDATAPROVIDER_H
