//
// Created by Robin on 10/30/2024.
//

#ifndef OSMEXPLORER_SLIPPYMAPTREEVIEW_H
#define OSMEXPLORER_SLIPPYMAPTREEVIEW_H

#include <QTreeView>


class SlippyMapTreeView : public QTreeView
{
    Q_OBJECT
public:
    explicit SlippyMapTreeView(QWidget *parent = nullptr);
protected:
    void wheelEvent(QWheelEvent *event) override;
};


#endif //OSMEXPLORER_SLIPPYMAPTREEVIEW_H
