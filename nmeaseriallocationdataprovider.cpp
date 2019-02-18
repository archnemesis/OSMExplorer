#include "nmeaseriallocationdataprovider.h"

#include <iostream>
#include <cmath>
#include <QDebug>
#include <QString>
#include <QStringRef>
#include <QVector>
#include <QSerialPort>
#include <QHash>
#include <QString>
#include <QVariant>

using namespace std;

NmeaSerialLocationDataProvider::NmeaSerialLocationDataProvider(QObject *parent) :
    LocationDataProvider(parent)
{
    if (m_serialPort == nullptr) {
        m_serialPort = new QSerialPort(this);
        connect(m_serialPort,
                &QSerialPort::readyRead,
                this,
                &NmeaSerialLocationDataProvider::onSerialPortReadyRead);
        connect(m_serialPort,
                &QSerialPort::errorOccurred,
                this,
                &NmeaSerialLocationDataProvider::onSerialPortErrorOccurred);
    }
}

void NmeaSerialLocationDataProvider::setLabelText(QString labelText)
{
    m_labelText = labelText;
}

void NmeaSerialLocationDataProvider::setPortName(QString portName)
{
    m_portName = portName;
}

void NmeaSerialLocationDataProvider::setBaudRate(int baudRate)
{
    m_baudRate = baudRate;
}

void NmeaSerialLocationDataProvider::setDataBits(QSerialPort::DataBits dataBits)
{
    m_dataBits = dataBits;
}

void NmeaSerialLocationDataProvider::setParity(QSerialPort::Parity parity)
{
    m_parity = parity;
}

void NmeaSerialLocationDataProvider::setFlowControl(QSerialPort::FlowControl flowControl)
{
    m_flowControl = flowControl;
}

void NmeaSerialLocationDataProvider::setStopBits(QSerialPort::StopBits stopBits)
{
    m_stopBits = stopBits;
}

QString NmeaSerialLocationDataProvider::labelText()
{
    return m_labelText;
}

QString NmeaSerialLocationDataProvider::portName()
{
    return m_portName;
}

int NmeaSerialLocationDataProvider::baudRate()
{
    return m_baudRate;
}

QSerialPort::DataBits NmeaSerialLocationDataProvider::dataBits()
{
    return m_dataBits;
}

QSerialPort::Parity NmeaSerialLocationDataProvider::parity()
{
    return m_parity;
}

QSerialPort::FlowControl NmeaSerialLocationDataProvider::flowControl()
{
    return m_flowControl;
}

QSerialPort::StopBits NmeaSerialLocationDataProvider::stopBits()
{
    return m_stopBits;
}

void NmeaSerialLocationDataProvider::start()
{

    m_serialPort->setPortName(m_portName);
    m_serialPort->setBaudRate(m_baudRate);
    m_serialPort->setDataBits(m_dataBits);
    m_serialPort->setFlowControl(m_flowControl);
    m_serialPort->setStopBits(m_stopBits);
    m_serialPort->setParity(m_parity);

    if (!m_serialPort->open(QIODevice::ReadOnly)) {
        emit dataProviderError();
        qDebug() << "Could not open port:" << m_serialPort->errorString();
        return;
    }

    m_serialPort->clear();
    m_serialPort->setTextModeEnabled(true);
}

void NmeaSerialLocationDataProvider::stop()
{
    if (m_serialPort != nullptr) {
        if (m_serialPort->isOpen()) {
            m_serialPort->close();
        }
    }
}

void NmeaSerialLocationDataProvider::onSerialPortReadyRead()
{
    if (!m_serialPort->canReadLine()) return;

    QByteArray data = m_serialPort->readLine(1024);

    //
    // $GPGGA,010139.238,4532.4814,N,12259.9776,W,1,03,03.3,00061.6,M,-019.6,M,000.0,0000*49
    //

    do {
        QString line = QString::fromLocal8Bit(data);
        QStringRef lineRef(&line);
        QVector<QStringRef> parts = lineRef.split(",");

        if (parts.count() > 1) {
            QStringRef lineType = parts[0];

            if (lineType == "$GPGGA" && parts.count() == 15) {
                QStringRef time = parts[1];
                int hour = time.mid(0, 2).toInt();
                int mins = time.mid(2, 2).toInt();
                double secs = time.mid(4, (time.length() - 4)).toDouble();

                qDebug() << "Got time:" << hour << mins << secs;

                bool ok;

                double lat = parts[2].toDouble(&ok);
                if (!ok) {
                    qDebug() << "Invalid latitude:" << parts[2];
                    goto nextLine;
                }

                double lon = parts[4].toDouble(&ok);
                if (!ok) {
                    qDebug() << "Invalid longitude:" << parts[4];
                    goto nextLine;
                }

                double lat_deg = floor(lat / 100.0);
                double lat_min = lat - (lat_deg * 100);
                lat_deg += (lat_min / 60.0);

                double lon_deg = floor(lon / 100.0);
                double lon_min = lon - (lon_deg * 100);
                lon_deg += (lon_min / 60.0);

                QStringRef latc = parts[3];
                if (parts[3] == "S") {
                    lat_deg = -1 * lat_deg;
                }

                QStringRef lonc = parts[5];
                if (parts[5] == "W") {
                    lon_deg = -1 * lon_deg;
                }

                QHash<QString,QVariant> metadata;
                metadata["gps_label"] = m_labelText;

                emit positionUpdated(
                            portName(),
                            QPointF(lon_deg, lat_deg),
                            metadata);
            }
        }

nextLine:
        data = m_serialPort->readLine();
    }
    while (data.length() > 0);
}

void NmeaSerialLocationDataProvider::onSerialPortErrorOccurred(QSerialPort::SerialPortError error)
{
    qDebug() << "Got Error:" << error;
}
