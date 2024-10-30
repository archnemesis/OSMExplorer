#include "markerdialog.h"
#include "ui_markerdialog.h"
#include <SlippyMap/SlippyMapWidget.h>

using namespace SlippyMap;

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
        return nullptr;
    }

    return nullptr;
}

bool MarkerDialog::getEditMarker(QWidget *parent, QString title, SlippyMapWidgetMarker *marker)
{
    MarkerDialog dlg(parent);
    if (title.length() > 0) dlg.setWindowTitle(title);

    dlg.ui->lblName->setText(marker->label());
    dlg.ui->txtInformation->setHtml(marker->description());
    dlg.ui->lblCoordinates->setText(tr("Coordinates: %1").arg(SlippyMapWidget::geoCoordinatesToString(marker->position())));

    if (!marker->isEditable()) {
        dlg.ui->lblName->setReadOnly(true);
        dlg.ui->txtInformation->setReadOnly(true);
    }

    int result = dlg.exec();

    if (result == MarkerDialog::Accepted) {
        if (marker->isEditable()) {
            QString name = dlg.ui->lblName->text();
            QString description = dlg.ui->txtInformation->toHtml();
            marker->setLabel(name);
            marker->setDescription(description);
        }
        return true;
    }

    return false;
}
