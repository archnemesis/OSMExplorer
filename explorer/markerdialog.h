#ifndef MARKERDIALOG_H
#define MARKERDIALOG_H

#include <QDialog>
#include "slippymapwidget.h"
#include "slippymapwidgetmarker.h"

namespace Ui {
class MarkerDialog;
}

class MarkerDialog : public QDialog
{
    Q_OBJECT

public:
    explicit MarkerDialog(QWidget *parent = nullptr);
    ~MarkerDialog();

    static SlippyMapWidgetMarker *getNewMarker(QWidget *parent, QString title);
    static bool getEditMarker(QWidget *parent, QString title, SlippyMapWidgetMarker *marker);

private:
    Ui::MarkerDialog *ui;
};

#endif // MARKERDIALOG_H
