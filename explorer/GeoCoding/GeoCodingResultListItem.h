//
// Created by robin on 11/16/2024.
//

#ifndef GEOCODINGRESULTLISTITEM_H
#define GEOCODINGRESULTLISTITEM_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>

#include "GeoCodingInterface.h"


class GeoCodingResultListItem : public QWidget {
    Q_OBJECT
public:
    explicit GeoCodingResultListItem(QWidget *parent = nullptr);
    explicit GeoCodingResultListItem(const GeoCodingInterface::GeoCodedAddress& address, QWidget* parent = nullptr);

private:
    QLabel *m_label;
    QPushButton *m_button;
};



#endif //GEOCODINGRESULTLISTITEM_H
