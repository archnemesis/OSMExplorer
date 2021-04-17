#include "mapdataimportdialog.h"
#include "ui_mapdataimportdialog.h"

MapDataImportDialog::MapDataImportDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MapDataImportDialog)
{
    ui->setupUi(this);
}

MapDataImportDialog::~MapDataImportDialog()
{
    delete ui;
}
