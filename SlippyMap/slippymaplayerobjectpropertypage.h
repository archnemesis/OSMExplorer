#ifndef SLIPPYMAPSHAPEPROPERTYPAGE_H
#define SLIPPYMAPSHAPEPROPERTYPAGE_H

#include "slippymap_global.h"

#include <QWidget>

class SlippyMapLayerObject;

class SLIPPYMAPSHARED_EXPORT SlippyMapLayerObjectPropertyPage : public QWidget
{
    Q_OBJECT
public:
    SlippyMapLayerObjectPropertyPage(QWidget *parent = nullptr);
    SlippyMapLayerObjectPropertyPage(SlippyMapLayerObject *object, QWidget *parent = nullptr);
    virtual ~SlippyMapLayerObjectPropertyPage();
    virtual QString tabTitle() = 0;
};

#endif // SLIPPYMAPSHAPEPROPERTYPAGE_H
