//
// Created by Robin on 11/5/2024.
//

#ifndef OSMEXPLORER_SERVERCONNECTIONDIALOG_H
#define OSMEXPLORER_SERVERCONNECTIONDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QCheckBox>


class ServerConnectionDialog : public QDialog
{
    Q_OBJECT
public:
    explicit ServerConnectionDialog(QWidget *parent = nullptr);

    QString username();
    QString password();
    bool remember();
    void setUsername(const QString& username);
    void setPassword(const QString& password);
    void setRemember(bool remember);

private:
    QLineEdit *m_username;
    QLineEdit *m_password;
    QPushButton *m_loginButton;
    QPushButton *m_cancelButton;
    QCheckBox *m_rememberPassword;
};


#endif //OSMEXPLORER_SERVERCONNECTIONDIALOG_H
