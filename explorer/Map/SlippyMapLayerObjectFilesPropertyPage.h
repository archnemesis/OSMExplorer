//
// Created by robin on 11/17/2024.
//

#ifndef SLIPPYMAPLAYEROBJECTFILESPROPERTYPAGE_H
#define SLIPPYMAPLAYEROBJECTFILESPROPERTYPAGE_H

#include <QNetworkRequest>
#include <SlippyMap/SlippyMapLayerObjectPropertyPage.h>

QT_BEGIN_NAMESPACE
class QTreeView;
class QPushButton;
class QStandardItemModel;
class QNetworkAccessManager;
class QNetworkReply;
class QProgressDialog;
QT_END_NAMESPACE

class ServerInterface;


class SlippyMapLayerObjectFilesPropertyPage : public SlippyMapLayerObjectPropertyPage {
    Q_OBJECT
public:
    explicit SlippyMapLayerObjectFilesPropertyPage(const SlippyMapLayerObject::Ptr& object, QWidget *parent = nullptr);
    QString tabTitle() override;
    void save() override;
    void setupUi() override;
    void updateUi() override;

protected slots:
    void onUploadButtonClicked();
    void onUploadFileFinished(QNetworkReply *reply);

protected:
    void processUploadQueue();

    QTreeView *m_listView = nullptr;
    QStandardItemModel *m_filesModel = nullptr;
    QPushButton *m_uploadButton = nullptr;
    QPushButton *m_deleteButton = nullptr;
    ServerInterface *m_interface = nullptr;
    QNetworkAccessManager *m_networkManager = nullptr;
    QStringList m_uploadFileNames;
    QProgressDialog *m_progressDialog = nullptr;
};



#endif //SLIPPYMAPLAYEROBJECTFILESPROPERTYPAGE_H
