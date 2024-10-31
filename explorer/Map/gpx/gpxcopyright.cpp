#include "gpxcopyright.h"

GPXCopyright::GPXCopyright(QObject *parent) : QObject(parent)
{

}

GPXCopyright::GPXCopyright(const GPXCopyright& other) {
    setAuthor(other.author());
    setYear(other.year());
    setLicense(other.license());
}

GPXCopyright& GPXCopyright::operator=(const GPXCopyright& other) {
    setAuthor(other.author());
    setYear(other.year());
    setLicense(other.license());
    return *this;
}

QString GPXCopyright::author() const
{
    return m_author;
}

void GPXCopyright::setAuthor(const QString &author)
{
    m_author = author;
}

QString GPXCopyright::year() const
{
    return m_year;
}

void GPXCopyright::setYear(const QString &year)
{
    m_year = year;
}

QString GPXCopyright::license() const
{
    return m_license;
}

void GPXCopyright::setLicense(const QString &license)
{
    m_license = license;
}
