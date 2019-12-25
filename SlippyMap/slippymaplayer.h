#ifndef SLIPPYMAPLAYER_H
#define SLIPPYMAPLAYER_H

#include "slippymap_global.h"

#include <QObject>
#include <QList>
#include <QDataStream>

class SlippyMapLayerObject;

class SLIPPYMAPSHARED_EXPORT SlippyMapLayer : public QObject
{
    Q_OBJECT
public:
    explicit SlippyMapLayer(QObject *parent = nullptr);

    void addObject(SlippyMapLayerObject *object);
    void takeObject(SlippyMapLayerObject *object);
    QList<SlippyMapLayerObject*> objects() const;
    QString name() const;
    QString description() const;
    bool isVisible();
    void setName(const QString &name);
    void setDescription(const QString &description);
    void setVisible(const bool visible);

signals:
    void objectAdded(SlippyMapLayerObject *shape);
    void objectRemoved(SlippyMapLayerObject *shape);

protected:
    int m_zOrder = 0;
    bool m_visible = true;
    QString m_name;
    QString m_description;
    QList<SlippyMapLayerObject*> m_objects;
};

QDataStream & operator<< (QDataStream& stream, const SlippyMapLayer* layer);
QDataStream & operator>> (QDataStream& stream, SlippyMapLayer* layer);

#endif // SLIPPYMAPLAYER_H
