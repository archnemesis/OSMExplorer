#include "slippymapwidgetmarkermodel.h"
#include "slippymapwidgetmarkergroup.h"

SlippyMapWidgetMarkerModel::SlippyMapWidgetMarkerModel(QObject *parent) :
    QAbstractItemModel (parent)
{

}

QModelIndex SlippyMapWidgetMarkerModel::index(int row, int column, const QModelIndex &parent) const
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
        SlippyMapWidgetMarkerGroup *group = static_cast<SlippyMapWidgetMarkerGroup*>(parent.internalPointer());
        if (m_groups.contains(group)) {
            return createIndex(row, column, group->markers().at(row));
        }
        else {
            return QModelIndex();
        }
    }
}

QModelIndex SlippyMapWidgetMarkerModel::parent(const QModelIndex &index) const
{
    if (!index.isValid()) {
        return QModelIndex();
    }

    SlippyMapWidgetMarkerGroup *group = static_cast<SlippyMapWidgetMarkerGroup*>(index.internalPointer());
    if (m_groups.contains(group)) {
        return QModelIndex();
    }

    SlippyMapWidgetMarker *marker = static_cast<SlippyMapWidgetMarker*>(index.internalPointer());
    for (SlippyMapWidgetMarkerGroup *grp : m_groups) {
        if (grp->markers().contains(marker)) {
            return createIndex(grp->markers().indexOf(marker), 0, grp);
        }
    }

    return QModelIndex();
}

int SlippyMapWidgetMarkerModel::rowCount(const QModelIndex &parent) const
{
    if (parent.column() > 0) {
        return 0;
    }

    if (!parent.isValid()) {
        return m_groups.count();
    }

    SlippyMapWidgetMarkerGroup *group = static_cast<SlippyMapWidgetMarkerGroup*>(parent.internalPointer());
    if (m_groups.contains(group)) {
        return group->markers().length();
    }

    return 0;
}

int SlippyMapWidgetMarkerModel::columnCount(const QModelIndex &parent) const
{
    (void)parent;
    return 3;
}

QVariant SlippyMapWidgetMarkerModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    if (role != Qt::DisplayRole) {
        return QVariant();
    }

    SlippyMapWidgetMarkerGroup *group = static_cast<SlippyMapWidgetMarkerGroup*>(index.internalPointer());
    if (index.column() == 0 && m_groups.contains(group)) {
        return group->label();
    }

    SlippyMapWidgetMarker *marker = static_cast<SlippyMapWidgetMarker*>(index.internalPointer());
    for (SlippyMapWidgetMarkerGroup *grp : m_groups) {
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

QVariant SlippyMapWidgetMarkerModel::headerData(int section, Qt::Orientation orientation, int role) const
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

QList<SlippyMapWidgetMarker *> SlippyMapWidgetMarkerModel::markersForRect(QRectF rect) const
{
    QList<SlippyMapWidgetMarker *> ret;

    for (SlippyMapWidgetMarkerGroup *group : m_groups) {
        for (SlippyMapWidgetMarker *marker : group->markers()) {
            if (rect.contains(marker->position())) {
                ret.append(marker);
            }
        }
    }

    return ret;
}

void SlippyMapWidgetMarkerModel::addMarkerGroup(SlippyMapWidgetMarkerGroup *group)
{
    m_groups.append(group);
    connect(group, &SlippyMapWidgetMarkerGroup::markerAdded, this, &SlippyMapWidgetMarkerModel::onMarkerGroupMarkerAdded);
    connect(group, &SlippyMapWidgetMarkerGroup::markerRemoved, this, &SlippyMapWidgetMarkerModel::onMarkerGroupMarkerRemoved);
    emit groupAdded(group);
}

void SlippyMapWidgetMarkerModel::removeMarkerGroup(SlippyMapWidgetMarkerGroup *group)
{
    disconnect(group, &SlippyMapWidgetMarkerGroup::markerAdded, this, &SlippyMapWidgetMarkerModel::onMarkerGroupMarkerAdded);
    disconnect(group, &SlippyMapWidgetMarkerGroup::markerRemoved, this, &SlippyMapWidgetMarkerModel::onMarkerGroupMarkerRemoved);
    m_groups.removeOne(group);
    emit groupRemoved(group);
}

void SlippyMapWidgetMarkerModel::onMarkerGroupMarkerAdded(SlippyMapWidgetMarker *marker)
{
    emit markerAdded(marker);
}

void SlippyMapWidgetMarkerModel::onMarkerGroupMarkerRemoved(SlippyMapWidgetMarker *marker)
{
    emit markerRemoved(marker);
}
