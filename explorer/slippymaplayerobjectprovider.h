#ifndef SLIPPYMAPLAYEROBJECTPROVIDER_H
#define SLIPPYMAPLAYEROBJECTPROVIDER_H

#include <QObject>
#include <SlippyMap/SlippyMapLayerObject.h>

using namespace SlippyMap;

class SlippyMapLayerObjectProvider : public QObject
{
    Q_OBJECT

public:
    explicit SlippyMapLayerObjectProvider(QObject *parent = nullptr);
    virtual void start();
    virtual void stop();

signals:
    void objectCreated(SlippyMapLayerObject *object);
    void objectRemoved(SlippyMapLayerObject *object);

};

#endif // SLIPPYMAPLAYEROBJECTPROVIDER_H
