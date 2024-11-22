#include "nmeaseriallocationdataprovider.h"
#include "nmea/message/gsv.hpp"

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
#include <QDateTime>

#include <nmea/sentence.hpp>
#include <nmea/message/gga.hpp>
#include <nmea/message/gll.hpp>
#include <nmea/message/gsa.hpp>

using namespace std;
using namespace nmea;

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

//    m_nmeaParser = new NMEAParser();
//    m_gpsService = new GPSService(*m_nmeaParser);
//    m_gpsService->onUpdate += [this]() {
//        if (m_gpsService->fix.locked()) {
//            QHash<QString,QVariant> metadata;
//            metadata["gps_label"] = m_labelText;
//
//            emit positionUpdated(
//                    portName(),
//                    QPointF(m_gpsService->fix.longitude, m_gpsService->fix.latitude),
//                    metadata);
//        }
//    };
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
    while (m_serialPort->canReadLine()) {
        QByteArray data = m_serialPort->readLine();

        // nmea library chokes on anything less than a full sentence
        if (data.length() <= 1) continue;

        sentence nmea_sentence(data.toStdString());

        emit lineReceived(QString::fromLocal8Bit(data).trimmed());

        if (nmea_sentence.type() == "GGA") {
            gga nmea_gga(nmea_sentence);
            if (nmea_gga.latitude.exists() && nmea_gga.longitude.exists()) {
                QHash<QString,QVariant> metadata;
                metadata["gps_label"] = m_labelText;

                m_positionData.setLatitude(nmea_gga.latitude.get());
                m_positionData.setLongitude(nmea_gga.longitude.get());
                m_positionData.setAltitude(nmea_gga.altitude.get());
                gpsUpdated(m_positionData);

                emit positionUpdated(
                        portName(),
                        QPointF(nmea_gga.longitude.get(), nmea_gga.latitude.get()),
                        metadata);
            }
            if (nmea_gga.utc.exists()) {
                double utc = nmea_gga.utc.get();
                QDateTime time = QDateTime::fromTime_t((unsigned int)utc);
                emit gpsTimeUpdated(time);

                m_positionData.setGpsTime(time);
                emit gpsUpdated(m_positionData);
            }
        }
        else if (nmea_sentence.type() == "GSA") {
            gsa nmea_gsa(nmea_sentence);

            emit gpsSatelliteStatusUpdated(
                nmea_gsa.hdop.get(),
                nmea_gsa.vdop.get(),
                nmea_gsa.pdop.get());

            m_positionData.setHdop(nmea_gsa.hdop.get());
            m_positionData.setVdop(nmea_gsa.vdop.get());
            m_positionData.setPdop(nmea_gsa.pdop.get());

            switch (nmea_gsa.fix.get()) {
            case gsa::fix_type::FIX_2D:
                m_positionData.setFixType(PositionData::Fix2D);
                break;
            case gsa::fix_type::FIX_3D:
                m_positionData.setFixType(PositionData::Fix3D);
                break;
            case gsa::fix_type::NONE:
                m_positionData.setFixType(PositionData::FixNone);
                break;
            }

            emit gpsUpdated(m_positionData);
        }
        else if (nmea_sentence.type() == "GSV") {
            gsv nmea_gsv(nmea_sentence);

            if (nmea_gsv.message_number.get() == 1) {
                m_satellites.clear();
            }

            if (nmea_gsv.satellite_count.exists()) {
                for (int i = 0; i < nmea_gsv.satellites.size(); i++) {
                    SatelliteStatus sat;

                    if (nmea_gsv.satellites.at(i).azimuth.exists())
                        sat.setAzimuth(nmea_gsv.satellites.at(i).azimuth.get());
                    if (nmea_gsv.satellites.at(i).elevation.exists())
                        sat.setElevation(nmea_gsv.satellites.at(i).elevation.get());
                    if (nmea_gsv.satellites.at(i).snr.exists())
                        sat.setSnr(nmea_gsv.satellites.at(i).snr.get());
                    if (nmea_gsv.satellites.at(i).prn.exists())
                        sat.setPrn(nmea_gsv.satellites.at(i).prn.get());
                    m_satellites[sat.prn()] = sat;
                }

                QHash<QString,QVariant> metadata;
                metadata["gps_label"] = m_labelText;
                emit satellitesUpdated(portName(), m_satellites, metadata);
            }

            if (nmea_gsv.message_number.get() == nmea_gsv.message_count.get()) {
                m_positionData.setSatellites(m_satellites.values());
                emit gpsUpdated(m_positionData);
            }
        }
    }
}

void NmeaSerialLocationDataProvider::onSerialPortErrorOccurred(QSerialPort::SerialPortError error)
{
    qDebug() << "Got Error:" << error;
}
