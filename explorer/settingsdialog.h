#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include "layerpropertiesdialog.h"
#include <QDialog>
#include <QListWidgetItem>

namespace Ui {
class SettingsDialog;
}

class ExplorerPluginInterface;

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(QList<ExplorerPluginInterface*> plugins = QList<ExplorerPluginInterface*>(), QWidget *parent = nullptr);
    ~SettingsDialog();

public slots:
    void loadDefaults();
    void loadSettings();

private slots:
    void on_buttonBox_accepted();
    void on_buttonBox_rejected();
    void on_cboWayfindingService_currentIndexChanged(int index);
    void on_btnChooseDefaultLocation_clicked();
    void on_chkCacheEnabled_toggled(bool checked);
    void on_btnIntegrationConfigure_clicked();
    void on_btnLayerAdd_clicked();
    void on_btnLayerDelete_clicked();
    void on_btnLayerConfigure_clicked();
    void on_lstLayerList_currentRowChanged(int currentRow);
    void on_lstLayerList_itemDoubleClicked(QListWidgetItem *item);

private:
    Ui::SettingsDialog *ui;
    QList<LayerPropertiesDialog::LayerProperties> m_layers;
    QList<ExplorerPluginInterface*> m_plugins;
};

#endif // SETTINGSDIALOG_H
