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
        QDateTime created;
        QDateTime updated;
    };

    struct Layer {
        QUuid id;
        QString name;
        QString description;
        int order;
        QList<Object> objects;
        QDateTime created;
        QDateTime updated;
        QColor color;
    };

    struct Workspace {
        QUuid id;
        QString name;
        QString description;
        QDateTime created;
        QDateTime updated;
        QUuid owner;
    };

    explicit ServerInterface(QObject *parent = nullptr);
    void requestAuthToken(const QString& username, const QString& password);
    void requestLayersForViewport(const QUuid& workspaceId, const QRectF& rect);
    void requestWorkspaces();
    void requestCreateWorkspace(
            const ServerInterface::Workspace &workspace,
            const std::function<void()>& callback);
    void requestSaveObject(const Object& object, const std::function<void()>& callback);
    void requestDeleteObject(const QUuid& id, const std::function<void()>& callback);
    const QList<Layer>& layers();
    const QList<Object>& objects();
    const QString& authToken();
    const QList<Workspace>& workspaces();
signals:
    void authTokenRequestSucceeded();
    void authTokenRequestFailed();
    void layersRequestFinished();
    void workspacesRequestFinished();
    void createWorkspaceRequestFinished();
    void createWorkspaceRequestFailed(const QString& errorMessage);
    void saveObjectRequestFinished();
protected slots:
    void onNetworkManagerRequestFinished(QNetworkReply *reply);
    void onLayersRequestFinished();
    void onWorkspacesRequestFinished();

protected:
    QNetworkAccessManager *m_networkManager;
    QNetworkReply *m_layersReply;
    QNetworkReply *m_objectsReply;
    QNetworkReply *m_workspacesReply;
    QList<Layer> m_layers;
    QList<Object> m_objects;
    QList<Workspace> m_workspaces;
    QString m_authToken;

};



#endif //SERVERINTERFACE_H
