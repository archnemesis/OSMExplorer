#ifndef NMEASERIALLOCATIONDATAPROVIDER_H
#define NMEASERIALLOCATIONDATAPROVIDER_H

#include "locationdataprovider.h"
#include <QSerialPort>
#include <QTimer>
#include <QList>
#include <QDateTime>


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

    class PositionData {
    public:
        enum FixType {
            FixNone,
            Fix2D,
            Fix3D
        };

        PositionData() {}
        void setGpsTime(const QDateTime& time) { m_time = time; }
        const QDateTime& gpsTime() const { return m_time; }
        void setSatellites(const QList<SatelliteStatus>& satellites) { m_satellites = satellites; }
        const QList<SatelliteStatus>& satellites() const { return m_satellites; }
        double latitude() const { return m_latitude; }
        double longitude() const { return m_longitude; }
        void setLatitude(double latitude) { m_latitude = latitude; }
        void setLongitude(double longitude) { m_longitude = longitude; }
        double altitude() const { return m_altitude; }
        void setAltitude(double altitude) { m_altitude = altitude; }

    private:
        QDateTime m_time;
        FixType m_fixType;
        double m_latitude;
        double m_longitude;
        double m_altitude;
        double m_hdop;
        double m_vdop;
        double m_pdop;
        double m_geoidSeparation;
        int m_numSatellites;
        QList<SatelliteStatus> m_satellites;
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
