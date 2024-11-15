//
// Created by robin on 11/1/2024.
//

#include "SlippyMapLayerTrackStylePropertyPage.h"
#include "SlippyMapLayerTrack.h"

#include <QCheckBox>
#include <QSpinBox>
#include <QFormLayout>
#include <QVBoxLayout>


SlippyMapLayerTrackStylePropertyPage::SlippyMapLayerTrackStylePropertyPage(SlippyMapLayerObject::Ptr object) :
    SlippyMapLayerObjectPropertyPage(object)
{
}

QString SlippyMapLayerTrackStylePropertyPage::tabTitle()
{
    return tr("Style");
}

void SlippyMapLayerTrackStylePropertyPage::save()
{
    m_track->setLineColor(m_lineColorSelector->color());
    m_track->setLineWidth(m_lineWidth->value());
    m_track->setStrokeColor(m_lineStrokeColorSelector->color());
    m_track->setStrokeWidth(m_lineStrokeWidth->value());
    m_track->setWaypointColor(m_waypointColorSelector->color());
    m_track->setWaypointRadius(m_waypointRadius->value());
}

void SlippyMapLayerTrackStylePropertyPage::setupUi()
{
    m_track = dynamic_cast<SlippyMapLayerTrack*>(m_object.get());
    m_lineColorSelector = new ColorSelector();
    m_lineStrokeColorSelector = new ColorSelector();
    m_waypointColorSelector = new ColorSelector();
    m_waypointRadius = new QSpinBox();
    m_lineWidth = new QSpinBox();
    m_lineStrokeWidth = new QSpinBox();
    m_waypointsVisible = new QCheckBox();

    m_lineColorSelector->setColor(m_track->lineColor());
    m_lineStrokeColorSelector->setColor(m_track->strokeColor());
    m_waypointColorSelector->setColor(m_track->waypointColor());
    m_waypointRadius->setValue(m_track->waypointRadius());
    m_lineWidth->setValue(m_track->lineWidth());
    m_lineStrokeWidth->setValue(m_track->strokeWidth());
    m_waypointsVisible->setChecked(m_track->waypointsVisible());

    auto *formLayout = new QFormLayout();
    formLayout->addRow(tr("Line Color"), m_lineColorSelector);
    formLayout->addRow(tr("Line Width"), m_lineWidth);
    formLayout->addRow(tr("Stroke Color"), m_lineStrokeColorSelector);
    formLayout->addRow(tr("Stroke Width"), m_lineStrokeWidth);
    formLayout->addRow(tr("Waypoint Color"), m_waypointColorSelector);
    formLayout->addRow(tr("Waypoint Radius"), m_waypointRadius);
    formLayout->addRow(m_waypointsVisible);
    formLayout->setSpacing(10);

    auto *vLayout = new QVBoxLayout();
    vLayout->addLayout(formLayout);
    vLayout->addStretch();
    vLayout->setSpacing(0);
    vLayout->setContentsMargins(10, 10, 10, 10);

    setLayout(vLayout);
}

void SlippyMapLayerTrackStylePropertyPage::updateUi()
{
    m_lineColorSelector->setColor(m_track->lineColor());
    m_lineStrokeColorSelector->setColor(m_track->strokeColor());
    m_waypointColorSelector->setColor(m_track->waypointColor());
    m_waypointRadius->setValue(m_track->waypointRadius());
    m_lineWidth->setValue(m_track->lineWidth());
    m_lineStrokeWidth->setValue(m_track->strokeWidth());
    m_waypointsVisible->setChecked(m_track->waypointsVisible());
}
