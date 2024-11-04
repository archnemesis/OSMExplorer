//
// Created by Robin on 11/3/2024.
//

#include "DatabaseManager.h"
#include <QSqlDatabase>
#include <QDebug>

DatabaseManager::DatabaseManager(QObject *parent) : QObject(parent)
{

}

bool DatabaseManager::connectDatabase()
{
    QSqlDatabase db = QSqlDatabase::addDatabase("QPSQL7");
    db.setHostName(m_hostAddress);
    db.setPort(m_port);
    db.setUserName(m_username);
    db.setPassword(m_password);

    if (db.open()) {
        qDebug() << "Connection open!";
        return true;
    }

    return false;
}

void DatabaseManager::setUsername(const QString &username)
{
    m_username = username;
}

void DatabaseManager::setPassword(const QString &password)
{
    m_password = password;
}

void DatabaseManager::setHostAddress(const QString &hostAddress)
{
    m_hostAddress = hostAddress;
}

void DatabaseManager::setHostPort(const int port)
{
    m_port = port;
}

void DatabaseManager::setDatabaseName(const QString &databaseName)
{
    m_databaseName = databaseName;
}

const QString& DatabaseManager::hostAddress()
{
    return m_hostAddress;
}

const int& DatabaseManager::hostPort()
{
    return m_port;
}

const QString& DatabaseManager::databaseName()
{
    return m_databaseName;
}

const QString& DatabaseManager::username() const
{
    return m_username;
}

const QString& DatabaseManager::password() const
{
    return m_password;
}

int DatabaseManager::port() const
{
    return m_port;
}

