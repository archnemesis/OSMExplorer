#ifndef NMEASERIALLOCATIONDATAPROVIDER_H
#define NMEASERIALLOCATIONDATAPROVIDER_H

#include "locationdataprovider.h"
#include <QSerialPort>

class NmeaSerialLocationDataProvider : public LocationDataProvider
{
    Q_OBJECT
public:
    NmeaSerialLocationDataProvider(QObject *parent = nullptr);

    void setPortName(QString portName);
    void setBaudRate(int baudRate);
    void setDataBits(QSerialPort::DataBits dataBits);
    void setParity(QSerialPort::Parity parity);
    void setFlowControl(QSerialPort::FlowControl flowControl);
    void setStopBits(QSerialPort::StopBits stopBits);

    QString portName();
    int baudRate();
    QSerialPort::DataBits dataBits();
    QSerialPort::Parity parity();
    QSerialPort::FlowControl flowControl();
    QSerialPort::StopBits stopBits();

    void start() override;
    void stop() override;

protected slots:
    void onSerialPortReadyRead();
    void onSerialPortErrorOccurred(QSerialPort::SerialPortError error);

protected:
    QString m_portName;
    int m_baudRate;
    QSerialPort::DataBits m_dataBits;
    QSerialPort::Parity m_parity;
    QSerialPort::FlowControl m_flowControl;
    QSerialPort::StopBits m_stopBits;
    QSerialPort *m_serialPort = nullptr;
};

#endif // NMEASERIALLOCATIONDATAPROVIDER_H
