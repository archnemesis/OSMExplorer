#ifndef GPXPERSON_H
#define GPXPERSON_H

#include <QObject>

#include "gpxtracklink.h"

class GPXPerson : public QObject
{
    Q_OBJECT
public:
    explicit GPXPerson(QObject *parent = nullptr);
    GPXPerson(const GPXPerson& other);
    GPXPerson& operator=(const GPXPerson& other);
    ~GPXPerson();

    void setName(const QString &name);
    void setEmail(const QString &email);
    void setLink(const GPXTrackLink& link);

    const QString& name() const;
    const QString& email() const;
    const GPXTrackLink& link() const;

private:
    QString m_name;
    QString m_email;
    GPXTrackLink m_link;
};

#endif // GPXPERSON_H
