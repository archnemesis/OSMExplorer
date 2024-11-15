//
// Created by Robin on 11/5/2024.
//

#ifndef OSMEXPLORER_WORKSPACESELECTIONDIALOG_H
#define OSMEXPLORER_WORKSPACESELECTIONDIALOG_H

#include <QDialog>
#include <QUuid>
#include <QMap>


QT_BEGIN_NAMESPACE
namespace Ui
{
    class WorkspaceSelectionDialog;
}
QT_END_NAMESPACE

class WorkspaceSelectionDialog : public QDialog
{
Q_OBJECT

public:
    explicit WorkspaceSelectionDialog(QWidget *parent = nullptr);

    ~WorkspaceSelectionDialog() override;

    bool createNew() const;
    QString newWorkspaceName() const;
    QString newWorkspaceDescription() const;
    QUuid existingWorkspaceId() const;
    void setWorkspaceList(const QMap<QUuid,QString>& workspaceList);

private:
    Ui::WorkspaceSelectionDialog *ui;
    QMap<QUuid,QString> m_workspaceList;
};


#endif //OSMEXPLORER_WORKSPACESELECTIONDIALOG_H
