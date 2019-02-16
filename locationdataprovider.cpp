#include "locationdataprovider.h"

LocationDataProvider::LocationDataProvider(QObject *parent) : QObject(parent)
{

}

void LocationDataProvider::setUpdateInterval(int interval)
{
    m_updateInterval = interval;
}

int LocationDataProvider::updateInterval()
{
    return m_updateInterval;
}
