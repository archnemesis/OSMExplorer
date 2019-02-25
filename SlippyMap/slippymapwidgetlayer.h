#ifndef SLIPPYMAPWIDGETLAYER_H
#define SLIPPYMAPWIDGETLAYER_H

#include <QObject>

#if defined EXPORT_SYMBOLS
#define DECLARATION Q_DECL_EXPORT
#else
#define DECLARATION Q_DECL_IMPORT
#endif

class DECLARATION SlippyMapWidgetLayer : public QObject
{
    Q_OBJECT
public:
    SlippyMapWidgetLayer(QObject *parent = nullptr);
    SlippyMapWidgetLayer(const QString& tileUrl, QObject *parent = nullptr);
    void setTileUrl(QString tileUrl);
    void setName(QString name);
    void setDescription(QString description);
    void setZOrder(int zOrder);
    void setVisible(bool visible);
    QString name();
    QString description();
    QString tileUrl();
    QString tileUrlHash();
    int zOrder();
    bool isVisible();

private:
    QString m_name;
    QString m_description;
    QString m_tileUrl;
    int m_zOrder;
    bool m_visible;

signals:
    void zOrderChanged(int zOrder);
    void visibilityChanged(bool visible);
    void nameChanged(const QString& name);
    void descriptionChanged(const QString& description);
    void updated();
};

#endif // SLIPPYMAPWIDGETLAYER_H
