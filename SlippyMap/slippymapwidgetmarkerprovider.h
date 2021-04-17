#ifndef SLIPPYMAPWIDGETMARKERPROVIDER_H
#define SLIPPYMAPWIDGETMARKERPROVIDER_H

#include "slippymap_global.h"

#include <QObject>
#include "slippymapwidgetmarker.h"

class SLIPPYMAPSHARED_EXPORT SlippyMapWidgetMarkerProvider : public QObject
{
    Q_OBJECT

public:
    explicit SlippyMapWidgetMarkerProvider(QObject *parent = nullptr);
    virtual void start();
    virtual void stop();

signals:
    void markerCreated(SlippyMapWidgetMarker *marker);
    void markerRemoved(SlippyMapWidgetMarker *marker);

};

#endif // SLIPPYMAPWIDGETMARKERPROVIDER_H
