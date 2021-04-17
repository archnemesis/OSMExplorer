#ifndef SLIPPYMAPLAYERTRACKPROPERTYPAGE_H
#define SLIPPYMAPLAYERTRACKPROPERTYPAGE_H

#include <QWidget>
#include "slippymaplayerobjectpropertypage.h"

class SlippyMapLayerTrack;

namespace Ui {
class SlippyMapLayerTrackPropertyPageForm;
}

class SlippyMapLayerTrackPropertyPageForm : public SlippyMapLayerObjectPropertyPage
{
    Q_OBJECT

public:
    explicit SlippyMapLayerTrackPropertyPageForm(QWidget *parent = nullptr);
    explicit SlippyMapLayerTrackPropertyPageForm(SlippyMapLayerTrack *track, QWidget *parent = nullptr);
    ~SlippyMapLayerTrackPropertyPageForm();
    QString tabTitle();

private:
    Ui::SlippyMapLayerTrackPropertyPageForm *ui;
    SlippyMapLayerTrack *m_track = nullptr;
};

#endif // SLIPPYMAPLAYERTRACKPROPERTYPAGE_H
