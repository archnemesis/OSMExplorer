#ifndef SLIPPYMAPWIDGETMARKERGROUP_H
#define SLIPPYMAPWIDGETMARKERGROUP_H

#include <QObject>
#include "slippymapwidgetmarker.h"

class SlippyMapWidgetMarkerGroup : public QObject
{
    Q_OBJECT
public:
    SlippyMapWidgetMarkerGroup(QString label, QObject *parent = nullptr);
    void addMarker(SlippyMapWidgetMarker *marker);
    void removeMarker(SlippyMapWidgetMarker *marker);
    QString label();
    QList<SlippyMapWidgetMarker*> markers();
signals:
    void markerAdded(SlippyMapWidgetMarker *marker);
    void markerRemoved(SlippyMapWidgetMarker *marker);
private:
    QString m_label;
    QList<SlippyMapWidgetMarker*> m_markers;
};

#endif // SLIPPYMAPWIDGETMARKERGROUP_H
