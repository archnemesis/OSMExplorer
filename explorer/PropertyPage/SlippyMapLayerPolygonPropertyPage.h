#ifndef SLIPPYMAPPOLYGONPROPERTYPAGE_H
#define SLIPPYMAPPOLYGONPROPERTYPAGE_H

#include <SlippyMap/SlippyMapLayerPolygon.h>
#include "SlippyMapLayerObjectPropertyPage.h"

using namespace SlippyMap;

class QLineEdit;

class SLIPPYMAPSHARED_EXPORT SlippyMapLayerPolygonPropertyPage : public SlippyMapLayerObjectPropertyPage
{
public:
    explicit SlippyMapLayerPolygonPropertyPage(SlippyMapLayerObject *object);
    SlippyMapLayerPolygonPropertyPage(SlippyMapLayerObject* object, QWidget* parent);
    virtual QString tabTitle();
protected:
    void setupUi();

    SlippyMapLayerPolygon *m_polygon;
    QLineEdit *m_lneX;
    QLineEdit *m_lneY;
    QLineEdit *m_lneWidth;
    QLineEdit *m_lneHeight;
};

#endif // SLIPPYMAPPOLYGONPROPERTYPAGE_H
