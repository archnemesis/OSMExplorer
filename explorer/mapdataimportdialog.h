#ifndef MAPDATAIMPORTDIALOG_H
#define MAPDATAIMPORTDIALOG_H

#include <QDialog>

namespace Ui {
class MapDataImportDialog;
}

class MapDataImportDialog : public QDialog
{
    Q_OBJECT

public:
    explicit MapDataImportDialog(QWidget *parent = nullptr);
    ~MapDataImportDialog();

private:
    Ui::MapDataImportDialog *ui;
};

#endif // MAPDATAIMPORTDIALOG_H
