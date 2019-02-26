#ifndef APRSSTATIONLISTFORM_H
#define APRSSTATIONLISTFORM_H

#include <QWidget>

namespace Ui {
class AprsStationListForm;
}

class AprsStationListForm : public QWidget
{
    Q_OBJECT

public:
    explicit AprsStationListForm(QWidget *parent = nullptr);
    ~AprsStationListForm();

private:
    Ui::AprsStationListForm *ui;
};

#endif // APRSSTATIONLISTFORM_H
