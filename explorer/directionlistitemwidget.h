#ifndef DIRECTIONLISTITEMWIDGET_H
#define DIRECTIONLISTITEMWIDGET_H

#include <QWidget>

namespace Ui {
class DirectionListItemWidget;
}

class DirectionListItemWidget : public QWidget
{
    Q_OBJECT

public:
    explicit DirectionListItemWidget(QWidget *parent = nullptr);
    ~DirectionListItemWidget();

    void setInstruction(QString instruction);
    void setDuration(double duration);
    void setDistance(double distance);

    QString instruction();
    double duration();
    double distance();

private:
    Ui::DirectionListItemWidget *ui;
    QString m_instruction;
    double m_duration;
    double m_distance;
};

#endif // DIRECTIONLISTITEMWIDGET_H
