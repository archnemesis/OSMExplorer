#include "slippymapwidgetlayer.h"

#include <QByteArray>
#include <QCryptographicHash>

SlippyMapWidgetLayer::SlippyMapWidgetLayer(const QString& tileUrl, QObject *parent) :
    QObject(parent)
{
    m_tileUrl = tileUrl;
    m_visible = true;
}

SlippyMapWidgetLayer::SlippyMapWidgetLayer(QObject *parent) :
    QObject(parent)
{

}

void SlippyMapWidgetLayer::setTileUrl(QString tileUrl)
{
    m_tileUrl = tileUrl;
    emit updated();
}

void SlippyMapWidgetLayer::setName(QString name)
{
    m_name = name;
    emit updated();
}

void SlippyMapWidgetLayer::setDescription(QString description)
{
    m_description = description;
    emit updated();
}

void SlippyMapWidgetLayer::setZOrder(int zOrder)
{
    m_zOrder = zOrder;
    emit updated();
}

void SlippyMapWidgetLayer::setVisible(bool visible)
{
    m_visible = visible;
    emit updated();
}

QString SlippyMapWidgetLayer::name()
{
    return m_name;
}

QString SlippyMapWidgetLayer::description()
{
    return m_description;
}

QString SlippyMapWidgetLayer::tileUrl()
{
    return m_tileUrl;
}

QString SlippyMapWidgetLayer::tileUrlHash()
{
    QByteArray hash = QCryptographicHash::hash(
                m_tileUrl.toLocal8Bit(),
                QCryptographicHash::Md5).toHex();
    return QString::fromLocal8Bit(hash);
}

int SlippyMapWidgetLayer::zOrder()
{
    return m_zOrder;
}

bool SlippyMapWidgetLayer::isVisible()
{
    return m_visible;
}
