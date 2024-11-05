//
// Created by robin on 11/4/2024.
//

#ifndef SERVERINTERFACE_H
#define SERVERINTERFACE_H

#include <QJsonObject>
#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QUuid>
#include <SlippyMap/SlippyMapLayer.h>
#include <SlippyMap/SlippyMapLayerObject.h>


class ServerInterface : public QObject {
    Q_OBJECT
public:
    struct Object {
        QUuid id;
        QUuid layerId;
        QString type;
        QString label;
        QString description;
        bool editable;
        QJsonObject data;
        QString geom;
    };

    struct Layer {
        QUuid id;
        QString name;
        QString description;
        int order;
        QList<Object> objects;
    };

    explicit ServerInterface(QObject *parent = nullptr);
    void requestLayersForViewport(const QRectF& rect);
    const QList<Layer>& layers();
    const QList<Object>& objects();
signals:
    void layersRequestFinished();
protected slots:
    void onNetworkManagerRequestFinished(QNetworkReply *reply);
    void onLayersRequestFinished();

protected:
    QNetworkAccessManager *m_networkManager;
    QNetworkReply *m_layersReply;
    QNetworkReply *m_objectsReply;
    QList<Layer> m_layers;
    QList<Object> m_objects;

};



#endif //SERVERINTERFACE_H
