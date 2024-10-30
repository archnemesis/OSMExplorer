//
// Created by robin on 10/29/24.
//

#include "WeatherStationMarker.h"

#include <utility>

WeatherStationMarker::WeatherStationMarker(QString stationId, QWidget* parent) :
    SlippyMap::SlippyMapWidgetMarker(),
    m_stationId(std::move(stationId))
{

}

const QString& WeatherStationMarker::stationId() const {
    return m_stationId;
}
