#ifndef APRSDOTFICONFIGURATIONDIALOG_H
#define APRSDOTFICONFIGURATIONDIALOG_H

#include <QDialog>

namespace Ui {
class AprsDotFiConfigurationDialog;
}

class AprsDotFiConfigurationDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AprsDotFiConfigurationDialog(QWidget *parent = nullptr);
    ~AprsDotFiConfigurationDialog();

private slots:
    void on_btnAddCallsign_clicked();
    void on_btnRemoveCallsign_clicked();
    void onAcceptButtonClicked();
    void onRejectButtonClicked();

private:
    Ui::AprsDotFiConfigurationDialog *ui;
};

#endif // APRSDOTFICONFIGURATIONDIALOG_H
