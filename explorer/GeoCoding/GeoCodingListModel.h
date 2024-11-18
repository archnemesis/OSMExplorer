//
// Created by robin on 11/16/2024.
//

#ifndef GEOCODINGLISTMODEL_H
#define GEOCODINGLISTMODEL_H

#include <QAbstractListModel>
#include "GeoCodingInterface.h"


class GeoCodingListModel : public QAbstractListModel {
    Q_OBJECT
public:
    explicit GeoCodingListModel(GeoCodingInterface *interface, QObject *parent = nullptr);
    int rowCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    void updateQuery(const QString& query);

protected:
    GeoCodingInterface *m_interface;
    QList<GeoCodingInterface::GeoCodedAddress> m_addresses;

protected slots:
    void onGeoCodingInterfaceLocationsFound(QList<GeoCodingInterface::GeoCodedAddress> locations);

signals:
    void suggestionsUpdated();
};



#endif //GEOCODINGLISTMODEL_H
