#ifndef MARKERLISTITEMWIDGET_H
#define MARKERLISTITEMWIDGET_H

#include <QWidget>

namespace Ui {
class MarkerListItemWidget;
}

class MarkerListItemWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MarkerListItemWidget(QWidget *parent = nullptr);
    ~MarkerListItemWidget();

    void setName(QString name);
    void setDescription(QString description);
    void setLatitude(double latitude);
    void setLongitude(double longitude);

signals:
    void markerMapButtonPressed(double latitude, double longitude);

protected slots:
    void centerMapButtonPressed();

private:
    Ui::MarkerListItemWidget *ui;

    QString m_name;
    QString m_description;
    double m_latitude;
    double m_longitude;
};

#endif // MARKERLISTITEMWIDGET_H
