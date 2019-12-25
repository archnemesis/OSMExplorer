#ifndef SLIPPYMAPPOLYGONPROPERTYPAGE_H
#define SLIPPYMAPPOLYGONPROPERTYPAGE_H

#include "slippymap_global.h"
#include "slippymaplayerobjectpropertypage.h"
#include "slippymaplayerpolygon.h"

#include <QLineEdit>

class SlippyMapLayerObject;

class SLIPPYMAPSHARED_EXPORT SlippyMapLayerPolygonPropertyPage : public SlippyMapLayerObjectPropertyPage
{
public:
    SlippyMapLayerPolygonPropertyPage(QWidget *parent = nullptr);
    SlippyMapLayerPolygonPropertyPage(SlippyMapLayerObject *object, QWidget *parent = nullptr);
    virtual QString tabTitle();
protected:
    SlippyMapLayerPolygon *m_polygon;
    QLineEdit *m_lneX;
    QLineEdit *m_lneY;
    QLineEdit *m_lneWidth;
    QLineEdit *m_lneHeight;
};

#endif // SLIPPYMAPPOLYGONPROPERTYPAGE_H
