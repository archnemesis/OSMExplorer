#include "slippymaplayermanager.h"
#include "slippymaplayer.h"
#include "slippymaplayerobject.h"

#include <QDebug>
#include <QFile>

SlippyMapLayerManager::SlippyMapLayerManager(QObject *parent) : QAbstractItemModel(parent)
{
    m_hiddenFont.setItalic(true);
    m_activeFont.setBold(true);
}

QModelIndex SlippyMapLayerManager::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent)) {
        return QModelIndex();
    }

    if (!parent.isValid()) {
        if (m_layers.count() > row) {
            return createIndex(row, column, m_layers.at(row));
        }
        else {
            return QModelIndex();
        }
    }
    else {
        SlippyMapLayer *layer = static_cast<SlippyMapLayer*>(parent.internalPointer());

        if (m_layers.contains(layer)) {
            return createIndex(row, column, layer->objects().at(row));
        }
        else {
            return QModelIndex();
        }
    }
}

QModelIndex SlippyMapLayerManager::parent(const QModelIndex &index) const
{
    if (!index.isValid()) {
        return QModelIndex();
    }

    SlippyMapLayer *layer = static_cast<SlippyMapLayer*>(index.internalPointer());
    if (m_layers.contains(layer)) {
        return QModelIndex();
    }

    SlippyMapLayerObject *obj = static_cast<SlippyMapLayerObject*>(index.internalPointer());

    for (SlippyMapLayer *layer : m_layers) {
        if (layer->objects().contains(obj)) {
            return createIndex(layer->objects().indexOf(obj), 0, layer);
        }
    }

    return QModelIndex();
}

int SlippyMapLayerManager::rowCount(const QModelIndex &parent) const
{
    if (parent.column() > 0) {
        return 0;
    }

    if (!parent.isValid()) {
        return m_layers.count();
    }

    SlippyMapLayer *layer = static_cast<SlippyMapLayer*>(parent.internalPointer());
    if (layer != nullptr && m_layers.contains(layer)) {
        return layer->objects().length();
    }

    return 0;
}

int SlippyMapLayerManager::columnCount(const QModelIndex &parent) const
{
    (void)parent;
    return 1;
}

QVariant SlippyMapLayerManager::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    SlippyMapLayer *layer = static_cast<SlippyMapLayer*>(index.internalPointer());
    if (layer != nullptr && index.column() == 0 && m_layers.contains(layer)) {
        switch (role) {
        case Qt::DisplayRole:
            return layer->name();
        case Qt::FontRole:
            if (!layer->isVisible()) {
                return m_hiddenFont;
            }
            if (m_activeLayer == layer) {
                return m_activeFont;
            }
        default:
            return QVariant();
        }
    }

    SlippyMapLayerObject *obj = static_cast<SlippyMapLayerObject*>(index.internalPointer());

    for (SlippyMapLayer *layer : m_layers) {
        if (layer != nullptr && layer->objects().contains(obj)) {
            switch (role) {
            case Qt::DisplayRole:
                switch (index.column()) {
                case 0:
                    return obj->name();
                default:
                    return QVariant();
                }
            case Qt::FontRole:
                if (!obj->isVisible()) {
                    return m_hiddenFont;
                }
                return QVariant();
            case Qt::ForegroundRole:
                if (!obj->isVisible()) {
                    return QColor(Qt::lightGray);
                }
                return QVariant();
            }
        }
    }

    return QVariant();
}

QVariant SlippyMapLayerManager::headerData(int section, Qt::Orientation orientation, int role) const
{
    (void)orientation;

    if (role != Qt::DisplayRole) {
        return QVariant();
    }

    switch (section) {
    case 0:
        return QString("Object Name");
    default:
        return QVariant();
    }
}

void SlippyMapLayerManager::addLayer(SlippyMapLayer *layer)
{
    Q_CHECK_PTR(layer);

    int first = m_layers.count();
    int last = first + 1;
    beginInsertRows(QModelIndex(), first, last);
    m_layers.append(layer);
    endInsertRows();
}

void SlippyMapLayerManager::addLayerObject(SlippyMapLayer *layer, SlippyMapLayerObject *object)
{
    Q_CHECK_PTR(layer);
    Q_CHECK_PTR(object);
    Q_ASSERT(m_layers.contains(layer));

    int first = layer->objects().count();
    int last = first + 1;
    beginInsertRows(createIndex(m_layers.indexOf(layer), 0, layer), first, last);
    layer->addObject(object);
    endInsertRows();
}

void SlippyMapLayerManager::addLayerObject(SlippyMapLayerObject *object)
{
    Q_CHECK_PTR(object);
    Q_ASSERT(m_activeLayer != nullptr);

    addLayerObject(m_activeLayer, object);
}

void SlippyMapLayerManager::takeLayer(SlippyMapLayer *layer)
{
    Q_CHECK_PTR(layer);
    Q_ASSERT(m_layers.contains(layer));

    m_layers.removeOne(layer);
}

void SlippyMapLayerManager::setActiveLayer(SlippyMapLayer *layer)
{
    Q_CHECK_PTR(layer);
    Q_ASSERT(m_layers.contains(layer));

    m_activeLayer = layer;
}

void SlippyMapLayerManager::setDefaultLayer(SlippyMapLayer *layer)
{
    Q_CHECK_PTR(layer);
    Q_ASSERT(m_layers.contains(layer));

    m_defaultLayer = layer;
}

void SlippyMapLayerManager::saveToFile(QString fileName)
{
    QFile outfile(fileName);

    outfile.open(QIODevice::WriteOnly);

    QDataStream out(&outfile);
    out.setVersion(QDataStream::Qt_5_12);
    out << (quint32)0xA0B0C0D0;
    out << (quint16)m_layers.count();

    for (SlippyMapLayer *layer : m_layers) {
        out << layer->name();
        out << layer->description();
        out << layer->objects().count();

        for (SlippyMapLayerObject *object : layer->objects()) {
            out << object->name();

        }
    }
}

bool SlippyMapLayerManager::contains(SlippyMapLayerObject *object)
{
    for (SlippyMapLayer *layer : m_layers) {
        if (layer->contains(object)) {
            return true;
        }
    }
    return false;
}

QList<SlippyMapLayer *> SlippyMapLayerManager::layers()
{
    return m_layers;
}

SlippyMapLayer *SlippyMapLayerManager::activeLayer()
{
    return m_activeLayer;
}

SlippyMapLayer *SlippyMapLayerManager::defaultLayer()
{
    return m_defaultLayer;
}

QList<SlippyMapLayerObject *> SlippyMapLayerManager::objectsAtPoint(QPointF point, int zoomLevel)
{
    QList<SlippyMapLayerObject *> objects;

    for (SlippyMapLayer *layer : m_layers) {
        for (SlippyMapLayerObject *object : layer->objects()) {
            if (object->contains(point, zoomLevel)) {
                objects.append(object);
            }
        }
    }
    return objects;
}
