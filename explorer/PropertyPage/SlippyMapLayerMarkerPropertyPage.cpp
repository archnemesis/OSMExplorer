//
// Created by robin on 10/30/2024.
//

#include "SlippyMapLayerMarkerPropertyPage.h"

#include <QLineEdit>
#include <QPlainTextEdit>
#include <QTabWidget>
#include <QFormLayout>
#include <QPushButton>
#include <QCheckBox>

#include <SlippyMap/SlippyMapWidgetMarker.h>

SlippyMapLayerMarkerPropertyPage::SlippyMapLayerMarkerPropertyPage(SlippyMapLayerObject *object) :
    SlippyMapLayerObjectPropertyPage(object) {
    m_name = new QLineEdit();
    m_description = new QPlainTextEdit();
    m_latitude = new QLineEdit();
    m_longitude = new QLineEdit();
    m_tabWidget = new QTabWidget();
    m_visibility = new QCheckBox();
    setupUi();
}

void SlippyMapLayerMarkerPropertyPage::setupUi()
{
    m_name->setText(m_object->label());
    m_description->setPlainText(m_object->description());
    m_latitude->setText(QString("%1").arg(m_object->position().y()));
    m_longitude->setText(QString("%1").arg(m_object->position().x()));
    m_visibility->setText(tr("Visible"));
    m_visibility->setChecked(m_object->isVisible());

    auto *fLayout = new QFormLayout();
    fLayout->addRow(tr("Label"), m_name);
    fLayout->addRow(tr("Description"), m_description);
    fLayout->addRow(tr("Latitude"), m_latitude);
    fLayout->addRow(tr("Longitude"), m_longitude);
    fLayout->addRow("", m_visibility);

    auto *formWidgetLayout = new QHBoxLayout();
    formWidgetLayout->addLayout(fLayout);
    formWidgetLayout->addStretch();

    auto *formWidget = new QWidget();
    formWidget->setLayout(formWidgetLayout);

    m_tabWidget->addTab(formWidget, tr("Properties"));

    auto *saveButton = new QPushButton(tr("Save"));
    connect(saveButton,
        &QPushButton::clicked,
        [this]() {
            auto *obj = qobject_cast<SlippyMapWidgetMarker*>(m_object);
            obj->setLabel(m_name->text());
            obj->setDescription(m_description->toPlainText());
            obj->setVisible(m_visibility->isChecked());

            QPointF position;
            position.setY(m_latitude->text().toDouble());
            position.setX(m_longitude->text().toDouble());
            obj->setPosition(position);

            close();
        });

    auto *buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    buttonLayout->addWidget(saveButton);

    auto *vLayout = new QVBoxLayout();
    vLayout->addWidget(m_tabWidget);
    vLayout->addLayout(buttonLayout);
    setLayout(vLayout);
}
