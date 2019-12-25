#include "slippymaplayer.h"
#include "slippymaplayerobject.h"

SlippyMapLayer::SlippyMapLayer(QObject *parent) : QObject(parent)
{

}

void SlippyMapLayer::addObject(SlippyMapLayerObject *object)
{
    m_objects.append(object);
}

void SlippyMapLayer::takeObject(SlippyMapLayerObject *object)
{
    m_objects.removeOne(object);
}

QList<SlippyMapLayerObject *> SlippyMapLayer::objects() const
{
    return m_objects;
}

QString SlippyMapLayer::name() const
{
    return m_name;
}

QString SlippyMapLayer::description() const
{
    return m_description;
}

bool SlippyMapLayer::isVisible()
{
    return m_visible;
}

void SlippyMapLayer::setName(const QString &name)
{
    m_name = name;
}

void SlippyMapLayer::setDescription(const QString &description)
{
    m_description = description;
}

void SlippyMapLayer::setVisible(const bool visible)
{
    m_visible = visible;
}

QDataStream &operator<<(QDataStream &stream, const SlippyMapLayer *layer)
{
    stream << layer->name();
    stream << layer->description();
    stream << layer->objects().length();
    return stream;
}

QDataStream &operator>>(QDataStream &stream, SlippyMapLayer *layer)
{
    QString name;
    QString description;
    int objectCount;

    stream >> name;
    stream >> description;
    stream >> objectCount;

    layer->setName(name);
    layer->setDescription(description);

    for (int i = 0; i < objectCount; i++) {

    }

    return stream;
}
