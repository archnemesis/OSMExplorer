#include "gpssourcedialog.h"
#include "ui_gpssourcedialog.h"

#include <QSerialPortInfo>

GpsSourceDialog::GpsSourceDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::GpsSourceDialog)
{
    ui->setupUi(this);

    for (QSerialPortInfo port : QSerialPortInfo::availablePorts()) {
        ui->cboSerialPort->addItem(port.portName());
    }
}

GpsSourceDialog::~GpsSourceDialog()
{
    delete ui;
}

GpsSourceDialog::GpsSourceInfo GpsSourceDialog::getNewSource(QWidget *parent, QString windowTitle)
{
    GpsSourceDialog dlg(parent);
    GpsSourceInfo source;
    dlg.setWindowTitle(windowTitle);
    int result = dlg.exec();

    if (result == QDialog::Accepted) {
        if (dlg.ui->cboSourceType->currentIndex() == 0) {
            source.sourceType = GpsSourceDialog::NmeaSource;
            source.isValid = true;
            source.label = dlg.ui->lneMarkerLabel->text();
            source.centerOnLocation = dlg.ui->chkCenterMap->isChecked();
            source.keepInFrame = dlg.ui->chkKeepInFrame->isChecked();
            source.showMarker = dlg.ui->chkMarkerVisible->isChecked();
            source.portName = dlg.ui->cboSerialPort->currentText();
            source.baudRate = dlg.ui->cboSerialSpeed->currentText().toInt();

            switch (dlg.ui->cboSerialParity->currentIndex()) {
            case 0:
                source.parity = QSerialPort::NoParity;
                break;
            case 1:
                source.parity = QSerialPort::EvenParity;
                break;
            case 2:
                source.parity = QSerialPort::OddParity;
                break;
            }

            switch (dlg.ui->cboSerialDataBits->currentIndex()) {
            case 0:
                source.dataBits = QSerialPort::Data8;
                break;
            case 1:
                source.dataBits = QSerialPort::Data7;
                break;
            }

            switch (dlg.ui->cboSerialFlowControl->currentIndex()) {
            case 0:
                source.flowControl = QSerialPort::NoFlowControl;
                break;
            case 1:
                source.flowControl = QSerialPort::HardwareControl;
                break;
            case 2:
                source.flowControl = QSerialPort::SoftwareControl;
                break;
            }

            switch (dlg.ui->cboSerialStopBits->currentIndex()) {
            case 0:
                source.stopBits = QSerialPort::OneStop;
                break;
            case 1:
                source.stopBits = QSerialPort::OneAndHalfStop;
                break;
            case 2:
                source.stopBits = QSerialPort::TwoStop;
                break;
            }
        }
    }

    return source;
}
