#include "settingsdialog.h"
#include "ui_settingsdialog.h"
#include "defaults.h"
#include "layerpropertiesdialog.h"
#include "explorerplugininterface.h"
#include "Application/ExplorerApplication.h"
#include "Application/PluginManager.h"

#include <QDebug>
#include <QSettings>
#include <QStandardPaths>

SettingsDialog::SettingsDialog(QList<ExplorerPluginInterface *> plugins, QWidget *parent) :
    m_plugins(plugins),
    QDialog(parent),
    ui(new Ui::SettingsDialog)
{
    ui->setupUi(this);
    ui->btnLayerConfigure->setEnabled(false);
    connect(ui->hslCacheSize, &QSlider::valueChanged, ui->spnCacheSize, &QSpinBox::setValue);
    connect(ui->spnCacheSize, QOverload<int>::of(&QSpinBox::valueChanged), ui->hslCacheSize, &QSlider::setValue);
    loadSettings();

    for (ExplorerPluginInterface *plugin : ExplorerApplication::pluginManager()->getPlugins()) {
        ui->lstIntegrationList->addItem(plugin->name());
    }
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

    bool cacheEnabled = settings.value("map/cache/enable", true).toBool();
    ui->chkCacheEnabled->setChecked(cacheEnabled);
    ui->lneCacheDir->setEnabled(cacheEnabled);
    ui->lneCacheDirBrowse->setEnabled(cacheEnabled);
    ui->hslCacheSize->setEnabled(cacheEnabled);
    ui->spnCacheSize->setEnabled(cacheEnabled);
    ui->btnClearCache->setEnabled(cacheEnabled);

    QString latLonUnits = settings.value("units/latlon", "dec").toString();
    if (latLonUnits == "dms") {
        ui->chkLatLonDegMinSec->setChecked(true);
        ui->chkLatLonDecimalDegrees->setChecked(false);
    }
    else {
        ui->chkLatLonDegMinSec->setChecked(false);
        ui->chkLatLonDecimalDegrees->setChecked(true);
    }

    ui->lneCacheDir->setText(settings.value("map/cache/tiledir", QStandardPaths::writableLocation(QStandardPaths::CacheLocation)).toString());
    ui->hslCacheSize->setValue(settings.value("map/cache/maxsize", 100).toInt());
    ui->spnCacheSize->setValue(settings.value("map/cache/maxsize", 100).toInt());

    m_layers.clear();
    ui->lstLayerList->clear();
    qDebug() << "Checking for layers config...";
    qDebug() << "Getting layers...";
    int count = settings.beginReadArray("layers");
    for (int i = 0; i < count; i++) {
        qDebug() << "Getting layer" << i;
        settings.setArrayIndex(i);
        LayerPropertiesDialog::LayerProperties props;
        props.name = settings.value("name").toString();
        props.description = settings.value("description").toString();
        props.tileServer = settings.value("tileServer").toString();
        props.zOrder = settings.value("zOrder").toInt();
        m_layers.append(props);
        ui->lstLayerList->addItem(props.name);
    }
    settings.endArray();

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

    settings.setValue("map/cache/enable", ui->chkCacheEnabled->isChecked());
    settings.setValue("map/cache/tiledir", ui->lneCacheDir->text());
    settings.setValue("map/cache/maxsize", ui->hslCacheSize->value());

    settings.beginWriteArray("layers");
    for (int i = 0; i < m_layers.count(); i++) {
        settings.setArrayIndex(i);
        settings.setValue("name", m_layers.at(i).name);
        settings.setValue("description", m_layers.at(i).description);
        settings.setValue("tileServer", m_layers.at(i).tileServer);
        settings.setValue("zOrder", m_layers.at(i).zOrder);
    }
    settings.endArray();

    if (ui->chkLatLonDegMinSec->isChecked()) {
        settings.setValue("units/latlon", "dms");
    }
    else {
        settings.setValue("units/latlon", "dec");
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

void SettingsDialog::on_chkCacheEnabled_toggled(bool checked)
{
    ui->lneCacheDir->setEnabled(checked);
    ui->lneCacheDirBrowse->setEnabled(checked);
    ui->hslCacheSize->setEnabled(checked);
    ui->spnCacheSize->setEnabled(checked);
    ui->btnClearCache->setEnabled(checked);
}

void SettingsDialog::on_btnIntegrationConfigure_clicked()
{
    qDebug() << "Showing config for plugin...";
    int currentRow = ui->lstIntegrationList->currentRow();
    if (currentRow >= 0) {
        QDialog *dlg = ExplorerApplication::pluginManager()->getPlugins().at(currentRow)->configurationDialog(this);
        dlg->setModal(true);
        int result = dlg->exec();
        delete dlg;
    }
}

void SettingsDialog::on_btnLayerAdd_clicked()
{
    LayerPropertiesDialog::LayerProperties properties =
            LayerPropertiesDialog::getLayerProperties(this);

    if (properties.isValid) {
        ui->lstLayerList->addItem(properties.name);
        m_layers.append(properties);
        m_layersChanged = true;
    }
}

void SettingsDialog::on_btnLayerDelete_clicked()
{
    int row = ui->lstLayerList->currentRow();
    if (row >= 0) {
        ui->lstLayerList->takeItem(row);
        m_layers.removeAt(row);
    }
    m_layersChanged = true;
}

void SettingsDialog::on_btnLayerConfigure_clicked()
{
    LayerPropertiesDialog::LayerProperties properties =
            LayerPropertiesDialog::editLayerProperties(
                this,
                tr("Layer Properties"),
                m_layers.at(ui->lstLayerList->currentRow()));
    if (properties.isValid) {
        m_layers.replace(ui->lstLayerList->currentRow(), properties);
        ui->lstLayerList->currentItem()->setText(properties.name);
        m_layersChanged = true;
    }
}

void SettingsDialog::on_lstLayerList_currentRowChanged(int currentRow)
{
    (void)currentRow;
    ui->btnLayerConfigure->setEnabled(true);
}

void SettingsDialog::on_lstLayerList_itemDoubleClicked(QListWidgetItem *item)
{
    (void)item;
    on_btnLayerConfigure_clicked();
}

bool SettingsDialog::layersChanged()
{
    return m_layersChanged;
}
