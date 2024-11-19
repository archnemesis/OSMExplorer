//
// Created by robin on 11/18/2024.
//

// You may need to build the project (run Qt uic code generator) to get "ui_ObjectBrowserDialog.h" resolved

#include "ObjectBrowserDialog.h"

#include <QLineEdit>
#include <QTreeView>
#include <QComboBox>
#include <QFormLayout>

#include "Application/ExplorerApplication.h"

ObjectBrowserDialog::ObjectBrowserDialog(const QUuid& workspaceId, QWidget *parent) :
    QWidget(parent),
    m_workspaceId(workspaceId)
{
    setupUi();

    m_model = new QStandardItemModel(this);
    m_model->setHorizontalHeaderLabels({
        tr("Name"),
        tr("Description"),
        tr("Created by")
    });
    m_treeView->setModel(m_model);

    m_serverInterface = ExplorerApplication::serverInterface();
    refresh();
}

void ObjectBrowserDialog::setupUi()
{
    m_searchLineEdit = new QLineEdit();
    m_searchLineEdit->setPlaceholderText(tr("Search text"));

    m_treeView = new QTreeView();

    m_ownerFilterComboBox = new QComboBox();
    m_typeFilterComboBox = new QComboBox();
    m_sortComboBox = new QComboBox();

    auto *formLayout = new QFormLayout();
    formLayout->addRow(tr("Search"), m_searchLineEdit);
    formLayout->addRow(tr("Type"), m_typeFilterComboBox);
    formLayout->addRow(tr("Owner"), m_ownerFilterComboBox);
    formLayout->addRow(tr("Sort"), m_sortComboBox);

    auto *vLayout = new QVBoxLayout();
    vLayout->addLayout(formLayout);
    vLayout->addWidget(m_treeView);

    setLayout(vLayout);
}

void ObjectBrowserDialog::refresh()
{
    m_serverInterface->getLayersForViewport(
        m_workspaceId,
        QRectF(),
        [this](const QList<ServerInterface::Layer>& layers) {
            for (const auto& layer : layers) {
                QList items = {
                    new QStandardItem(layer.name),
                    new QStandardItem(layer.description),
                    new QStandardItem(layer.creator.username)
                };

                for (const auto& object : layer.objects) {
                    QList childItems = {
                        new QStandardItem(object.label),
                        new QStandardItem(object.description),
                        new QStandardItem(object.creator.username)
                    };

                    items[0]->appendRow(childItems);
                }

                m_model->appendRow(items);
            }
        },
        [this](ServerInterface::RequestError error) {

        });
}
