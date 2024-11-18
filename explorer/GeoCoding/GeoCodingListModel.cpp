//
// Created by robin on 11/16/2024.
//

#include "GeoCodingListModel.h"
#include "RadarGeoCodingInterface.h"

#include <QDebug>


GeoCodingListModel::GeoCodingListModel(GeoCodingInterface* interface, QObject* parent)
{
    m_interface = interface;
    connect(m_interface,
        &GeoCodingInterface::locationsFound,
        this,
        &GeoCodingListModel::onGeoCodingInterfaceLocationsFound);
}

int GeoCodingListModel::rowCount(const QModelIndex& parent) const
{
    return m_addresses.size();
}

QVariant GeoCodingListModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid()) return {};
    if (role != Qt::EditRole && role != Qt::DisplayRole) return {};
    int row = index.row();
    QString label = m_addresses.at(row).formatted;
    qDebug() << "Getting suggestion" << label;
    return {label};
}

void GeoCodingListModel::updateQuery(const QString& query)
{
    m_interface->submitQuery(query);
}

void GeoCodingListModel::onGeoCodingInterfaceLocationsFound(QList<GeoCodingInterface::GeoCodedAddress> locations)
{
    beginResetModel();
    m_addresses = locations;
    endResetModel();
    emit suggestionsUpdated();
}
