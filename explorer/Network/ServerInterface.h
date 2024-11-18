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
        bool visible;
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

    struct Attachment
    {
        QUuid id;
        QUuid objectId;
        QString name;
        qint64 size;
        QDateTime created;
        QDateTime updated;
    };

    enum RequestError
    {
        AuthenticationError,
        ItemNotFoundError,
        InvalidRequestError,
        RequestFailedError,
        UserCancelledError
    };

    explicit ServerInterface(QObject *parent = nullptr);

    /**
     * Request an authentication token from the server.
     * @param parent
     * @param onSuccess
     * @param onFailure
     */
    void login(
        QWidget *parent,
        const std::function<void(const QString&)> &onSuccess,
        const std::function<void(ServerInterface::RequestError)> &onFailure);

    /**
     * Get layers and the objects visible within the viewport.
     * @param workspaceId
     * @param rect
     * @param onSuccess
     * @param onFailure
     */
    void getLayersForViewport(
        const QUuid& workspaceId,
        const QRectF& rect,
        const std::function<void(const QList<Layer>&)>& onSuccess,
        const std::function<void(RequestError)>& onFailure);

    /**
     * Request the list of workspaces from the server.
     * @param onSuccess
     * @param onFailure
     */
    void getWorkspaceList(
        const std::function<void(const QList<Workspace>&)>& onSuccess,
        const std::function<void(RequestError)>& onFailure);

    /**
     * Request a new workspace created with the provided parameters.
     * @param workspace
     * @param callback
     */
    void createWorkspace(
        const Workspace &workspace,
        const std::function<void()>& callback);

    /**
     * Save an object to the server.
     * @param object
     * @param onSuccess
     * @param onFailure
     */
    void saveObject(
        const Object &object,
        const std::function<void()>& onSuccess,
        const std::function<void(RequestError)>& onFailure);

    /**
     * Upload an object attachment to the server.
     * @param objectId id of the object to attach the file to
     * @param multiPart the QHttpMultiPart object containing the upload
     * @param uploadStatus called as the file is uploaded to report status
     * @param onSuccess called when the request finishes successfully
     * @param onFailure called when the request fails for any reason
     */
    void uploadAttachment(
        const QUuid& objectId,
        QHttpMultiPart *multiPart,
        const std::function<void(qint64 bytesSent, qint64 bytesTotal)>& uploadStatus,
        const std::function<void(const QUuid&)>& onSuccess,
        const std::function<void(RequestError)>& onFailure);

    void getAttachmentsForObject(
        const QUuid& objectId,
        const std::function<void(const QList<Attachment>&)>& onSuccess,
        const std::function<void(RequestError)>& onFailure
        );

    /**
     * Delete an object from the server.
     * @param id
     * @param callback
     */
    void requestDeleteObject(const QUuid& id, const std::function<void()>& callback);

    void checkAuthorization(const std::function<void()>& callback);

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
