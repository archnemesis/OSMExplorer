#include "gpxmetadata.h"
#include "gpxtracklink.h"

GPXMetadata::GPXMetadata(QObject *parent) : QObject(parent)
{

}

GPXMetadata::~GPXMetadata()
{
    for (GPXTrackLink *link : m_links) {
        delete link;
    }
    m_links.clear();

    if (m_copyright != nullptr) {
        delete m_copyright;
    }
}

QString GPXMetadata::name() const
{
    return m_name;
}

void GPXMetadata::setName(const QString &name)
{
    m_name = name;
}

QString GPXMetadata::description() const
{
    return m_description;
}

void GPXMetadata::setDescription(const QString &description)
{
    m_description = description;
}

GPXPerson *GPXMetadata::author()
{
    return m_author;
}

void GPXMetadata::setAuthor(GPXPerson *author)
{
    m_author = author;
}

GPXCopyright *GPXMetadata::copyright() const
{
    return m_copyright;
}

void GPXMetadata::setCopyright(GPXCopyright *copyright)
{
    m_copyright = copyright;
}

QDateTime GPXMetadata::time() const
{
    return m_time;
}

void GPXMetadata::setTime(const QDateTime &time)
{
    m_time = time;
}

QString GPXMetadata::keywords() const
{
    return m_keywords;
}

void GPXMetadata::setKeywords(const QString &keywords)
{
    m_keywords = keywords;
}

QRectF GPXMetadata::bounds() const
{
    return m_bounds;
}

void GPXMetadata::setBounds(const QRectF &bounds)
{
    m_bounds = bounds;
}

QList<GPXTrackLink *> GPXMetadata::links() const
{
    return m_links;
}

void GPXMetadata::addLink(GPXTrackLink *link)
{
    m_links.append(link);
}
