//
// Created by Robin on 11/5/2024.
//

#include "OEDockWidget.h"

OEDockWidget::OEDockWidget(QWidget *parent) : QDockWidget(parent)
{

}

void OEDockWidget::closeEvent(QCloseEvent *event)
{
    emit closed();
    QDockWidget::closeEvent(event);
}
