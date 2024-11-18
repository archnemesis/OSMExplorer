//
// Created by robin on 11/17/2024.
//

#include "SlippyMapLayerObjectFilesPropertyPage.h"
#include "Network/ServerInterface.h"
#include "Application/ExplorerApplication.h"
#include "config.h"

#include <QDebug>
#include <QFileDialog>
#include <QHeaderView>
#include <QTreeView>
#include <QPushButton>
#include <QStandardItemModel>
#include <QVBoxLayout>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QHttpMultiPart>
#include <QMessageBox>
#include <QProgressDialog>

#include "Application/ExplorerApplication.h"
#include "Network/ServerConnectionDialog.h"
#include "Weather/ForecastPeriodWidget.h"
#include "Weather/ForecastPeriodWidget.h"


SlippyMapLayerObjectFilesPropertyPage::SlippyMapLayerObjectFilesPropertyPage(
    const SlippyMapLayerObject::Ptr& object, QWidget *parent) :
        SlippyMapLayerObjectPropertyPage(object, parent)
{
    m_interface = ExplorerApplication::serverInterface();
    m_networkManager = new QNetworkAccessManager(this);
}

QString SlippyMapLayerObjectFilesPropertyPage::tabTitle()
{
    return tr("Attachments");
}

void SlippyMapLayerObjectFilesPropertyPage::save()
{

}

void SlippyMapLayerObjectFilesPropertyPage::setupUi()
{
    QStringList columns = {tr("Filename"), tr("Size"), tr("Owner")};
    m_filesModel = new QStandardItemModel(0, 3, this);
    m_filesModel->setHorizontalHeaderLabels(columns);

    m_listView = new QTreeView();
    m_listView->setModel(m_filesModel);
    m_listView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_listView->header()->setVisible(false);

    m_uploadButton = new QPushButton(tr("Add"));
    connect(m_uploadButton,
        &QPushButton::clicked,
        this,
        &SlippyMapLayerObjectFilesPropertyPage::onUploadButtonClicked);
    m_deleteButton = new QPushButton(tr("Delete"));

    auto *buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    buttonLayout->addWidget(m_uploadButton);
    buttonLayout->addWidget(m_deleteButton);

    auto *layout = new QVBoxLayout();
    layout->addWidget(m_listView);
    layout->addLayout(buttonLayout);

    setLayout(layout);
    updateUi();
}

void SlippyMapLayerObjectFilesPropertyPage::updateUi()
{
    m_filesModel->clear();

    m_interface->getAttachmentsForObject(
        m_object->id().toUuid(),
        [this](const QList<ServerInterface::Attachment>& attachments) {
            for (const auto& attachment : attachments) {
                QList<QStandardItem*> items = {
                    new QStandardItem(attachment.name),
                    new QStandardItem(QString("%1 bytes").arg(attachment.size)),
                    new QStandardItem(attachment.created.toString(Qt::ISODate))
                };

                m_filesModel->appendRow(items);
            }
        },
        [this](ServerInterface::RequestError error) {

        });
}

void SlippyMapLayerObjectFilesPropertyPage::onUploadButtonClicked()
{
    QStringList files = QFileDialog::getOpenFileNames(
        this,
        tr("Upload Files"));

    qDebug() << "Got filenames:" << files;

    if (!files.isEmpty()) {
        m_uploadFileNames = files;

        if (m_progressDialog == nullptr)
            m_progressDialog = new QProgressDialog(this);

        m_progressDialog->setWindowTitle(tr("Uploading"));
        m_progressDialog->setMaximum(m_uploadFileNames.size());
        m_progressDialog->setValue(0);
        m_progressDialog->setModal(true);
        m_progressDialog->show();

        qDebug() << "Starting upload queue processing...";

        processUploadQueue();
    }
}

void SlippyMapLayerObjectFilesPropertyPage::onUploadFileFinished(QNetworkReply* reply)
{

}

void SlippyMapLayerObjectFilesPropertyPage::processUploadQueue()
{
    if (!m_uploadFileNames.isEmpty()) {
        auto filename = m_uploadFileNames.first();
        auto *multipart = new QHttpMultiPart(QHttpMultiPart::FormDataType);
        auto *file = new QFile(filename);

        if (!file->open(QIODevice::ReadOnly)) {
            qCritical() << "Failed to open file " << filename;
            return;
        }

        QHttpPart filePart;
        filePart.setHeader(
            QNetworkRequest::ContentTypeHeader,
            "application/octet-stream");
        filePart.setHeader(
            QNetworkRequest::ContentDispositionHeader,
            R"(form-data; name="file"; filename=")" + QFileInfo(filename).fileName() + "\"");
        filePart.setBodyDevice(file);
        file->setParent(multipart);
        multipart->append(filePart);

        m_interface->uploadAttachment(
            m_object->id().toUuid(),
            multipart,
            [this](qint64 bytesSent, qint64 bytesTotal) {
                //qint64 percent = bytesTotal > 0 ? ((bytesSent * 100) / bytesTotal) : 0;
                //m_progressDialog->setValue(static_cast<int>(percent));
            },
            [this](const QUuid& id) {
                m_uploadFileNames.removeFirst();
                processUploadQueue();
            },
            [this](ServerInterface::RequestError error) {
                m_progressDialog->close();
                m_uploadFileNames.clear();
                QMessageBox::critical(
                    this,
                    tr("Upload failed"),
                    tr("Unable to upload file. Please try again."));
            });
    }
    else {
        m_progressDialog->close();
        updateUi();
    }
}
