#ifndef NMEASERIALLOCATIONDATAPROVIDER_H
#define NMEASERIALLOCATIONDATAPROVIDER_H

#include "locationdataprovider.h"
#include <QSerialPort>
#include <QTimer>
#include <QList>



class NmeaSerialLocationDataProvider : public LocationDataProvider
{
    Q_OBJECT
public:
    NmeaSerialLocationDataProvider(QObject *parent = nullptr);

    void setLabelText(QString labelText);
    void setPortName(QString portName);
    void setBaudRate(int baudRate);
    void setDataBits(QSerialPort::DataBits dataBits);
    void setParity(QSerialPort::Parity parity);
    void setFlowControl(QSerialPort::FlowControl flowControl);
    void setStopBits(QSerialPort::StopBits stopBits);

    QString labelText();
    QString portName();
    int baudRate();
    QSerialPort::DataBits dataBits();
    QSerialPort::Parity parity();
    QSerialPort::FlowControl flowControl();
    QSerialPort::StopBits stopBits();

    void start() override;
    void stop() override;

    class SatelliteStatus {
    public:
        SatelliteStatus() {}
        ~SatelliteStatus() {}
        void setPrn(int prn) { m_prn = prn; }
        void setElevation(double elev) { m_elevation = elev; }
        void setAzimuth(double azim) { m_azimuth = azim; }
        void setSnr(int snr) { m_snr = snr; }
        int prn() { return m_prn; }
        double elevation() { return m_elevation; }
        double azimuth() { return m_azimuth; }
        int snr() { return m_snr; }

    private:
        int m_prn;
        double m_elevation;
        double m_azimuth;
        int m_snr;
    };

signals:
    void lineReceived(const QString& line);
    void satellitesUpdated(QString portName,
                           const QList<SatelliteStatus>& satellites,
                           QHash<QString, QVariant> metadata);

protected slots:
    void onSerialPortReadyRead();
    void onSerialPortErrorOccurred(QSerialPort::SerialPortError error);

protected:
    QString m_labelText;
    QString m_portName;
    int m_baudRate;
    QSerialPort::DataBits m_dataBits;
    QSerialPort::Parity m_parity;
    QSerialPort::FlowControl m_flowControl;
    QSerialPort::StopBits m_stopBits;
    QSerialPort *m_serialPort = nullptr;
    QTimer m_readTimer;
    QList<SatelliteStatus> m_satellites;
};

#endif // NMEASERIALLOCATIONDATAPROVIDER_H
