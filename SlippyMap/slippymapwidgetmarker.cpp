#include "slippymapwidgetmarker.h"

#include <QPainter>
#include <QPoint>
#include <QPointF>
#include <QBrush>
#include <QPen>
#include <QColor>
#include <QFont>
#include <QFontMetrics>
#include <QPalette>
#include <QGuiApplication>

SlippyMapWidgetMarker::SlippyMapWidgetMarker(QPointF position, QObject *parent) :
    QObject(parent),
    m_position(position)
{
    m_label = QString("%1 %2")
            .arg(position.x())
            .arg(position.y());
    initColors();
}

SlippyMapWidgetMarker::SlippyMapWidgetMarker(QPointF position, QString label, QObject *parent) :
    QObject(parent),
    m_position(position),
    m_label(label)
{
    initColors();
}

void SlippyMapWidgetMarker::setPosition(QPointF position)
{
    bool chg = false;
    if (m_position != position) chg = true;
    m_position = position;
    if (chg) {
        emit positionChanged(m_position);
        emit changed();
    }
}

void SlippyMapWidgetMarker::setLabel(QString label)
{
    bool chg = false;
    if (m_label != label) chg = true;
    m_label = label;
    if (chg) {
        emit labelTextChanged(m_label);
        emit changed();
    }
}

void SlippyMapWidgetMarker::setInformation(QString information)
{
    bool chg = false;
    if (m_information != information) chg = true;
    m_information = information;
    if (chg) {
        emit informationTextChanged(m_information);
        emit changed();
    }
}

void SlippyMapWidgetMarker::setMarkerColor(QColor color)
{
    m_markerColor = color;
    m_dotBrush.setColor(color);
    m_activeDotPen.setColor(color);
}

void SlippyMapWidgetMarker::setMovable(bool movable)
{
    m_movable = movable;
}

void SlippyMapWidgetMarker::setEditable(bool editable)
{
    m_editable = editable;
}

void SlippyMapWidgetMarker::setMetadata(QHash<QString, QVariant> metadata)
{
    m_metadata = metadata;
}

QPointF SlippyMapWidgetMarker::position()
{
    return m_position;
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

QString SlippyMapWidgetMarker::information()
{
    return m_information;
}

QColor SlippyMapWidgetMarker::color()
{
    return m_markerColor;
}

QHash<QString, QVariant> SlippyMapWidgetMarker::metadata()
{
    return m_metadata;
}

bool SlippyMapWidgetMarker::isMovable()
{
    return m_movable;
}

bool SlippyMapWidgetMarker::isEditable()
{
    return m_editable;
}

void SlippyMapWidgetMarker::drawMarker(QPainter *painter, QPoint pos, MarkerState state)
{
    qint32 rad = 5;

    if (state == ActiveState) {
        painter->setBrush(m_activeDotBrush);
        painter->setPen(m_activeDotPen);
        painter->drawEllipse(pos, rad + 2, rad + 2);
    }

    painter->setBrush(m_dotBrush);
    painter->setPen(m_dotPen);
    painter->drawEllipse(pos, rad, rad);

    QFontMetrics metrics(painter->font());
    if (label().length() > 0) {
        qint32 label_w = metrics.width(label());
        qint32 label_h = metrics.height();
        qint32 label_x = pos.x() - (label_w / 2);
        qint32 label_y = pos.y() - (label_h + 15);
        painter->setBrush(m_labelBrush);
        painter->setPen(m_labelPen);
        painter->drawRoundRect(
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
                    label());
    }
}

void SlippyMapWidgetMarker::initColors()
{
    QPalette systemPalette = QGuiApplication::palette();

    m_dotBrush.setStyle(Qt::SolidPattern);
    m_dotBrush.setColor(systemPalette.highlight().color());
    m_dotPen.setStyle(Qt::NoPen);

    m_activeDotBrush.setStyle(Qt::NoBrush);
    m_activeDotPen.setColor(systemPalette.highlight().color());
    m_activeDotPen.setWidth(2);

    m_labelBrush.setStyle(Qt::SolidPattern);
    m_labelBrush.setColor(systemPalette.background().color());
    m_labelPen.setStyle(Qt::SolidLine);
    m_labelPen.setColor(systemPalette.dark().color());

    m_labelTextBrush.setStyle(Qt::NoBrush);
    m_labelTextPen.setStyle(Qt::SolidLine);
    m_labelTextPen.setColor(systemPalette.text().color());
}