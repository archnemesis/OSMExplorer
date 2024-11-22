//
// Created by robin on 11/4/2024.
//

#include "ServerInterface.h"

#include <complex>

#include "config.h"

#include <QDebug>
#include <QHttpMultiPart>
#include <QNetworkRequest>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#include <SlippyMap/SlippyMapLayerManager.h>
#include <SlippyMap/SlippyMapLayer.h>
#include <QMessageBox>
#include <boost/mpl/arg.hpp>

#include "ServerConnectionDialog.h"
#include "Application/ExplorerApplication.h"

using namespace SlippyMap;

ServerInterface::ServerInterface(QObject *parent) : QObject(parent)
{
    m_networkManager = new QNetworkAccessManager(this);
}

void ServerInterface::getLayersForViewport(
    const QUuid& workspaceId,
    const QRectF& rect,
    const std::function<void(const QList<Layer>&)>& onSuccess,
    const std::function<void(RequestError)>& onFailure)
{
    m_layers.clear();
    QNetworkRequest request;

    QString url;
    if (rect.isNull()) {
        url = QString(OSM_SERVER_HOST "/layers?workspace=%1")
                .arg(workspaceId.toString());
    }
    else {
        url = QString(OSM_SERVER_HOST "/layers?x=%1&y=%2&w=%3&h=%4&workspace=%5")
                .arg(rect.x(), 0, 'f', 7)
                .arg(rect.y(), 0, 'f', 7)
                .arg(rect.width(), 0, 'f', 7)
                .arg(rect.width(), 0, 'f', 7)
                .arg(workspaceId.toString());
    }

    request.setUrl(QUrl(url));
    request.setRawHeader("Authorization",
                         QString("Bearer %1").arg(m_authToken).toUtf8());
    auto *reply = m_networkManager->get(request);
    connect(reply,
        &QNetworkReply::finished,
        [this, reply, onSuccess, onFailure]() {
            if (reply->error() != QNetworkReply::NoError \
                && reply->error() != QNetworkReply::ProtocolInvalidOperationError) {
                qCritical() << "Unable to save object";
                onFailure(RequestFailedError);
                return;
            }

            int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

            if (statusCode == 401) {
                onFailure(AuthenticationError);
                return;
            }

            if (statusCode != 200) {
                onFailure(RequestFailedError);
                return;
            }

            QList<Layer> layers;
            QByteArray replyData = reply->readAll();
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

                QJsonObject creatorObject = layerObject["creator"].toObject();
                layer.creator.id = QUuid(creatorObject["id"].toString());
                layer.creator.username = creatorObject["username"].toString();
                layer.creator.email = creatorObject["email"].toString();
                layer.creator.created = QDateTime::fromString(
                            creatorObject["created"].toString(),
                            "yyyy-MM-ddThh:mm:ss.zzzZ");

                QString updated = creatorObject["updated"].toString();
                if (!updated.isEmpty()) {
                    layer.creator.updated = QDateTime::fromString(
                            creatorObject["updated"].toString(),
                            "yyyy-MM-ddThh:mm:ss.zzzZ");
                }

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

                    QJsonObject objCreatorObject = object["creator"].toObject();
                    obj.creator.id = QUuid(objCreatorObject["id"].toString());
                    obj.creator.username = objCreatorObject["username"].toString();
                    obj.creator.email = objCreatorObject["email"].toString();
                    obj.creator.created = QDateTime::fromString(
                                objCreatorObject["created"].toString(),
                                "yyyy-MM-ddThh:mm:ss.zzzZ");

                    layer.objects.append(obj);
                }

                layers.append(layer);
            }

            onSuccess(layers);
        });
}

const QList<ServerInterface::Layer>& ServerInterface::layers()
{
    return m_layers;
}

const QList<ServerInterface::Object>& ServerInterface::objects()
{
    return m_objects;
}

void ServerInterface::login(
    const QString& username,
    const QString& password,
    const std::function<void(const QString&)>& onSuccess,
    const std::function<void(ServerInterface::RequestError)>& onFailure)
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
        [this, reply, onSuccess, onFailure]() {
            if (reply->error() != QNetworkReply::NoError \
                && reply->error() != QNetworkReply::ProtocolInvalidOperationError \
                && reply->error() != QNetworkReply::AuthenticationRequiredError) {
                qCritical() << "Login request failed:" << reply->errorString();
                onFailure(RequestFailedError);
                return;
            }

            int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

            if (statusCode == 401) {
                onFailure(AuthenticationError);
                return;
            }

            if (statusCode != 200) {
                onFailure(RequestFailedError);
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
            onSuccess(token);
        });
}

const QString &ServerInterface::authToken()
{
    return m_authToken;
}

void ServerInterface::getWorkspaceList(
    const std::function<void(const QList<Workspace>&)>& onSuccess,
    const std::function<void(RequestError)>& onFailure)
{
    QNetworkRequest request;
    request.setUrl(QUrl(OSM_SERVER_HOST "/workspaces"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization",
                         QString("Bearer %1").arg(m_authToken).toUtf8());

    auto *reply = m_networkManager->get(request);
    connect(reply,
        &QNetworkReply::finished,
        [this, reply, onSuccess, onFailure]() {
            if (reply->error() != QNetworkReply::NoError \
                    && reply->error() != QNetworkReply::ProtocolInvalidOperationError) {
                    qCritical() << "Unable to save object";
                    onFailure(RequestFailedError);
                    return;
                }

                int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

                if (statusCode == 401) {
                    onFailure(AuthenticationError);
                    return;
                }

                if (statusCode != 200) {
                    onFailure(RequestFailedError);
                    return;
                }

                QByteArray replyData = reply->readAll();
                QJsonDocument jsonDoc = QJsonDocument::fromJson(replyData);
                QJsonObject root = jsonDoc.object();
                QJsonArray workspacesArray = root["workspaces"].toArray();

                QList<Workspace> workspaces;

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

                    workspaces.append(workspace);
                }

                onSuccess(workspaces);
        });
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

void ServerInterface::saveObject(
    const ServerInterface::Object &object,
    const std::function<void()>& onSuccess,
    const std::function<void(RequestError)>& onFailure)
{
    QJsonObject json;

    json["id"] = object.id.toString();
    json["layer_id"] = object.layerId.toString();
    json["label"] = object.label;
    json["description"] = object.description;
    json["type"] = object.type;
    json["visible"] = object.visible;
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
            [this, reply, onSuccess, onFailure]() {
                    if (reply->error() != QNetworkReply::NoError \
                        && reply->error() != QNetworkReply::ProtocolInvalidOperationError) {
                        qCritical() << "Unable to save object";
                        onFailure(RequestFailedError);
                        return;
                    }

                    int statusCode = reply->attribute(
                        QNetworkRequest::HttpStatusCodeAttribute).toInt();

                    if (statusCode == 401) {
                        onFailure(AuthenticationError);
                        return;
                    }

                    if (statusCode != 200) {
                        onFailure(RequestFailedError);
                        return;
                    }

                    onSuccess();
            });
}

void ServerInterface::deleteObject(
    const QUuid& objectId,
    const std::function<void()>& onSuccess,
    const std::function<void(RequestError)>& onFailure)
{
    QString url = QString(OSM_SERVER_HOST "/objects/%1")
        .arg(objectId.toString());
    QNetworkRequest request;
    request.setUrl(QUrl(url));
    request.setRawHeader("Authorization",
        QString("Bearer %1").arg(authToken()).toUtf8());

    auto *reply = m_networkManager->deleteResource(request);

    connect(reply,
        &QNetworkReply::finished,
        [this, reply, onSuccess, onFailure]() {
            int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

            if (reply->error() != QNetworkReply::NoError \
                && reply->error() != QNetworkReply::ProtocolInvalidOperationError) {
                qCritical() << "Unable to save object" << reply->error() << reply->errorString();
                onFailure(RequestFailedError);
                return;
            }

            if (statusCode == 401) {
                onFailure(AuthenticationError);
                return;
            }

            QByteArray responseData = reply->readAll();
            QJsonDocument jsonDoc = QJsonDocument::fromJson(responseData);
            QJsonObject root = jsonDoc.object();

            QString status = root["status"].toString();
            QString fileId = root["id"].toString();

            if (status != "ok") {
                qDebug() << "Server rejected request:" << root["message"];
                onFailure(InvalidRequestError);
            }

            onSuccess();
        });
}

void ServerInterface::deleteLayer(
    const QUuid& layerId,
    const std::function<void()>& onSuccess,
    const std::function<void(RequestError)>& onFailure)
{
    QString url = QString(OSM_SERVER_HOST "/layers/%1")
        .arg(layerId.toString().mid(1, layerId.toString().length() - 2));
    QNetworkRequest request;
    request.setUrl(QUrl(url));
    request.setRawHeader("Authorization",
        QString("Bearer %1").arg(authToken()).toUtf8());

    auto *reply = m_networkManager->deleteResource(request);

    connect(reply,
        &QNetworkReply::finished,
        [this, reply, onSuccess, onFailure]() {
            const int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

            if (reply->error() != QNetworkReply::NoError \
                && reply->error() != QNetworkReply::ProtocolInvalidOperationError) {
                qCritical() << "Unable to save object" << reply->error() << reply->errorString();
                onFailure(RequestFailedError);
                return;
            }

            if (statusCode == 401) {
                onFailure(AuthenticationError);
                return;
            }

            QByteArray responseData = reply->readAll();
            QJsonDocument jsonDoc = QJsonDocument::fromJson(responseData);
            QJsonObject root = jsonDoc.object();

            QString status = root["status"].toString();

            if (status != "ok") {
                qDebug() << "Server rejected request:" << root["message"];
                onFailure(InvalidRequestError);
            }

            onSuccess();
        });
}

void ServerInterface::uploadAttachment(
    const QUuid& objectId,
    QHttpMultiPart *multiPart,
    const std::function<void(qint64 bytesSent, qint64 bytesTotal)>& uploadStatus,
    const std::function<void(const QUuid&)>& onSuccess,
    const std::function<void(RequestError)>& onFailure)
{
    QString url = QString(OSM_SERVER_HOST "/objects/%1/files")
        .arg(objectId.toString().mid(1, objectId.toString().length() - 2));
    QNetworkRequest request;
    request.setUrl(QUrl(url));
    request.setRawHeader("Authorization",
        QString("Bearer %1").arg(authToken()).toUtf8());

    auto *reply = m_networkManager->post(request, multiPart);

    connect(reply,
        &QNetworkReply::uploadProgress,
        uploadStatus);

    connect(reply,
        &QNetworkReply::finished,
        [this, reply, onSuccess, onFailure]() {
            int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

            if (reply->error() != QNetworkReply::NoError \
                && reply->error() != QNetworkReply::ProtocolInvalidOperationError) {
                qCritical() << "Unable to save object" << reply->error() << reply->errorString();
                onFailure(RequestFailedError);
                return;
            }

            if (statusCode == 401) {
                onFailure(AuthenticationError);
                return;
            }

            QByteArray responseData = reply->readAll();
            QJsonDocument jsonDoc = QJsonDocument::fromJson(responseData);
            QJsonObject root = jsonDoc.object();

            QString status = root["status"].toString();
            QString fileId = root["id"].toString();

            if (status != "ok") {
                qDebug() << "Server rejected request:" << root["message"];
                onFailure(InvalidRequestError);
            }

            onSuccess(QUuid::fromString(fileId));
        });
}

void ServerInterface::getAttachmentsForObject(const QUuid& objectId,
    const std::function<void(const QList<Attachment>&)>& onSuccess,
    const std::function<void(RequestError)>& onFailure)
{
    QString url = QString(OSM_SERVER_HOST "/objects/%1/files")
        .arg(objectId.toString().mid(1, objectId.toString().length() - 2));
    QNetworkRequest request;
    request.setUrl(QUrl(url));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization",
                         QString("Bearer %1").arg(m_authToken).toUtf8());

    auto *reply = m_networkManager->get(request);
    connect(reply,
        &QNetworkReply::finished,
        [this, reply, onSuccess, onFailure]() {
            int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

            if (reply->error() != QNetworkReply::NoError \
                && reply->error() != QNetworkReply::ProtocolInvalidOperationError) {
                qCritical() << "Unable to save object" << reply->error() << reply->errorString();
                onFailure(RequestFailedError);
                return;
            }

            if (statusCode == 401) {
                onFailure(AuthenticationError);
                return;
            }

            QList<Attachment> attachments;
            QByteArray responseData = reply->readAll();
            QJsonDocument jsonDoc = QJsonDocument::fromJson(responseData);
            QJsonObject root = jsonDoc.object();
            QString status = root["status"].toString();

            if (status != "ok") {
                qDebug() << "Server rejected request:" << root["message"];
                onFailure(InvalidRequestError);
                return;
            }

            QJsonArray filesArray = root["files"].toArray();
            for (int i = 0; i < filesArray.size(); i++) {
                QJsonObject fileObject = filesArray[i].toObject();
                Attachment attachment;
                attachment.id = fileObject["id"].toString();
                attachment.objectId = fileObject["object_id"].toString();
                attachment.name = fileObject["name"].toString();
                attachment.size = fileObject["size"].toInt();
                attachment.created = QDateTime::fromString(
                                fileObject["created"].toString(),
                                "yyyy-MM-ddThh:mm:ss.zzzZ");

                QString updated = fileObject["updated"].toString();
                if (!updated.isEmpty())
                    attachment.updated = QDateTime::fromString(
                                fileObject["updated"].toString(),
                                "yyyy-MM-ddThh:mm:ss.zzzZ");

                attachments.append(attachment);
            }

            onSuccess(attachments);
        });
}

void ServerInterface::getObjectsForWorkspace(
    const QUuid& workspaceId,
    const std::function<void(const QList<Layer>&)>& onSuccess,
    const std::function<void(RequestError)>& onFailure)
{
    QString url = QString(OSM_SERVER_HOST "/layers?workspace=%1")
        .arg(workspaceId.toString());
    QNetworkRequest request;
    request.setUrl(QUrl(url));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization",
                         QString("Bearer %1").arg(m_authToken).toUtf8());

    auto *reply = m_networkManager->get(request);
    connect(reply,
        &QNetworkReply::finished,
        [this, reply, onSuccess, onFailure]() {
            int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

            if (reply->error() != QNetworkReply::NoError \
                && reply->error() != QNetworkReply::ProtocolInvalidOperationError) {
                qCritical() << "Unable to save object" << reply->error() << reply->errorString();
                onFailure(RequestFailedError);
                return;
            }

            if (statusCode == 401) {
                onFailure(AuthenticationError);
                return;
            }

            QList<Attachment> attachments;
            QByteArray responseData = reply->readAll();
            QJsonDocument jsonDoc = QJsonDocument::fromJson(responseData);
            QJsonObject root = jsonDoc.object();
            QString status = root["status"].toString();

            if (status != "ok") {
                qDebug() << "Server rejected request:" << root["message"];
                onFailure(InvalidRequestError);
                return;
            }

            QJsonArray layersArray = root["layers"].toArray();
            for (int i = 0; i < layersArray.size(); i++) {
                QJsonObject layerObject = layersArray[i].toObject();
                Layer layer;
                layer.id = QUuid(layerObject["id"].toString());
                layer.name = layerObject["name"].toString();
                layer.description = layerObject["description"].toString();
                layer.color = layerObject["color"].toInt();
                layer.order = layerObject["order"].toInt();
                layer.created = QDateTime::fromString(
                                layerObject["created"].toString(),
                                "yyyy-MM-ddThh:mm:ss.zzzZ");

                QString updated = layerObject["updated"].toString();
                if (!updated.isEmpty())
                    layer.updated = QDateTime::fromString(
                        updated,
                        "yyyy-MM-ddThh:mm:ss.zzzZ");

                QJsonArray objectsArray = layerObject["objects"].toArray();
                for (int j = 0; j < objectsArray.size(); j++) {

                }
            }
        });

}

void ServerInterface::createWorkspace(
        const Workspace &workspace,
        const std::function<void(void)>& onSuccess,
        const std::function<void(RequestError)>& onFailure)
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
            [this, reply, onSuccess, onFailure]() {
                int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

                if (reply->error() != QNetworkReply::NoError \
                    && reply->error() != QNetworkReply::ProtocolInvalidOperationError) {
                    qCritical() << "Unable to save object" << reply->error() << reply->errorString();
                    onFailure(RequestFailedError);
                    return;
                }

                if (statusCode == 401) {
                    onFailure(AuthenticationError);
                    return;
                }

                onSuccess();
            });
}

void ServerInterface::saveLayer(
    const Layer& layer,
    const std::function<void()>& onSuccess,
    const std::function<void(RequestError)>& onFailure)
{
    QJsonObject json;
    json["id"] = layer.id.toString();
    json["workspace_id"] = layer.workspaceId.toString();
    json["name"] = layer.name;
    json["description"] = layer.description;
    json["order"] = layer.order;
    json["color"] = layer.color.name(QColor::HexArgb);

    QJsonDocument jsonDocument(json);
    QByteArray jsonData = jsonDocument.toJson();

    QNetworkRequest request;
    request.setUrl(QUrl(OSM_SERVER_HOST "/layers"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization",
                         QString("Bearer %1").arg(authToken()).toUtf8());

    auto *reply = m_networkManager->put(request, jsonData);
    connect(reply,
        &QNetworkReply::finished,
        [this, reply, onSuccess, onFailure]() {
            int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

            if (reply->error() != QNetworkReply::NoError \
                && reply->error() != QNetworkReply::ProtocolInvalidOperationError) {
                qCritical() << "Unable to save object" << reply->error() << reply->errorString();
                onFailure(RequestFailedError);
                return;
            }

            if (statusCode == 401) {
                onFailure(AuthenticationError);
                return;
            }

            QByteArray responseData = reply->readAll();
            QJsonDocument jsonDoc = QJsonDocument::fromJson(responseData);
            QJsonObject root = jsonDoc.object();
            QString status = root["status"].toString();

            if (status != "ok") {
                qDebug() << "Server rejected request:" << root["message"];
                onFailure(InvalidRequestError);
                return;
            }

            onSuccess();
        });

}

void ServerInterface::requestDeleteObject(const QUuid &id, const std::function<void()> &callback)
{

}

void ServerInterface::checkAuthorization(const std::function<void()>& callback)
{

}
