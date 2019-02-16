#include "layerpropertiesdialog.h"
#include "ui_layerpropertiesdialog.h"

LayerPropertiesDialog::LayerPropertiesDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LayerPropertiesDialog)
{
    ui->setupUi(this);
}

LayerPropertiesDialog::~LayerPropertiesDialog()
{
    delete ui;
}

LayerPropertiesDialog::LayerProperties LayerPropertiesDialog::getLayerProperties(QWidget *parent, QString windowTitle)
{
    LayerPropertiesDialog dlg(parent);
    dlg.setWindowTitle(windowTitle);
    int result = dlg.exec();
    struct LayerProperties props;

    if (result == QDialog::Accepted) {
        props.isValid = true;
        props.name = dlg.ui->lneLayerName->text();
        props.description = dlg.ui->pteDescription->document()->toPlainText();
        props.tileServer = dlg.ui->lneTileServer->text();
        props.zOrder = dlg.ui->spnZOrder->value();
    }

    return props;
}

LayerPropertiesDialog::LayerProperties LayerPropertiesDialog::editLayerProperties(QWidget *parent, QString windowTitle, LayerPropertiesDialog::LayerProperties props)
{
    LayerPropertiesDialog dlg(parent);
    dlg.setWindowTitle(windowTitle);
    dlg.ui->lneLayerName->setText(props.name);
    dlg.ui->pteDescription->setPlainText(props.description);
    dlg.ui->lneTileServer->setText(props.tileServer);
    dlg.ui->spnZOrder->setValue(props.zOrder);
    props.isValid = false;

    int result = dlg.exec();

    if (result == QDialog::Accepted) {
        props.isValid = true;
        props.name = dlg.ui->lneLayerName->text();
        props.description = dlg.ui->pteDescription->document()->toPlainText();
        props.tileServer = dlg.ui->lneTileServer->text();
        props.zOrder = dlg.ui->spnZOrder->value();
    }

    return props;
}
