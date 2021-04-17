#ifndef APRSMESSAGE_H
#define APRSMESSAGE_H

#include <QObject>

class APRSMessage : public QObject
{
    Q_OBJECT
public:
    explicit APRSMessage(QObject *parent = nullptr);

};

#endif // APRSMESSAGE_H
