//
// Created by robin on 11/18/2024.
//

#ifndef OBJECTBROWSERDIALOG_H
#define OBJECTBROWSERDIALOG_H

#include <QStandardItemModel>
#include <QWidget>
#include <QUuid>


class ServerInterface;
class QLineEdit;
class QTreeView;
class QComboBox;

class ObjectBrowserDialog : public QWidget {
Q_OBJECT

public:
    explicit ObjectBrowserDialog(const QUuid& workspaceId, QWidget *parent = nullptr);
    void setupUi();
    void refresh();

private:
    ServerInterface *m_serverInterface;
    QLineEdit *m_searchLineEdit;
    QTreeView *m_treeView;
    QComboBox *m_ownerFilterComboBox;
    QComboBox *m_typeFilterComboBox;
    QComboBox *m_sortComboBox;
    QUuid m_workspaceId;
    QStandardItemModel *m_model;
};


#endif //OBJECTBROWSERDIALOG_H
