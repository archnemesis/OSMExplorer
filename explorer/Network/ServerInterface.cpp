//
// Created by robin on 11/4/2024.
//

#include "ServerInterface.h"
#include "config.h"

#include <QDebug>
#include <QNetworkRequest>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#include <SlippyMap/SlippyMapLayerManager.h>
#include <SlippyMap/SlippyMapLayer.h>
#include <QMessageBox>
#include "Application/ExplorerApplication.h"

using namespace SlippyMap;

ServerInterface::ServerInterface(QObject *parent) : QObject(parent)
{
    m_networkManager = new QNetworkAccessManager(this);
}

void ServerInterface::requestLayersForViewport(const QUuid& workspaceId, const QRectF& rect)
{
    m_layers.clear();
    QNetworkRequest request;
    QString url = QString(OSM_SERVER_HOST "/layers?x=%1&y=%2&w=%3&h=%4&workspace=%5")
            .arg(rect.x(), 0, 'f', 7)
            .arg(rect.y(), 0, 'f', 7)
            .arg(rect.width(), 0, 'f', 7)
            .arg(rect.width(), 0, 'f', 7)
            .arg(workspaceId.toString());
    request.setUrl(QUrl(url));
    request.setRawHeader("Authorization",
                         QString("Bearer %1").arg(m_authToken).toUtf8());
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
    m_layers.clear();

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
        QString color = layerObject["color"].toString();

        qDebug() << "Processing layer:" << layerId << layerName;

        Layer layer;
        layer.id = QUuid(layerId);
        layer.name = layerName;
        layer.description = layerDesc;
        layer.order = order;
        layer.color = QColor(color);

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

void ServerInterface::requestAuthToken(const QString &username, const QString &password)
{
    QNetworkRequest request;
    request.setUrl(QUrl(OSM_SERVER_HOST "/login"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QJsonObject auth;
    auth["username"] = username;
    auth["password"] = password;
    QJsonDocument requestDoc(auth);
    QByteArray requestJson = requestDoc.toJson();

    QNetworkReply *reply = m_networkManager->post(request, requestJson);
    connect(reply,
            &QNetworkReply::finished,
            [this, reply]() {
                auto statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
                if (statusCode.toInt() != 200) {
                    qCritical() << "Unable to get authentication token from server";
                    return;
                }

                QByteArray replyJson = reply->readAll();
                QJsonDocument replyDoc = QJsonDocument::fromJson(replyJson);
                QJsonObject root = replyDoc.object();
                QString status = root["status"].toString("fail");
                QString token = root["token"].toString();

                if (status != "ok") {
                    qCritical() << "Server returned status" << status;
                    qCritical() << "Reason:" << root["reason"].toString();
                    emit authTokenRequestFailed();
                    return;
                }

                m_authToken = token;
                emit authTokenRequestSucceeded();
    });
}

const QString &ServerInterface::authToken()
{
    return m_authToken;
}

void ServerInterface::requestWorkspaces()
{
    QNetworkRequest request;
    request.setUrl(QUrl(OSM_SERVER_HOST "/workspaces"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization",
                         QString("Bearer %1").arg(m_authToken).toUtf8());

    m_workspacesReply = m_networkManager->get(request);
    connect(m_workspacesReply,
            &QNetworkReply::finished,
            this,
            &ServerInterface::onWorkspacesRequestFinished);
}

void ServerInterface::onWorkspacesRequestFinished()
{
    m_workspaces.clear();

    if (m_workspacesReply->error() != QNetworkReply::NoError) {
        qCritical() << "Request failed:" << m_workspacesReply->errorString();
        return;
    }

    qDebug() << "Request finished";

    QByteArray replyData = m_workspacesReply->readAll();
    QJsonDocument jsonDoc = QJsonDocument::fromJson(replyData);
    QJsonObject root = jsonDoc.object();
    QJsonArray workspacesArray = root["workspaces"].toArray();

    for (int i = 0; i < workspacesArray.count(); i++) {
        QJsonObject workspaceObj = workspacesArray[i].toObject();
        QUuid id = QUuid(workspaceObj["id"].toString());
        QString name = workspaceObj["name"].toString();
        QString description = workspaceObj["description"].toString();
        QUuid owner = QUuid(workspaceObj["owner"].toString());
        QDateTime created = QDateTime::fromString(
                workspaceObj["created"].toString(),
                "yyyy-MM-ddThh:mm:ss.zzzZ");

        QDateTime updated;
        if (workspaceObj["updated"].toString().length())
            updated = QDateTime::fromString(
                    workspaceObj["updated"].toString(),
                    "yyyy-MM-ddThh:mm:ss.zzzZ");

        Workspace workspace;
        workspace.id = id;
        workspace.owner = owner;
        workspace.name = name;
        workspace.description = description;
        workspace.created = created;
        workspace.updated = updated;

        m_workspaces.append(workspace);
    }

    emit workspacesRequestFinished();
}

const QList<ServerInterface::Workspace> &ServerInterface::workspaces()
{
    return m_workspaces;
}

void ServerInterface::requestSaveObject(const ServerInterface::Object &object, const std::function<void()> &callback)
{
    QJsonObject json;

    json["id"] = object.id.toString();
    json["layer_id"] = object.layerId.toString();
    json["label"] = object.label;
    json["description"] = object.description;
    json["type"] = object.type;
    json["geom"] = object.geom;
    json["data"] = object.data;

    QJsonDocument doc(json);
    QByteArray data = doc.toJson();

    QNetworkRequest request;
    request.setUrl(QUrl(OSM_SERVER_HOST "/objects"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization",
                         QString("Bearer %1").arg(m_authToken).toUtf8());

    QNetworkReply *reply = m_networkManager->put(request, data);
    connect(reply,
            &QNetworkReply::finished,
            [this, reply]() {
                emit saveObjectRequestFinished();
            });
}

void ServerInterface::requestCreateWorkspace(
        const ServerInterface::Workspace &workspace,
        const std::function<void()>& callback)
{
    QJsonObject root;
    root["id"] = workspace.id.toString();
    root["name"] = workspace.name;
    root["description"] = workspace.description;

    QJsonDocument doc(root);
    QByteArray data = doc.toJson();

    QNetworkRequest request;
    request.setUrl(QUrl(OSM_SERVER_HOST "/workspaces"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization",
                         QString("Bearer %1").arg(m_authToken).toUtf8());

    QNetworkReply *reply = m_networkManager->put(request, data);
    connect(reply,
            &QNetworkReply::finished,
            [this, reply]() {
                emit createWorkspaceRequestFinished();
            });
}

void ServerInterface::requestDeleteObject(const QUuid &id, const std::function<void()> &callback)
{

}
