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

SlippyMapWidgetMarker *MarkerDialog::getNewMarker(QWidget *parent, QString title)
{
    MarkerDialog dlg(parent);
    if (title.length() > 0) dlg.setWindowTitle(title);
    int result = dlg.exec();

    if (result == MarkerDialog::Accepted) {
        bool ok;
        double lat = dlg.ui->lblLatitude->text().toDouble(&ok);
        double lon = dlg.ui->lblLongitude->text().toDouble(&ok);
        QPointF pos(lon, lat);
        QString name = dlg.ui->lblName->text();
        QString description = dlg.ui->txtInformation->toHtml();
        SlippyMapWidgetMarker *marker = new SlippyMapWidgetMarker(
                    pos,
                    name);
        marker->setInformation(description);
        return marker;
    }

    return nullptr;
}

bool MarkerDialog::getEditMarker(QWidget *parent, QString title, SlippyMapWidgetMarker *marker)
{
    MarkerDialog dlg(parent);
    if (title.length() > 0) dlg.setWindowTitle(title);

    dlg.ui->lblName->setText(marker->label());
    dlg.ui->txtInformation->setHtml(marker->information());
    dlg.ui->lblLatitude->setText(QString("%1").arg(marker->latitude(), 8, 'f', 4, '0'));
    dlg.ui->lblLongitude->setText(QString("%1").arg(marker->longitude(), 8, 'f', 4, '0'));

    int result = dlg.exec();

    if (result == MarkerDialog::Accepted) {
        bool ok;
        double lat = dlg.ui->lblLatitude->text().toDouble(&ok);
        double lon = dlg.ui->lblLongitude->text().toDouble(&ok);
        QPointF pos(lon, lat);
        QString name = dlg.ui->lblName->text();
        QString description = dlg.ui->txtInformation->toHtml();
        marker->setLabel(name);
        marker->setPosition(pos);
        marker->setInformation(description);
        return true;
    }

    return false;
}
