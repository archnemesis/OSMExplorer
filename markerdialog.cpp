#include "markerdialog.h"
#include "ui_markerdialog.h"

MarkerDialog::MarkerDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MarkerDialog)
{
    ui->setupUi(this);
}

MarkerDialog::~MarkerDialog()
{
    delete ui;
}

SlippyMapWidget::Marker *MarkerDialog::getNewMarker(QWidget *parent, QString title)
{
    MarkerDialog dlg(parent);
    if (title.length() > 0) dlg.setWindowTitle(title);
    int result = dlg.exec();

    if (result == MarkerDialog::Accepted) {
        bool ok;
        double lat = dlg.ui->lblLatitude->text().toDouble(&ok);
        double lon = dlg.ui->lblLongitude->text().toDouble(&ok);
        QString name = dlg.ui->lblName->text();
        QString description = dlg.ui->lblDescription->toPlainText();
        SlippyMapWidget::Marker *marker = new SlippyMapWidget::Marker(
                    lat,
                    lon,
                    name);
        return marker;
    }

    return nullptr;
}
