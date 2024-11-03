//
// Created by Robin on 11/1/2024.
//

#ifndef OSMEXPLORER_SLIPPYMAPGPSMARKER_H
#define OSMEXPLORER_SLIPPYMAPGPSMARKER_H

#include <SlippyMap/SlippyMapWidgetMarker.h>
#include <QDateTime>
#include "nmeaseriallocationdataprovider.h"

using namespace SlippyMap;


class SlippyMapGpsMarker : public SlippyMapWidgetMarker {
    Q_OBJECT
public:
    explicit SlippyMapGpsMarker(const QPointF& position);
    void draw(QPainter *painter, const QTransform &transform, ObjectState state) override;
    QList<SlippyMapLayerObjectPropertyPage*> propertyPages() const;
    void setGpsTime(const QDateTime& time);
    void setSatellites(const QList<NmeaSerialLocationDataProvider::SatelliteStatus>& satellites);
    const QList<NmeaSerialLocationDataProvider::SatelliteStatus>& satellites() const;

private:
    QDateTime m_gpsTime;
    QBrush m_whiteBrush;
    QList<NmeaSerialLocationDataProvider::SatelliteStatus> m_satellites;
    int m_numSats;
};


#endif //OSMEXPLORER_SLIPPYMAPGPSMARKER_H
