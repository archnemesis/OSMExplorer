//
// Created by robin on 10/29/24.
//

#ifndef SLIPPYMAPWEATHERMARKER_H
#define SLIPPYMAPWEATHERMARKER_H

#include <SlippyMap/SlippyMapWidgetMarker.h>

class SlippyMapWeatherMarker : public SlippyMap::SlippyMapWidgetMarker {
    Q_OBJECT
public:
    explicit SlippyMapWeatherMarker(QWidget *parent = nullptr);
    void draw(QPainter *painter, const QTransform &transform, ObjectState state) override;
};



#endif //SLIPPYMAPWEATHERMARKER_H
