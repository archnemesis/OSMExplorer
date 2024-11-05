//
// Created by Robin on 11/3/2024.
//

#include "DatabaseObjectPropertyPage.h"
#include <QLineEdit>
#include <QLabel>
#include <QFormLayout>
#include <QVBoxLayout>

DatabaseObjectPropertyPage::DatabaseObjectPropertyPage(SlippyMap::SlippyMapLayerObject::Ptr object)
        : SlippyMapLayerObjectPropertyPage(object)
{

}

QString DatabaseObjectPropertyPage::tabTitle()
{
    return tr("Database");
}

void DatabaseObjectPropertyPage::setupUi()
{
    m_id = new QLineEdit();
    m_syncLabel = new QLabel();

    m_id->setText(m_object->id().toString());
    m_id->setReadOnly(true);
    m_syncLabel->setText(m_object->isSynced() ? tr("Synced") : tr("Not Synced"));

    auto *form = new QFormLayout();
    form->addRow(tr("Object ID"), m_id);
    form->addRow(tr("Status"), m_syncLabel);

    auto *layout = new QVBoxLayout();
    layout->addLayout(form);
    layout->addStretch();

    setLayout(layout);
}

void DatabaseObjectPropertyPage::save()
{

}

void DatabaseObjectPropertyPage::updateUi()
{
    m_id->setText(m_object->id().toString());
    m_id->setReadOnly(true);
    m_syncLabel->setText(m_object->isSynced() ? tr("Synced") : tr("Not Synced"));
}
