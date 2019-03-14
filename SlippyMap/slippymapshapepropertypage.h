#ifndef SLIPPYMAPSHAPEPROPERTYPAGE_H
#define SLIPPYMAPSHAPEPROPERTYPAGE_H

#include <QWidget>

#include "slippymapwidgetshape.h"

class SlippyMapShapePropertyPage : public QWidget
{
    Q_OBJECT
public:
    SlippyMapShapePropertyPage(QWidget *parent = nullptr);
    SlippyMapShapePropertyPage(SlippyMapWidgetShape *shape, QWidget *parent = nullptr);
    virtual ~SlippyMapShapePropertyPage();
    virtual QString tabTitle() = 0;
};

#endif // SLIPPYMAPSHAPEPROPERTYPAGE_H
