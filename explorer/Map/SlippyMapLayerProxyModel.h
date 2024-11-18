//
// Created by robin on 11/16/2024.
//

#ifndef SLIPPYMAPLAYERPROXYMODEL_H
#define SLIPPYMAPLAYERPROXYMODEL_H

#include <QSortFilterProxyModel>


class SlippyMapLayerProxyModel : public QSortFilterProxyModel {
    Q_OBJECT
public:
    explicit SlippyMapLayerProxyModel(QObject *parent = nullptr);
    bool filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const override;
    bool filterAcceptsColumn(int sourceColumn, const QModelIndex& sourceParent) const override;
};



#endif //SLIPPYMAPLAYERPROXYMODEL_H
