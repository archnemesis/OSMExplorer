#ifndef APRSPARSER_H
#define APRSPARSER_H

#include "aprs_global.h"
#include <QObject>

class APRSSHARED_EXPORT APRSParser : public QObject
{
    Q_OBJECT
public:
    APRSParser();
};

#endif // APRSPARSER_H
