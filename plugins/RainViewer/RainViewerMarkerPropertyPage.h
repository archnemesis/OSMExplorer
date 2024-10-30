//
// Created by Robin on 10/29/2024.
//

#ifndef OSMEXPLORER_RAINVIEWERMARKERPROPERTYPAGE_H
#define OSMEXPLORER_RAINVIEWERMARKERPROPERTYPAGE_H

#include "rainviewer_global.h"
#include "PropertyPage/SlippyMapLayerObjectPropertyPage.h"

#include <SlippyMap/SlippyMapLayerObject.h>

class QTabWidget;
class QLabel;

using namespace SlippyMap;

class RainViewerMarkerPropertyPage : public  SlippyMapLayerObjectPropertyPage {
    Q_OBJECT
public:
    explicit RainViewerMarkerPropertyPage(SlippyMapLayerObject *object);

private:
    QTabWidget *m_tabWidget;
    QLabel *m_temperatureLabel;
    QLabel *m_relativeHumidityLabel;
    QLabel *m_windSpeed;
    QLabel *m_windDirection;
    QLabel *m_dewPoint;
    QLabel *m_timestamp;
};


#endif //OSMEXPLORER_RAINVIEWERMARKERPROPERTYPAGE_H
