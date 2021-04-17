#ifndef SLIPPYMAPLAYERMANAGER_H
#define SLIPPYMAPLAYERMANAGER_H

#include "slippymap_global.h"

#include <QObject>
#include <QAbstractItemModel>
#include <QList>
#include <QFont>

class SlippyMapLayer;
class SlippyMapLayerObject;

class SLIPPYMAPSHARED_EXPORT SlippyMapLayerManager : public QAbstractItemModel
{
    Q_OBJECT
public:
    explicit SlippyMapLayerManager(QObject *parent = nullptr);

    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex &index) const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

    void addLayer(SlippyMapLayer *layer);
    void addLayerObject(SlippyMapLayer *layer, SlippyMapLayerObject *object);
    void addLayerObject(SlippyMapLayerObject *object);
    void takeLayer(SlippyMapLayer *layer);
    void setActiveLayer(SlippyMapLayer *layer);
    void setDefaultLayer(SlippyMapLayer *layer);
    void saveToFile(QString fileName);

    bool contains(SlippyMapLayerObject *object);

    QList<SlippyMapLayer*> layers();
    SlippyMapLayer *activeLayer();
    SlippyMapLayer *defaultLayer();
    QList<SlippyMapLayerObject*> objectsAtPoint(QPointF point, int zoomLevel);

signals:
    void activeLayerChanged(SlippyMapLayer *layer);
    void layerAdded(SlippyMapLayer *layer);
    void layerRemoved(SlippyMapLayer *layer);
    void layerObjectAdded(SlippyMapLayer *layer, SlippyMapLayerObject *object);
    void layerObjectRemoved(SlippyMapLayer *layer, SlippyMapLayerObject *object);
protected:
    QList<SlippyMapLayer*> m_layers;
    SlippyMapLayer *m_activeLayer = nullptr;
    SlippyMapLayer *m_defaultLayer = nullptr;
    QFont m_hiddenFont;
    QFont m_activeFont;
};

#endif // SLIPPYMAPLAYERMANAGER_H
