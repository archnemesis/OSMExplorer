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


signals:
    void markerCreated(SlippyMapWidgetMarker *marker);
    void markerRemoved(SlippyMapWidgetMarker *marker);

public slots:
};

#endif // SLIPPYMAPWIDGETMARKERPROVIDER_H
