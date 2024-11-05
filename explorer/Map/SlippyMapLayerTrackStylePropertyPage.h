//
// Created by robin on 11/1/2024.
//

#ifndef SLIPPYMAPLAYERTRACKSTYLEPROPERTYPAGE_H
#define SLIPPYMAPLAYERTRACKSTYLEPROPERTYPAGE_H

#include <SlippyMap/SlippyMapLayerObjectPropertyPage.h>
#include <SlippyMap/SlippyMapLayerObject.h>
#include <QtColorWidgets/ColorSelector>

using namespace color_widgets;
using namespace SlippyMap;

class QSpinBox;
class SlippyMapLayerTrack;

class SlippyMapLayerTrackStylePropertyPage : public SlippyMapLayerObjectPropertyPage {
    Q_OBJECT
public:
    explicit SlippyMapLayerTrackStylePropertyPage(SlippyMapLayerObject::Ptr object);
    QString tabTitle() override;
    void save() override;
    void setupUi() override;
    void updateUi() override;
private:
    ColorSelector *m_lineColorSelector;
    ColorSelector *m_lineStrokeColorSelector;
    ColorSelector *m_waypointColorSelector;
    QSpinBox *m_lineWidth;
    QSpinBox *m_lineStrokeWidth;
    QSpinBox *m_waypointRadius;
    SlippyMapLayerTrack *m_track;
};



#endif //SLIPPYMAPLAYERTRACKSTYLEPROPERTYPAGE_H
