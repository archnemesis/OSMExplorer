//
// Created by robin on 11/4/2024.
//

#include "ServerInterface.h"

#include <QDebug>
#include <QNetworkRequest>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#include <SlippyMap/SlippyMapLayerManager.h>
#include <SlippyMap/SlippyMapLayer.h>
#include "Application/ExplorerApplication.h"

using namespace SlippyMap;

ServerInterface::ServerInterface(QObject *parent) : QObject(parent)
{
    m_networkManager = new QNetworkAccessManager(this);
}

void ServerInterface::requestLayersForViewport(const QRectF& rect)
{
    m_layers.clear();
    QNetworkRequest request;
    request.setUrl(QUrl("http://localhost:5000/layers"));
    m_layersReply = m_networkManager->get(request);
    connect(m_layersReply,
        &QNetworkReply::finished,
        this,
        &ServerInterface::onLayersRequestFinished);
}

const QList<ServerInterface::Layer>& ServerInterface::layers()
{
    return m_layers;
}

const QList<ServerInterface::Object>& ServerInterface::objects()
{
    return m_objects;
}

void ServerInterface::onNetworkManagerRequestFinished(QNetworkReply *reply)
{
    if (reply->error() != QNetworkReply::NoError) {
        qCritical() << "Request failed:" << reply->errorString();
        return;
    }
}

void ServerInterface::onLayersRequestFinished()
{
    if (m_layersReply->error() != QNetworkReply::NoError) {
        qCritical() << "Request failed:" << m_layersReply->errorString();
        return;
    }

    qDebug() << "Request finished";

    QByteArray replyData = m_layersReply->readAll();
    QJsonDocument jsonDoc = QJsonDocument::fromJson(replyData);
    QJsonObject root = jsonDoc.object();
    QJsonArray layersArray = root["layers"].toArray();

    for (int i = 0; i < layersArray.size(); i++) {
        QJsonObject layerObject = layersArray[i].toObject();
        QString layerId = layerObject["id"].toString();
        QString layerName = layerObject["name"].toString();
        QString layerDesc = layerObject["description"].toString();
        int order = layerObject["order"].toInt();

        qDebug() << "Processing layer:" << layerId << layerName;

        Layer layer;
        layer.id = layerId;
        layer.name = layerName;
        layer.description = layerDesc;
        layer.order = order;

        if (!layerObject.contains("objects")) continue;

        QJsonArray objectsArray = layerObject["objects"].toArray();

        for (int i = 0; i < objectsArray.size(); i++) {
            QJsonObject object = objectsArray[i].toObject();
            QString objectId = object["id"].toString();
            QString layerId = object["layer_id"].toString();
            QString type = object["type"].toString();
            QString label = object["label"].toString();
            QString description = object["description"].toString();
            bool editable = object["editable"].toBool();
            QJsonObject data = object["data"].toObject();
            QString geom = object["geom"].toString();

            qDebug() << "Processing object" << objectId << label;

            Object obj;
            obj.id = objectId;
            obj.layerId = layerId;
            obj.type = type;
            obj.label = label;
            obj.description = description;
            obj.editable = editable;
            obj.data = data;
            obj.geom = geom;

            layer.objects.append(obj);
        }

        m_layers.append(layer);
    }

    emit layersRequestFinished();
}
