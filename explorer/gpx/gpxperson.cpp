#include "gpxperson.h"
#include "gpxtracklink.h"

GPXPerson::GPXPerson(QObject *parent) : QObject(parent)
{

}

GPXPerson::GPXPerson(const GPXPerson& other) {
    setName(other.name());
    setEmail(other.email());
    setLink(other.link());
}

GPXPerson& GPXPerson::operator=(const GPXPerson& other) {
    setName(other.name());
    setEmail(other.email());
    setLink(other.link());
    return *this;
}

GPXPerson::~GPXPerson()
{
}

void GPXPerson::setName(const QString &name)
{
    m_name = name;
}

void GPXPerson::setEmail(const QString &email)
{
    m_email = email;
}

void GPXPerson::setLink(const GPXTrackLink& link)
{
    m_link = link;
}

const QString& GPXPerson::name() const
{
    return m_name;
}

const QString& GPXPerson::email() const
{
    return m_email;
}

const GPXTrackLink& GPXPerson::link() const
{
    return m_link;
}
