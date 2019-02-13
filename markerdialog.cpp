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

bool MarkerDialog::getEditMarker(QWidget *parent, QString title, SlippyMapWidget::Marker *marker)
{
    MarkerDialog dlg(parent);
    if (title.length() > 0) dlg.setWindowTitle(title);
    int result = dlg.exec();

    dlg.ui->lblName->setText(marker->label());
    dlg.ui->lblDescription->setPlainText("");
    dlg.ui->lblLatitude->setText(QString("%1").arg(marker->latitude(), 8, 'f', 4, '0'));
    dlg.ui->lblLongitude->setText(QString("%1").arg(marker->longitude(), 8, 'f', 4, '0'));

    if (result == MarkerDialog::Accepted) {
        bool ok;
        double lat = dlg.ui->lblLatitude->text().toDouble(&ok);
        double lon = dlg.ui->lblLongitude->text().toDouble(&ok);
        QString name = dlg.ui->lblName->text();
        QString description = dlg.ui->lblDescription->toPlainText();
        marker->setLabel(name);
        marker->setLatitude(lat);
        marker->setLongitude(lon);
        return true;
    }

    return false;
}
