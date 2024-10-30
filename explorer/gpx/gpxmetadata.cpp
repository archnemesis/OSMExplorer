#include "gpxmetadata.h"
#include "gpxtracklink.h"

GPXMetadata::GPXMetadata(QObject *parent) : QObject(parent)
{

}

GPXMetadata::GPXMetadata(const GPXMetadata& other) {
    setName(other.name());
    setDescription(other.description());
    setAuthor(other.author());
    setCopyright(other.copyright());
    setTime(other.time());
    setBounds(other.bounds());

    for (const auto& link : other.links())
        addLink(link);
}

GPXMetadata& GPXMetadata::operator=(const GPXMetadata& other) {
    setName(other.name());
    setDescription(other.description());
    setAuthor(other.author());
    setCopyright(other.copyright());
    setTime(other.time());
    setBounds(other.bounds());

    for (const auto& link : other.links())
        addLink(link);

    return *this;
}

GPXMetadata::~GPXMetadata()
{
    m_links.clear();
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

const GPXPerson& GPXMetadata::author() const
{
    return m_author;
}

void GPXMetadata::setAuthor(const GPXPerson& author)
{
    m_author = author;
}

const GPXCopyright& GPXMetadata::copyright() const
{
    return m_copyright;
}

void GPXMetadata::setCopyright(const GPXCopyright& copyright)
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

const QList<GPXTrackLink>& GPXMetadata::links() const
{
    return m_links;
}

void GPXMetadata::addLink(const GPXTrackLink& link)
{
    m_links.append(link);
}
