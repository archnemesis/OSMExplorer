//
// Created by Robin on 11/5/2024.
//

// You may need to build the project (run Qt uic code generator) to get "ui_WorkspaceSelectionDialog.h" resolved

#include "WorkspaceSelectionDialog.h"
#include "ui_WorkspaceSelectionDialog.h"


WorkspaceSelectionDialog::WorkspaceSelectionDialog(QWidget *parent) :
        QDialog(parent), ui(new Ui::WorkspaceSelectionDialog)
{
    ui->setupUi(this);

    ui->existingWorkspaceList->setEnabled(false);

    connect(ui->newWorkspaceRadio,
            &QRadioButton::toggled,
            [this](bool state) {
        ui->newWorkspaceName->setEnabled(state);
        ui->newWorkspaceDescription->setEnabled(state);
        ui->existingWorkspaceList->setDisabled(state);
        if (state)
            ui->okButton->setEnabled(ui->newWorkspaceName->text().length() > 0);
        else
            ui->okButton->setEnabled(ui->existingWorkspaceList->currentIndex().isValid());
    });

    connect(ui->newWorkspaceName,
            &QLineEdit::textChanged,
            [this](const QString& text) {
        ui->okButton->setEnabled(text.length() > 0);
    });

    connect(ui->existingWorkspaceRadio,
            &QRadioButton::toggled,
            [this](bool state) {
                ui->newWorkspaceName->setDisabled(state);
                ui->newWorkspaceDescription->setDisabled(state);
                ui->existingWorkspaceList->setEnabled(state);
            });

    ui->okButton->setEnabled(false);
    connect(ui->existingWorkspaceList,
            &QListView::clicked,
            [this](QModelIndex index) {
        if (index.isValid())
            ui->okButton->setEnabled(true);
        else
            ui->okButton->setEnabled(false);
    });

    connect(ui->existingWorkspaceList,
            &QListView::activated,
            [this](QModelIndex index) {
        if (index.isValid()) {
            ui->okButton->setEnabled(true);
            accept();
        }
    });
}

WorkspaceSelectionDialog::~WorkspaceSelectionDialog()
{
    delete ui;
}

bool WorkspaceSelectionDialog::createNew() const
{
    return ui->newWorkspaceRadio->isChecked();
}

QString WorkspaceSelectionDialog::newWorkspaceName() const
{
    return ui->newWorkspaceName->text();
}

QString WorkspaceSelectionDialog::newWorkspaceDescription() const
{
    return ui->newWorkspaceDescription->toPlainText();
}

QUuid WorkspaceSelectionDialog::existingWorkspaceId() const
{
    int index = ui->existingWorkspaceList->currentRow();
    QUuid id = m_workspaceList.keys().at(index);
    return id;
}

void WorkspaceSelectionDialog::setWorkspaceList(const QMap<QUuid, QString> &workspaceList)
{
    m_workspaceList = workspaceList;
    for (const auto& id: m_workspaceList.keys()) {
        ui->existingWorkspaceList->addItem(m_workspaceList.value(id));
    }
}
