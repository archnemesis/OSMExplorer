//
// Created by Robin on 11/3/2024.
//

#ifndef OSMEXPLORER_DATABASEMANAGER_H
#define OSMEXPLORER_DATABASEMANAGER_H

#include <QObject>


class DatabaseManager : public QObject
{
    Q_OBJECT
public:
    explicit DatabaseManager(QObject *parent = nullptr);
    bool connectDatabase();
    void setDatabaseName(const QString& databaseName);
    void setUsername(const QString& username);
    void setPassword(const QString& password);
    void setHostAddress(const QString& hostAddress);
    void setHostPort(const int port);

    const QString& hostAddress();
    const int& hostPort();
    const QString& databaseName();
    const QString& username();
    const QString& password();
    const QString &username() const;
    const QString &password() const;
    int port() const;

protected:
    QString m_hostAddress;
    QString m_databaseName;
    QString m_username;
public:

protected:
    QString m_password;
    int m_port;
};


#endif //OSMEXPLORER_DATABASEMANAGER_H
