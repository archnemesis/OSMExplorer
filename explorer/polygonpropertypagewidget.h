#ifndef POLYGONPROPERTYPAGEWIDGET_H
#define POLYGONPROPERTYPAGEWIDGET_H

#include "shapepropertypagewidget.h"
#include "slippymaplayerpolygon.h"

class QLineEdit;

class PolygonPropertyPageWidget : public ShapePropertyPageWidget
{
    Q_OBJECT
public:
    PolygonPropertyPageWidget(QWidget *parent = nullptr);
    PolygonPropertyPageWidget(SlippyMapLayerObject *object, QWidget *parent = nullptr);
protected:
    SlippyMapLayerPolygon *m_polygon;
    QLineEdit *m_lneX;
    QLineEdit *m_lneY;
    QLineEdit *m_lneWidth;
    QLineEdit *m_lneHeight;
};

#endif // POLYGONPROPERTYPAGEWIDGET_H
