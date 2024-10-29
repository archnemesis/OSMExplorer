#ifndef RAINVIEWER_GLOBAL_H
#define RAINVIEWER_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(APRS_LIBRARY)
#  define RAINVIEWERSHARED_EXPORT Q_DECL_EXPORT
#else
#  define RAINVIEWERSHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // RAINVIEWER_GLOBAL_H
