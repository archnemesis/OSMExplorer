#ifndef GPSSOURCEDIALOG_H
#define GPSSOURCEDIALOG_H

#include <QDialog>
#include <QSerialPort>

namespace Ui {
class GpsSourceDialog;
}

class GpsSourceDialog : public QDialog
{
    Q_OBJECT

public:
    explicit GpsSourceDialog(QWidget *parent = nullptr);
    ~GpsSourceDialog();

    enum SourceType {
        NmeaSource,
        GpsdSource
    };

    struct GpsSourceInfo {
        SourceType sourceType;
        QString portName;
        int baudRate;
        QSerialPort::Parity parity;
        QSerialPort::DataBits dataBits;
        QSerialPort::StopBits stopBits;
        QSerialPort::FlowControl flowControl;
        QString label;
        bool centerOnLocation;
        bool keepInFrame;
        bool showMarker;
        bool isValid = false;
    };

    static GpsSourceInfo getNewSource(QWidget *parent, QString windowTitle);

private:
    Ui::GpsSourceDialog *ui;
};

#endif // GPSSOURCEDIALOG_H
