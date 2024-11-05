//
// Created by Robin on 11/1/2024.
//

#ifndef OSMEXPLORER_SLIPPYMAPGPSMARKERPROPERTYPAGE_H
#define OSMEXPLORER_SLIPPYMAPGPSMARKERPROPERTYPAGE_H

#include <SlippyMap/SlippyMapLayerObjectPropertyPage.h>

class QLabel;
class QProgressBar;

using namespace SlippyMap;

class SlippyMapGpsMarkerPropertyPage : public SlippyMapLayerObjectPropertyPage
{
    Q_OBJECT
public:
    explicit SlippyMapGpsMarkerPropertyPage(const SlippyMapLayerObject::Ptr& object);
    virtual QString tabTitle();
    void setupUi() override;
    void save() override;
    void updateUi() override;

protected:
    QLabel *m_numSatsLabel;
    QLabel *m_gpsTimeLabel;
    QList<QProgressBar*> m_satellites;
};


#endif //OSMEXPLORER_SLIPPYMAPGPSMARKERPROPERTYPAGE_H
