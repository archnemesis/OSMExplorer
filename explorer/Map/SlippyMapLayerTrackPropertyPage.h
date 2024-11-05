#ifndef SLIPPYMAPLAYERTRACKPROPERTYPAGE_H
#define SLIPPYMAPLAYERTRACKPROPERTYPAGE_H

#include <QWidget>
#include <SlippyMap/SlippyMapLayerObjectPropertyPage.h>

#include "SlippyMapLayerTrack.h"

class SlippyMapLayerTrackPropertyPage : public SlippyMapLayerObjectPropertyPage
{
    Q_OBJECT

public:
    explicit SlippyMapLayerTrackPropertyPage(const SlippyMapLayerObject::Ptr& object);
    ~SlippyMapLayerTrackPropertyPage() override;
    QString tabTitle() override;
    void save() override;
    void setupUi() override;

public slots:
    void updateUi() override;

protected:
    SlippyMapLayerTrack *m_track;
};

#endif // SLIPPYMAPLAYERTRACKPROPERTYPAGE_H
