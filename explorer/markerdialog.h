#ifndef MARKERDIALOG_H
#define MARKERDIALOG_H

#include <QDialog>
#include <SlippyMap/SlippyMapWidgetMarker.h>

namespace Ui {
class MarkerDialog;
}

using namespace SlippyMap;

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
