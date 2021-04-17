#ifndef GPXMETADATA_H
#define GPXMETADATA_H

#include <QObject>
#include <QDateTime>
#include <QRectF>

class GPXTrackLink;
class GPXPerson;
class GPXCopyright;

class GPXMetadata : public QObject
{
    Q_OBJECT
public:
    explicit GPXMetadata(QObject *parent = nullptr);
    ~GPXMetadata();

    QString name() const;
    void setName(const QString &name);

    QString description() const;
    void setDescription(const QString &description);

    GPXPerson *author();
    void setAuthor(GPXPerson *author);

    GPXCopyright *copyright() const;
    void setCopyright(GPXCopyright *copyright);

    QDateTime time() const;
    void setTime(const QDateTime &time);

    QString keywords() const;
    void setKeywords(const QString &keywords);

    QRectF bounds() const;
    void setBounds(const QRectF &bounds);

    QList<GPXTrackLink *> links() const;
    void addLink(GPXTrackLink *link);

private:
    QString m_name;
    QString m_description;
    GPXPerson *m_author = nullptr;
    GPXCopyright *m_copyright = nullptr;
    QList<GPXTrackLink*> m_links;
    QDateTime m_time;
    QString m_keywords;
    QRectF m_bounds;
};

#endif // GPXMETADATA_H
