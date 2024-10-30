#ifndef GPXTRACKLINK_H
#define GPXTRACKLINK_H

#include <QObject>

class GPXTrackLink : public QObject
{
    Q_OBJECT
public:
    GPXTrackLink() = default;
    GPXTrackLink(const GPXTrackLink& other);
    GPXTrackLink& operator=(const GPXTrackLink& other);
    QString href() const;
    QString text() const;
    QString type() const;
    void setHref(QString href);
    void setText(QString text);
    void setType(QString type);
private:
    QString m_href;
    QString m_text;
    QString m_type;

};

#endif // GPXTRACKLINK_H
