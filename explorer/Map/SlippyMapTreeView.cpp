//
// Created by Robin on 10/30/2024.
//

#include "SlippyMapTreeView.h"
#include <QWheelEvent>

void SlippyMapTreeView::wheelEvent(QWheelEvent *event)
{
    QAbstractScrollArea::wheelEvent(event);
    event->accept();
}

SlippyMapTreeView::SlippyMapTreeView(QWidget *parent) : QTreeView(parent)
{

}
