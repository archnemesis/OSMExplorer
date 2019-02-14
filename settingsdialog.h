#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>

namespace Ui {
class SettingsDialog;
}

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget *parent = nullptr);
    ~SettingsDialog();

public slots:
    void loadDefaults();
    void loadSettings();

private slots:
    void on_buttonBox_accepted();
    void on_buttonBox_rejected();

    void on_cboWayfindingService_currentIndexChanged(int index);

private:
    Ui::SettingsDialog *ui;
};

#endif // SETTINGSDIALOG_H
