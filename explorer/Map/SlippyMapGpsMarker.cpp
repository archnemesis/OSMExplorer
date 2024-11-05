//
// Created by Robin on 11/1/2024.
//

#include "SlippyMapGpsMarker.h"
#include "SlippyMapGpsMarkerPropertyPage.h"
#include <SlippyMap/SlippyMap.h>

SlippyMapGpsMarker::SlippyMapGpsMarker(const QPointF& position) :
    SlippyMapWidgetMarker(position)
{
    m_whiteBrush.setColor(Qt::white);
    m_whiteBrush.setStyle(Qt::SolidPattern);
    m_activeDotPen.setWidth(4);
}

void SlippyMapGpsMarker::draw(QPainter *painter, const QTransform &transform, SlippyMapLayerObject::ObjectState state)
{
    QPointF pos = transform.map(m_position);

    painter->setBrush(m_whiteBrush);
    painter->setPen(Qt::NoPen);
    painter->drawEllipse(pos, m_radius + 5, m_radius + 5);

    if (state == ObjectState::SelectedState) {
        painter->setBrush(m_activeDotBrush);
        painter->setPen(m_activeDotPen);
        painter->drawEllipse(pos, m_radius + 5, m_radius + 5);
    }

    painter->setBrush(m_dotBrush);
    painter->setPen(m_dotPen);
    painter->drawEllipse(pos, m_radius, m_radius);

    QFontMetrics metrics(painter->font());

    QString label = QString("%1: %2")
            .arg(m_label)
            .arg(latLonToString(m_position));

    qint32 label_w = metrics.horizontalAdvance(label);
    qint32 label_h = metrics.height();
    qint32 label_x = static_cast<qint32>(pos.x()) - (label_w / 2);
    qint32 label_y = static_cast<qint32>(pos.y()) - (label_h + m_radius + 15);
    painter->setBrush(m_labelBrush);
    painter->setPen(m_labelPen);
    painter->drawRoundedRect(
            label_x - 5,
            label_y - 5,
            label_w + 10,
            label_h + 10,
            5, 5);
    painter->setBrush(m_labelTextBrush);
    painter->setPen(m_labelTextPen);
    painter->drawText(
            label_x,
            label_y + (label_h / 2) + 5,
            label);
}

QList<SlippyMapLayerObjectPropertyPage *> SlippyMapGpsMarker::propertyPages(
        SlippyMapLayerObject::Ptr object
        ) const
{
    return {new SlippyMapGpsMarkerPropertyPage(object)};
}

void SlippyMapGpsMarker::setGpsTime(const QDateTime &time)
{
    m_gpsTime = time;
    emit SlippyMapLayerObject::updated();
}

void SlippyMapGpsMarker::setSatellites(const QList<NmeaSerialLocationDataProvider::SatelliteStatus> &satellites)
{
    m_satellites = satellites;
    emit SlippyMapLayerObject::updated();
}

const QList<NmeaSerialLocationDataProvider::SatelliteStatus> &SlippyMapGpsMarker::satellites() const
{
    return m_satellites;
}
