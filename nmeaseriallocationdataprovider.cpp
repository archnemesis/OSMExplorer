#include "nmeaseriallocationdataprovider.h"

#include <QDebug>
#include <QString>
#include <QStringRef>
#include <QVector>
#include <QSerialPort>

NmeaSerialLocationDataProvider::NmeaSerialLocationDataProvider(QObject *parent) :
    LocationDataProvider(parent)
{

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

    m_serialPort->setPortName(m_portName);
    m_serialPort->setBaudRate(m_baudRate);
    m_serialPort->setDataBits(m_dataBits);
    m_serialPort->setFlowControl(m_flowControl);
    m_serialPort->setStopBits(m_stopBits);
    m_serialPort->setParity(m_parity);

    if (!m_serialPort->open(QIODevice::ReadWrite)) {
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
    QByteArray data = m_serialPort->readLine();

    //
    // $GPGGA,010139.238,4532.4814,N,12259.9776,W,1,03,03.3,00061.6,M,-019.6,M,000.0,0000*49
    //

    do {
        QString line = QString::fromLocal8Bit(data);
        QStringRef lineRef(&line);
        QVector<QStringRef> parts = lineRef.split(",");
        QStringRef lineType = parts[0];

        if (lineType == "$GPGGA") {
            QStringRef time = parts[1];
            int hour = time.mid(0, 2).toInt();
            int mins = time.mid(2, 2).toInt();
            double secs = time.mid(4, (time.length() - 4)).toDouble();

            qDebug() << "Got time:" << hour << mins << secs;
        }

        data = m_serialPort->readLine();
    }
    while (data.length() > 0);
}

void NmeaSerialLocationDataProvider::onSerialPortErrorOccurred(QSerialPort::SerialPortError error)
{
    qDebug() << "Got Error:" << error;
}
