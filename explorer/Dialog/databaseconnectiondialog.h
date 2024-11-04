//
// Created by Robin on 11/3/2024.
//

#ifndef OSMEXPLORER_DATABASECONNECTIONDIALOG_H
#define OSMEXPLORER_DATABASECONNECTIONDIALOG_H

#include <QDialog>


QT_BEGIN_NAMESPACE
namespace Ui
{
    class DatabaseConnectionDialog;
}
QT_END_NAMESPACE

class DatabaseConnectionDialog : public QDialog
{
Q_OBJECT

public:
    explicit DatabaseConnectionDialog(QWidget *parent = nullptr);
    ~DatabaseConnectionDialog() override;
    QString connectionName();
    QString databaseName();
    QString databaseAddress();
    QString databaseUsername();
    QString databasePassword();
    int databasePort();

private:
    Ui::DatabaseConnectionDialog *ui;
};


#endif //OSMEXPLORER_DATABASECONNECTIONDIALOG_H
