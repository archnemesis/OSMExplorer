#ifndef SLIPPYMAPLAYERTRACKPROPERTYPAGE_H
#define SLIPPYMAPLAYERTRACKPROPERTYPAGE_H

#include <QWidget>
#include <SlippyMap/SlippyMapLayerObjectPropertyPage.h>

#include "SlippyMapLayerTrack.h"

class SlippyMapLayerTrackPropertyPage : public SlippyMapLayerObjectPropertyPage
{
    Q_OBJECT

public:
    explicit SlippyMapLayerTrackPropertyPage(SlippyMapLayerObject *object);
    ~SlippyMapLayerTrackPropertyPage() override;
    QString tabTitle() override;

protected:
    void setupUi();
    SlippyMapLayerTrack *m_track;
};

#endif // SLIPPYMAPLAYERTRACKPROPERTYPAGE_H
