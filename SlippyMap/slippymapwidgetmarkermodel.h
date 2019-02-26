#ifndef MAPMARKERMODEL_H
#define MAPMARKERMODEL_H

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QList>
#include <QMap>

#include "slippymapwidgetmarker.h"
#include "slippymapwidgetmarkergroup.h"

#if defined EXPORT_SYMBOLS
#define DECLARATION Q_DECL_EXPORT
#else
#define DECLARATION Q_DECL_IMPORT
#endif

class DECLARATION SlippyMapWidgetMarkerModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    SlippyMapWidgetMarkerModel(QObject *parent = nullptr);
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex &index) const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
    QList<SlippyMapWidgetMarker*> markersForRect(QRectF rect) const;

    void addMarkerGroup(SlippyMapWidgetMarkerGroup *group);
    void removeMarkerGroup(SlippyMapWidgetMarkerGroup *group);
    bool contains(SlippyMapWidgetMarker *marker);

protected slots:
    void onMarkerGroupMarkerAdded(SlippyMapWidgetMarker *marker);
    void onMarkerGroupMarkerRemoved(SlippyMapWidgetMarker *marker);
    void onMarkerChanged();

signals:
    void markerAdded(SlippyMapWidgetMarker *marker);
    void markerRemoved(SlippyMapWidgetMarker *marker);
    void groupAdded(SlippyMapWidgetMarkerGroup *group);
    void groupRemoved(SlippyMapWidgetMarkerGroup *group);
    void updated();

private:
    QList<SlippyMapWidgetMarkerGroup*> m_groups;
};

#endif // MAPMARKERMODEL_H
