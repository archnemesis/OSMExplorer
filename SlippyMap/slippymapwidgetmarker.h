#ifndef SLIPPYMAPWIDGETMARKER_H
#define SLIPPYMAPWIDGETMARKER_H

#include <QObject>
#include <QColor>
#include <QPointF>
#include <QPoint>
#include <QBrush>
#include <QPen>

class QPainter;

#if defined EXPORT_SYMBOLS
#define DECLARATION Q_DECL_EXPORT
#else
#define DECLARATION Q_DECL_IMPORT
#endif

class DECLARATION  SlippyMapWidgetMarker : public QObject
{
    Q_OBJECT
public:
    SlippyMapWidgetMarker(QPointF position, QObject *parent = nullptr);
    SlippyMapWidgetMarker(QPointF position, QString label, QObject *parent = nullptr);
    void setPosition(QPointF position);
    void setLabel(QString label);
    void setInformation(QString information);
    void setMarkerColor(QColor color);
    void setMovable(bool movable);
    void setEditable(bool editable);
    QPointF position();
    double latitude();
    double longitude();
    QString label();
    QString information();
    QColor color();
    bool isMovable();
    bool isEditable();
    void drawMarker(QPainter *painter, QPoint pos);

signals:
    void labelTextChanged(QString text);
    void informationTextChanged(QString text);
    void positionChanged(QPointF position);
    void changed();

private:
    void initColors();

    QPointF m_position;
    QString m_label;
    QString m_information;
    QColor m_markerColor;
    QBrush m_dotBrush;
    QBrush m_labelBrush;
    QBrush m_labelTextBrush;
    QPen m_dotPen;
    QPen m_labelPen;
    QPen m_labelTextPen;
    bool m_active = false;
    bool m_movable = true;
    bool m_editable = true;
};

#endif // SLIPPYMAPWIDGETMARKER_H
