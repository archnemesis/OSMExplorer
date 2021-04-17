#ifndef GPXPERSON_H
#define GPXPERSON_H

#include <QObject>

class GPXTrackLink;

class GPXPerson : public QObject
{
    Q_OBJECT
public:
    explicit GPXPerson(QObject *parent = nullptr);
    ~GPXPerson();

    void setName(const QString &name);
    void setEmail(const QString &email);
    void setLink(GPXTrackLink *link);

    QString name() const;
    QString email() const;
    GPXTrackLink *link() const;

private:
    QString m_name;
    QString m_email;
    GPXTrackLink *m_link = nullptr;
};

#endif // GPXPERSON_H
