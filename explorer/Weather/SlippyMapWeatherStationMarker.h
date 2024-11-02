//
// Created by Robin on 11/1/2024.
//

#ifndef OSMEXPLORER_SLIPPYMAPWEATHERSTATIONMARKER_H
#define OSMEXPLORER_SLIPPYMAPWEATHERSTATIONMARKER_H

#include <SlippyMap/SlippyMapWidgetMarker.h>

using namespace SlippyMap;


class SlippyMapWeatherStationMarker : public SlippyMapWidgetMarker
{
    Q_OBJECT
public:
    explicit SlippyMapWeatherStationMarker(SlippyMapLayerObject *object);
};


#endif //OSMEXPLORER_SLIPPYMAPWEATHERSTATIONMARKER_H
