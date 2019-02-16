#include "aprsdotficonfigurationdialog.h"
#include "ui_aprsdotficonfigurationdialog.h"

#include <QInputDialog>
#include <QSettings>
#include <QListWidgetItem>

AprsDotFiConfigurationDialog::AprsDotFiConfigurationDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AprsDotFiConfigurationDialog)
{
    ui->setupUi(this);
    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &AprsDotFiConfigurationDialog::onAcceptButtonClicked);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &AprsDotFiConfigurationDialog::onRejectButtonClicked);

    QSettings settings;
    int size = settings.beginReadArray("integrations/aprs.fi/callsigns");
    for (int i = 0; i < size; i++) {
        settings.setArrayIndex(i);
        ui->lstCallsigns->addItem(settings.value("callsign").toString());
    }
    settings.endArray();

    ui->lneApiUrl->setText(settings.value("integrations/aprs.fi/apiUrl").toString());
    ui->lneApiKey->setText(settings.value("integrations/aprs.fi/apiKey").toString());
    ui->spnUpdateInterval->setValue(settings.value("integrations/aprs.fi/updateInterval").toInt());
}

AprsDotFiConfigurationDialog::~AprsDotFiConfigurationDialog()
{
    delete ui;
}

void AprsDotFiConfigurationDialog::on_btnAddCallsign_clicked()
{
    QString callsign = QInputDialog::getText(
                this,
                tr("Add Callsign"),
                tr("Enter the name of the callsign as it appears on aprs.fi"));

    if (callsign.length() > 0) {
        ui->lstCallsigns->addItem(callsign);
    }
}

void AprsDotFiConfigurationDialog::on_btnRemoveCallsign_clicked()
{
    if (ui->lstCallsigns->currentRow() >= 0) {
        ui->lstCallsigns->takeItem(ui->lstCallsigns->currentRow());
    }
}

void AprsDotFiConfigurationDialog::onAcceptButtonClicked()
{
    QSettings settings;

    settings.setValue("integrations/aprs.fi/apiUrl", ui->lneApiUrl->text());
    settings.setValue("integrations/aprs.fi/apiKey", ui->lneApiKey->text());
    settings.setValue("integrations/aprs.fi/updateInterval", ui->spnUpdateInterval->value());

    settings.remove("integrations/aprs.fi/callsigns");
    settings.beginWriteArray("integrations/aprs.fi/callsigns");
    for (int i = 0; i < ui->lstCallsigns->count(); i++) {
        settings.setArrayIndex(i);
        settings.setValue("callsign", ui->lstCallsigns->item(i)->text());
    }
    settings.endArray();
    accept();
}

void AprsDotFiConfigurationDialog::onRejectButtonClicked()
{
    reject();
}
