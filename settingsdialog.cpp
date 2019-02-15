#include "settingsdialog.h"
#include "ui_settingsdialog.h"
#include "defaults.h"

#include <QSettings>

SettingsDialog::SettingsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SettingsDialog)
{
    ui->setupUi(this);
    loadSettings();
}

SettingsDialog::~SettingsDialog()
{
    delete ui;
}

void SettingsDialog::loadDefaults()
{

}

void SettingsDialog::loadSettings()
{
    QSettings settings;
    ui->lneDefaultLatitude->setText(settings.value("map/defaults/latitude", DEFAULT_LATITUDE).toString());
    ui->lneDefaultLongitude->setText(settings.value("map/defaults/longitude", DEFAULT_LONGITUDE).toString());
    ui->spnDefaultZoom->setValue(settings.value("map/defaults/zoomLevel", DEFAULT_ZOOM).toInt());
    ui->chkCenterOnCursorWhileZooming->setChecked(settings.value("map/zoom/centerOnCursor", DEFAULT_CENTER_ON_CURSOR_ZOOM).toBool());

    if (settings.value("wayfinding/service").toString() == "openrouteservice.org") {
        ui->cboWayfindingService->setCurrentIndex(1);
        ui->lneOpenRouteServiceURL->setText(settings.value("wayfinding/openrouteservice/url").toString());
        ui->lneOpenRouteServiceAPIKey->setText(settings.value("wayfinding/openrouteservice/apikey").toString());
        ui->grpOpenRouteServiceSettings->setVisible(true);
    }
    else {
        ui->cboWayfindingService->setCurrentIndex(0);
        ui->grpOpenRouteServiceSettings->setVisible(false);
    }
}

void SettingsDialog::on_buttonBox_accepted()
{
    QSettings settings;
    settings.setValue("map/defaults/latitude", ui->lneDefaultLatitude->text().toDouble());
    settings.setValue("map/defaults/longitude", ui->lneDefaultLongitude->text().toDouble());
    settings.setValue("map/defaults/zoomLevel", ui->spnDefaultZoom->value());
    settings.setValue("map/zoom/centerOnCursor", ui->chkCenterOnCursorWhileZooming->isChecked());

    switch (ui->cboWayfindingService->currentIndex())
    {
    case 0:
        settings.remove("wayfinding/service");
        settings.remove("wayfinding/openrouteservice");
        break;
    case 1:
        settings.setValue("wayfinding/service", "openrouteservice.org");
        settings.setValue("wayfinding/openrouteservice/url", ui->lneOpenRouteServiceURL->text());
        settings.setValue("wayfinding/openrouteservice/apikey", ui->lneOpenRouteServiceAPIKey->text());
        break;
    }

    accept();
}

void SettingsDialog::on_buttonBox_rejected()
{
    loadSettings();
    reject();
}

void SettingsDialog::on_cboWayfindingService_currentIndexChanged(int index)
{
    switch (index) {
    case 0:
        ui->grpOpenRouteServiceSettings->setVisible(false);
        break;
    case 1:
        ui->grpOpenRouteServiceSettings->setVisible(true);
    }
}

void SettingsDialog::on_btnChooseDefaultLocation_clicked()
{

}
