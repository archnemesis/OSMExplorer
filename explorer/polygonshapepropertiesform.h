#ifndef POLYGONSHAPEPROPERTIESFORM_H
#define POLYGONSHAPEPROPERTIESFORM_H

#include <QWidget>

namespace Ui {
class PolygonShapePropertiesForm;
}

class PolygonShapePropertiesForm : public QWidget
{
    Q_OBJECT

public:
    explicit PolygonShapePropertiesForm(QWidget *parent = nullptr);
    ~PolygonShapePropertiesForm();

private:
    Ui::PolygonShapePropertiesForm *ui;
};

#endif // POLYGONSHAPEPROPERTIESFORM_H
