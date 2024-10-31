#include "SlippyMapLayerTrackPropertyPage.h"
#include "SlippyMapLayerTrack.h"

#include <QDebug>
#include <QListWidget>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QHBoxLayout>

#include "gpx/gpxtracksegment.h"
#include "gpx/gpxwaypoint.h"

SlippyMapLayerTrackPropertyPage::SlippyMapLayerTrackPropertyPage(SlippyMapLayerObject *object):
    SlippyMapLayerObjectPropertyPage(object) {
    m_track = qobject_cast<SlippyMapLayerTrack*>(object);
    setupUi();
    setWindowTitle(m_track->label());
}

SlippyMapLayerTrackPropertyPage::~SlippyMapLayerTrackPropertyPage()
{

}

QString SlippyMapLayerTrackPropertyPage::tabTitle()
{
    return m_track->label();
}

void SlippyMapLayerTrackPropertyPage::setupUi() {
    auto *tableWidget = new QTableWidget();
    tableWidget->setColumnCount(4);

    int i = 0;
    int r = 0;
    for (const auto& segment : m_track->track().segments()) {
        r += segment.points().length();
        tableWidget->setRowCount(r);

        qDebug() << "Processing segment";

        for (const auto& waypoint : segment.points()) {
            auto *id_item = new QTableWidgetItem(tr("%1").arg(i));
            auto *timestamp_item = new QTableWidgetItem(tr("%1").arg(waypoint.time().toString(Qt::ISODate)));
            auto *latitude_item = new QTableWidgetItem(tr("%1").arg(waypoint.latitude()));
            auto *longitude_item = new QTableWidgetItem(tr("%1").arg(waypoint.longitude()));

            tableWidget->setItem(i, 0, id_item);
            tableWidget->setItem(i, 1, timestamp_item);
            tableWidget->setItem(i, 2, latitude_item);
            tableWidget->setItem(i, 3, longitude_item);

            i++;
        }
    }

    auto *hLayout = new QHBoxLayout();
    hLayout->addWidget(tableWidget);
    setLayout(hLayout);
}