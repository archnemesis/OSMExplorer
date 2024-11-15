//
// Created by Robin on 11/5/2024.
//

#ifndef OSMEXPLORER_OEDOCKWIDGET_H
#define OSMEXPLORER_OEDOCKWIDGET_H

#include <QDockWidget>


class OEDockWidget : public QDockWidget
{
    Q_OBJECT
public:
    explicit OEDockWidget(QWidget *parent = nullptr);
protected:
    void closeEvent(QCloseEvent *event) override;
signals:
    void closed();
};


#endif //OSMEXPLORER_OEDOCKWIDGET_H
