//
// Created by robin on 11/16/2024.
//

#include "SlippyMapLayerProxyModel.h"

#include "SlippyMap/SlippyMapLayer.h"

SlippyMapLayerProxyModel::SlippyMapLayerProxyModel(QObject* parent) : QSortFilterProxyModel(parent)
{

}

bool SlippyMapLayerProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const
{
    auto index = sourceModel()->index(sourceRow, 0, sourceParent);
    auto *ptr = static_cast<SlippyMap::SlippyMapLayer*>(index.internalPointer());
    if (ptr != nullptr) {
        if (ptr->showInLayerView())
            return true;
    }
    return false;
}

bool SlippyMapLayerProxyModel::filterAcceptsColumn(int sourceColumn, const QModelIndex& sourceParent) const
{
    return true;
}
