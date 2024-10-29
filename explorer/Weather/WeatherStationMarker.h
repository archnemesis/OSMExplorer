//
// Created by robin on 10/29/24.
//

#ifndef WEATHERSTATIONMARKER_H
#define WEATHERSTATIONMARKER_H

#include <SlippyMap/SlippyMapWidgetMarker.h>


class WeatherStationMarker : public SlippyMap::SlippyMapWidgetMarker {
    Q_OBJECT
public:
    explicit WeatherStationMarker(QString stationId, QWidget* parent = nullptr);
    const QString& stationId() const;

private:
    QString m_stationId;
};



#endif //WEATHERSTATIONMARKER_H
