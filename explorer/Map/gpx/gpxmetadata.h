#ifndef GPXMETADATA_H
#define GPXMETADATA_H

#include <QObject>
#include <QDateTime>
#include <QRectF>

#include "gpxtracklink.h"
#include "gpxperson.h"
#include "gpxcopyright.h"

class GPXMetadata : public QObject
{
    Q_OBJECT
public:
    explicit GPXMetadata(QObject *parent = nullptr);
    GPXMetadata(const GPXMetadata& other);
    GPXMetadata& operator=(const GPXMetadata& other);
    ~GPXMetadata();

    QString name() const;
    void setName(const QString &name);

    QString description() const;
    void setDescription(const QString &description);

    const GPXPerson& author() const;
    void setAuthor(const GPXPerson& author);

    const GPXCopyright& copyright() const;
    void setCopyright(const GPXCopyright& copyright);

    QDateTime time() const;
    void setTime(const QDateTime &time);

    QString keywords() const;
    void setKeywords(const QString &keywords);

    QRectF bounds() const;
    void setBounds(const QRectF &bounds);

    const QList<GPXTrackLink>& links() const;
    void addLink(const GPXTrackLink& link);

private:
    QString m_name;
    QString m_description;
    GPXPerson m_author;
    GPXCopyright m_copyright;
    QList<GPXTrackLink> m_links;
    QDateTime m_time;
    QString m_keywords;
    QRectF m_bounds;
};

#endif // GPXMETADATA_H
