#include "slippymapwidgetmarker.h"

SlippyMapWidgetMarker::SlippyMapWidgetMarker(QPointF position, QString label, QObject *parent) :
    QObject(parent),
    m_position(position),
    m_label(label)
{

}

void SlippyMapWidgetMarker::setPosition(QPointF position)
{
    bool changed = false;
    if (m_position != position) changed = true;
    m_position = position;
    if (changed) emit positionChanged(m_position);
}

void SlippyMapWidgetMarker::setLabel(QString label)
{
    bool changed = false;
    if (m_label != label) changed = true;
    m_label = label;
    if (changed) emit labelTextChanged(m_label);
}

void SlippyMapWidgetMarker::setColor(QColor color)
{
    m_color = color;
}

double SlippyMapWidgetMarker::latitude()
{
    return m_position.y();
}

double SlippyMapWidgetMarker::longitude()
{
    return m_position.x();
}

QString SlippyMapWidgetMarker::label()
{
    return m_label;
}

QColor SlippyMapWidgetMarker::color()
{
    return m_color;
}
