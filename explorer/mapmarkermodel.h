#ifndef MAPMARKERMODEL_H
#define MAPMARKERMODEL_H

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QList>
#include <QMap>

#include "slippymapwidgetmarker.h"

class MapMarkerModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    MapMarkerModel(QObject *parent = nullptr);
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex &index) const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

    class MarkerGroup {
    public:
        MarkerGroup(QString label) {
            m_label = label;
        }
        void addMarker(SlippyMapWidgetMarker *marker) {
            m_markers.append(marker);
        }
        void removeMarker(SlippyMapWidgetMarker *marker) {
            m_markers.removeOne(marker);
        }
        QString label() {
            return m_label;
        }
        QList<SlippyMapWidgetMarker*> markers() {
            return m_markers;
        }
    private:
        QString m_label;
        QList<SlippyMapWidgetMarker*> m_markers;
    };

    void addMarkerGroup(MarkerGroup *group);

private:
    QList<MarkerGroup*> m_groups;
};

#endif // MAPMARKERMODEL_H
