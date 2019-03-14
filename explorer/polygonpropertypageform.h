#ifndef POLYGONPROPERTYPAGEFORM_H
#define POLYGONPROPERTYPAGEFORM_H

#include <QWidget>

namespace Ui {
class PolygonPropertyPageForm;
}

class PolygonPropertyPageForm : public QWidget
{
    Q_OBJECT

public:
    explicit PolygonPropertyPageForm(QWidget *parent = nullptr);
    ~PolygonPropertyPageForm();

private:
    Ui::PolygonPropertyPageForm *ui;
};

#endif // POLYGONPROPERTYPAGEFORM_H
