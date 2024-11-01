//
// Created by robin on 11/1/2024.
//

#include "SlippyMapLayerTrackStylePropertyPage.h"
#include "SlippyMapLayerTrack.h"

#include <QSpinBox>
#include <QFormLayout>
#include <QVBoxLayout>


SlippyMapLayerTrackStylePropertyPage::SlippyMapLayerTrackStylePropertyPage(SlippyMapLayerObject *object) :
    SlippyMapLayerObjectPropertyPage(object)
{
}

QString SlippyMapLayerTrackStylePropertyPage::tabTitle()
{
    return tr("Style");
}

void SlippyMapLayerTrackStylePropertyPage::save()
{
    m_track->setTrackLineColor(m_lineColorSelector->color());
    m_track->setTrackLineWidth(m_lineWidth->value());
    m_track->setTrackLineStrokeColor(m_lineStrokeColorSelector->color());
    m_track->setTrackLineStrokeWidth(m_lineStrokeWidth->value());
    m_track->setWaypointColor(m_waypointColorSelector->color());
    m_track->setWaypointRadius(m_waypointRadius->value());
}

void SlippyMapLayerTrackStylePropertyPage::setupUi()
{
    m_track = dynamic_cast<SlippyMapLayerTrack*>(m_object);
    m_lineColorSelector = new ColorSelector();
    m_lineStrokeColorSelector = new ColorSelector();
    m_waypointColorSelector = new ColorSelector();
    m_waypointRadius = new QSpinBox();
    m_lineWidth = new QSpinBox();
    m_lineStrokeWidth = new QSpinBox();

    m_lineColorSelector->setColor(m_track->trackLineColor());
    m_lineStrokeColorSelector->setColor(m_track->trackLineStrokeColor());
    m_waypointColorSelector->setColor(m_track->waypointColor());
    m_waypointRadius->setValue(m_track->waypointRadius());
    m_lineWidth->setValue(m_track->trackLineWidth());
    m_lineStrokeWidth->setValue(m_track->trackLineStrokeWidth());

    auto *formLayout = new QFormLayout();
    formLayout->addRow(tr("Line Color"), m_lineColorSelector);
    formLayout->addRow(tr("Line Width"), m_lineWidth);
    formLayout->addRow(tr("Stroke Color"), m_lineStrokeColorSelector);
    formLayout->addRow(tr("Stroke Width"), m_lineStrokeWidth);
    formLayout->addRow(tr("Waypoint Color"), m_waypointColorSelector);
    formLayout->addRow(tr("Waypoint Radius"), m_waypointRadius);

    auto *vLayout = new QVBoxLayout();
    vLayout->addLayout(formLayout);
    vLayout->addStretch();

    setLayout(vLayout);
}

void SlippyMapLayerTrackStylePropertyPage::updateUi()
{
    m_lineColorSelector->setColor(m_track->trackLineColor());
    m_lineStrokeColorSelector->setColor(m_track->trackLineStrokeColor());
    m_waypointColorSelector->setColor(m_track->waypointColor());
    m_waypointRadius->setValue(m_track->waypointRadius());
    m_lineWidth->setValue(m_track->trackLineWidth());
    m_lineStrokeWidth->setValue(m_track->trackLineStrokeWidth());
}
