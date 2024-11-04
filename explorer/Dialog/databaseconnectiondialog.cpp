//
// Created by Robin on 11/3/2024.
//

// You may need to build the project (run Qt uic code generator) to get "ui_DatabaseConnectionDialog.h" resolved

#include "databaseconnectiondialog.h"
#include "ui_DatabaseConnectionDialog.h"


DatabaseConnectionDialog::DatabaseConnectionDialog(QWidget *parent) :
        QDialog(parent), ui(new Ui::DatabaseConnectionDialog)
{
    ui->setupUi(this);

    connect(ui->connectButton,
            &QPushButton::clicked,
            [this]() {
        accept();
    });

    connect(ui->cancelButton,
            &QPushButton::clicked,
            [this]() {
        reject();
    });
}

DatabaseConnectionDialog::~DatabaseConnectionDialog()
{
    delete ui;
}

QString DatabaseConnectionDialog::connectionName()
{
    return ui->connectionName->text();
}

QString DatabaseConnectionDialog::databaseAddress()
{
    return ui->address->text();
}

QString DatabaseConnectionDialog::databaseUsername()
{
    return ui->username->text();
}

QString DatabaseConnectionDialog::databasePassword()
{
    return ui->password->text();
}

int DatabaseConnectionDialog::databasePort()
{
    return ui->port->text().toInt();
}

QString DatabaseConnectionDialog::databaseName()
{
    return ui->database->text();
}
