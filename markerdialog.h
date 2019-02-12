#ifndef MARKERDIALOG_H
#define MARKERDIALOG_H

#include <QDialog>
#include "slippymapwidget.h"

namespace Ui {
class MarkerDialog;
}

class MarkerDialog : public QDialog
{
    Q_OBJECT

public:
    explicit MarkerDialog(QWidget *parent = nullptr);
    ~MarkerDialog();

    static SlippyMapWidget::Marker *getNewMarker(QWidget *parent, QString title);

private:
    Ui::MarkerDialog *ui;
};

#endif // MARKERDIALOG_H
