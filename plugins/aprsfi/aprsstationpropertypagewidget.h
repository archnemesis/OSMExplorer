#ifndef APRSSTATIONPROPERTYPAGEWIDGET_H
#define APRSSTATIONPROPERTYPAGEWIDGET_H

#include <QWidget>
#include <QListWidgetItem>

namespace Ui {
class AprsStationPropertyPageWidget;
}

class AprsStationPropertyPageWidget : public QWidget
{
    Q_OBJECT

public:
    explicit AprsStationPropertyPageWidget(QWidget *parent = nullptr);
    ~AprsStationPropertyPageWidget();

private:
    Ui::AprsStationPropertyPageWidget *ui;

    QListWidgetItem *m_stationNameItem;
    QListWidgetItem *m_typeItem;
    QListWidgetItem *m_timestampItem;
};

#endif // APRSSTATIONPROPERTYPAGEWIDGET_H
