#ifndef POLYGONPROPERTYPAGEWIDGET_H
#define POLYGONPROPERTYPAGEWIDGET_H

#include "shapepropertypagewidget.h"
#include "slippymapwidgetpolygon.h"

class QLineEdit;

class PolygonPropertyPageWidget : public ShapePropertyPageWidget
{
    Q_OBJECT
public:
    PolygonPropertyPageWidget(QWidget *parent = nullptr);
    PolygonPropertyPageWidget(SlippyMapWidgetShape *shape, QWidget *parent = nullptr);
protected:
    SlippyMapWidgetPolygon *m_polygon;
    QLineEdit *m_lneX;
    QLineEdit *m_lneY;
    QLineEdit *m_lneWidth;
    QLineEdit *m_lneHeight;
};

#endif // POLYGONPROPERTYPAGEWIDGET_H
