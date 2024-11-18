//
// Created by robin on 11/16/2024.
//

#include "GeoCodingResultListItem.h"
#include "GeoCodingInterface.h"

#include <QHBoxLayout>


GeoCodingResultListItem::GeoCodingResultListItem(QWidget* parent) : QWidget(parent)
{
    m_label = new QLabel(this);
    m_button = new QPushButton(this);

    auto *layout = new QHBoxLayout(this);
    layout->addWidget(m_label);
    layout->addWidget(m_button);
    setLayout(layout);
}

GeoCodingResultListItem::GeoCodingResultListItem(const GeoCodingInterface::GeoCodedAddress& address, QWidget* parent) :
    GeoCodingResultListItem(parent)
{
    m_label->setText(address.formatted);
}

