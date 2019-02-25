#include "mapmarkermodel.h"

MapMarkerModel::MapMarkerModel(QObject *parent) :
    QAbstractItemModel (parent)
{

}

QModelIndex MapMarkerModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent)) {
        return QModelIndex();
    }

    if (!parent.isValid()) {
        if (m_groups.count() > row) {
            return createIndex(row, column, m_groups.at(row));
        }
        else {
            return QModelIndex();
        }
    }
    else {
        MarkerGroup *group = static_cast<MarkerGroup*>(parent.internalPointer());
        if (m_groups.contains(group)) {
            return createIndex(row, column, group->markers().at(row));
        }
        else {
            return QModelIndex();
        }
    }
}

QModelIndex MapMarkerModel::parent(const QModelIndex &index) const
{
    if (!index.isValid()) {
        return QModelIndex();
    }

    MarkerGroup *group = static_cast<MarkerGroup*>(index.internalPointer());
    if (m_groups.contains(group)) {
        return QModelIndex();
    }

    SlippyMapWidgetMarker *marker = static_cast<SlippyMapWidgetMarker*>(index.internalPointer());
    for (MarkerGroup *grp : m_groups) {
        if (grp->markers().contains(marker)) {
            return createIndex(grp->markers().indexOf(marker), 0, grp);
        }
    }

    return QModelIndex();
}

int MapMarkerModel::rowCount(const QModelIndex &parent) const
{
    if (parent.column() > 0) {
        return 0;
    }

    if (!parent.isValid()) {
        return m_groups.count();
    }

    MarkerGroup *group = static_cast<MarkerGroup*>(parent.internalPointer());
    if (m_groups.contains(group)) {
        return group->markers().length();
    }

    return 0;
}

int MapMarkerModel::columnCount(const QModelIndex &parent) const
{
    return 3;
}

QVariant MapMarkerModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    if (role != Qt::DisplayRole) {
        return QVariant();
    }

    MarkerGroup *group = static_cast<MarkerGroup*>(index.internalPointer());
    if (index.column() == 0 && m_groups.contains(group)) {
        return group->label();
    }

    SlippyMapWidgetMarker *marker = static_cast<SlippyMapWidgetMarker*>(index.internalPointer());
    for (MarkerGroup *grp : m_groups) {
        if (grp->markers().contains(marker)) {
            switch (index.column()) {
            case 0:
                return marker->label();
            case 1:
                return QVariant(marker->latitude());
            case 2:
                return QVariant(marker->longitude());
            }
        }
    }

    return QVariant();
}

QVariant MapMarkerModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal) {
        if (role == Qt::DisplayRole) {
            switch (section) {
            case 0: return QString("Name");
            case 1: return QString("Latitude");
            case 2: return QString("Longitude");
            }
        }
    }

    return QVariant();
}

void MapMarkerModel::addMarkerGroup(MapMarkerModel::MarkerGroup *group)
{
    m_groups.append(group);
}
