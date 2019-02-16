#ifndef LAYERPROPERTIESDIALOG_H
#define LAYERPROPERTIESDIALOG_H

#include <QDialog>

namespace Ui {
class LayerPropertiesDialog;
}

class LayerPropertiesDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LayerPropertiesDialog(QWidget *parent = nullptr);
    ~LayerPropertiesDialog();

    struct LayerProperties {
        bool isValid = false;
        QString name;
        QString description;
        QString tileServer;
        int zOrder;
    };

    static struct LayerProperties getLayerProperties(QWidget *parent = nullptr, QString windowTitle = "Layer Properties");
    static struct LayerProperties editLayerProperties(QWidget *parent, QString windowTitle, struct LayerProperties props);

private:
    Ui::LayerPropertiesDialog *ui;
};

#endif // LAYERPROPERTIESDIALOG_H
