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
    QLabel *m_fixTypeLabel;
    QLabel *m_numSatsLabel;
    QLabel *m_gpsTimeLabel;
    QLabel *m_gpsLatitudeLabel;
    QLabel *m_gpsLongitudeLabel;
    QLabel *m_gpsAltitudeLabel;
    QLabel *m_hdopLabel;
    QLabel *m_vdopLabel;
    QLabel *m_pdopLabel;
    QList<QProgressBar*> m_satelliteProgressBars;
    QList<QLabel*> m_satellitePrnLabels;
};


#endif //OSMEXPLORER_SLIPPYMAPGPSMARKERPROPERTYPAGE_H
