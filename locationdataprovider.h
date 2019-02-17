#ifndef LOCATIONDATAPROVIDER_H
#define LOCATIONDATAPROVIDER_H

#include <QObject>
#include <QHash>
#include <QPointF>
#include <QString>
#include <QVariant>

class LocationDataProvider : public QObject
{
    Q_OBJECT
public:
    explicit LocationDataProvider(QObject *parent = nullptr);
    void setUpdateInterval(int interval);
    int updateInterval();
signals:
    void positionUpdated(QString identifier, QPointF position, QHash<QString,QVariant> metadata);
    void dataProviderError();

public slots:
    virtual void start() = 0;
    virtual void stop() = 0;

protected:
    int m_updateInterval;
};

#endif // LOCATIONDATAPROVIDER_H
