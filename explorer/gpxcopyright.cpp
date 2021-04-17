#include "gpxcopyright.h"

GPXCopyright::GPXCopyright(QObject *parent) : QObject(parent)
{

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
