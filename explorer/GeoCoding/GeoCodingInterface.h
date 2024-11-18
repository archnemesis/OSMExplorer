//
// Created by robin on 11/15/2024.
//

#ifndef GEOCODINGINTERFACE_H
#define GEOCODINGINTERFACE_H

#include <QObject>
#include <QString>
#include <QPointF>


class GeoCodingInterface : public QObject {
    Q_OBJECT
public:
    explicit GeoCodingInterface(QObject *parent = nullptr);
    virtual void submitQuery(const QString& query) = 0;

    class GeoCodedAddress {
    public:
        GeoCodedAddress() = default;
        QString formatted;
        QString street;
        QString city;
        QString state;
        QString zip;
        QString county;
        QString country;
        QPointF location;
    };

signals:
    void locationsFound(QList<GeoCodedAddress> locations);
};



#endif //GEOCODINGINTERFACE_H
