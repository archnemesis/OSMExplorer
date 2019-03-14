#ifndef SLIPPYMAPPOLYGONPROPERTYPAGE_H
#define SLIPPYMAPPOLYGONPROPERTYPAGE_H

#include "slippymapshapepropertypage.h"
#include "slippymapwidgetpolygon.h"

#include <QLineEdit>

class SlippyMapPolygonPropertyPage : public SlippyMapShapePropertyPage
{
public:
    SlippyMapPolygonPropertyPage(QWidget *parent = nullptr);
    SlippyMapPolygonPropertyPage(SlippyMapWidgetShape *shape, QWidget *parent = nullptr);
    QString tabTitle();
protected:
    SlippyMapWidgetPolygon *m_polygon;
    QLineEdit *m_lneX;
    QLineEdit *m_lneY;
    QLineEdit *m_lneWidth;
    QLineEdit *m_lneHeight;
};

#endif // SLIPPYMAPPOLYGONPROPERTYPAGE_H
