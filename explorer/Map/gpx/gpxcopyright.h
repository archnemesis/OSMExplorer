#ifndef GPXCOPYRIGHT_H
#define GPXCOPYRIGHT_H

#include <QObject>

class GPXCopyright : public QObject
{
    Q_OBJECT
public:
    explicit GPXCopyright(QObject *parent = nullptr);
    GPXCopyright(const GPXCopyright& other);
    GPXCopyright& operator=(const GPXCopyright& other);

    QString author() const;
    void setAuthor(const QString &author);

    QString year() const;
    void setYear(const QString &year);

    QString license() const;
    void setLicense(const QString &license);

private:
    QString m_author;
    QString m_year;
    QString m_license;
};

#endif // GPXCOPYRIGHT_H
