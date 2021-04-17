#include "gpxperson.h"
#include "gpxtracklink.h"

GPXPerson::GPXPerson(QObject *parent) : QObject(parent)
{

}

GPXPerson::~GPXPerson()
{
    if (m_link != nullptr) {
        delete m_link;
    }
}

void GPXPerson::setName(const QString &name)
{
    m_name = name;
}

void GPXPerson::setEmail(const QString &email)
{
    m_email = email;
}

void GPXPerson::setLink(GPXTrackLink *link)
{
    m_link = link;
}

QString GPXPerson::name() const
{
    return m_name;
}

QString GPXPerson::email() const
{
    return m_email;
}

GPXTrackLink *GPXPerson::link() const
{
    return m_link;
}
