#ifndef SLIPPYMAPWIDGETMARKER_H
#define SLIPPYMAPWIDGETMARKER_H

#include <QObject>
#include <QColor>
#include <QPointF>

class SlippyMapWidgetMarker : public QObject
{
    Q_OBJECT
public:
    SlippyMapWidgetMarker(QPointF position, QString label, QObject *parent = nullptr);
    void setPosition(QPointF position);
    void setLabel(QString label);
    void setInformation(QString information);
    void setColor(QColor color);
    double latitude();
    double longitude();
    QString label();
    QString information();
    QColor color();

signals:
    void labelTextChanged(QString text);
    void informationTextChanged(QString text);
    void positionChanged(QPointF position);
    void changed();

private:
    QPointF m_position;
    QString m_label;
    QString m_information;
    QColor m_color;

signals:

public slots:
};

#endif // SLIPPYMAPWIDGETMARKER_H
