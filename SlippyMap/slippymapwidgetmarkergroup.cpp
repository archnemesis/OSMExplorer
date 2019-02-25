#include "slippymapwidgetmarkergroup.h"

SlippyMapWidgetMarkerGroup::SlippyMapWidgetMarkerGroup(QString label, QObject *parent) :
    QObject(parent),
    m_label(label)
{

}

void SlippyMapWidgetMarkerGroup::addMarker(SlippyMapWidgetMarker *marker)
{
    m_markers.append(marker);
    emit markerAdded(marker);
}

void SlippyMapWidgetMarkerGroup::removeMarker(SlippyMapWidgetMarker *marker)
{
    m_markers.removeOne(marker);
    emit markerRemoved(marker);
}

QString SlippyMapWidgetMarkerGroup::label()
{
    return m_label;
}

QList<SlippyMapWidgetMarker *> SlippyMapWidgetMarkerGroup::markers()
{
    return m_markers;
}
