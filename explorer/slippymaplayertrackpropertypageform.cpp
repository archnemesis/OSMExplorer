#include "slippymaplayertrackpropertypageform.h"
#include "ui_slippymaplayertrackpropertypageform.h"
#include "slippymaplayertrack.h"

SlippyMapLayerTrackPropertyPageForm::SlippyMapLayerTrackPropertyPageForm(QWidget *parent) :
    SlippyMapLayerObjectPropertyPage (parent),
    ui(new Ui::SlippyMapLayerTrackPropertyPageForm)
{
    ui->setupUi(this);
}

SlippyMapLayerTrackPropertyPageForm::SlippyMapLayerTrackPropertyPageForm(SlippyMapLayerTrack *track, QWidget *parent) :
    SlippyMapLayerTrackPropertyPageForm(parent)
{
    m_track = track;
    setWindowTitle(m_track->name());
    ui->lneName->setText(m_track->name());
    ui->pteDescription->setPlainText(m_track->description());
}

SlippyMapLayerTrackPropertyPageForm::~SlippyMapLayerTrackPropertyPageForm()
{
    delete ui;
}

QString SlippyMapLayerTrackPropertyPageForm::tabTitle()
{
    return tr("Track");
}
