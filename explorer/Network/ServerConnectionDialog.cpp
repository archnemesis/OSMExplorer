//
// Created by Robin on 11/5/2024.
//

#include "ServerConnectionDialog.h"

#include <QFormLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSettings>
#include <QLabel>

ServerConnectionDialog::ServerConnectionDialog(QWidget *parent) : QDialog(parent)
{
    setFixedWidth(500);

    QString username = QSettings().value("server/username", "").toString();
    QString password = QSettings().value("server/password", "").toString();

    m_username = new QLineEdit();
    m_username->setPlaceholderText(tr("Username"));
    m_username->setText(username);

    m_password = new QLineEdit();
    m_password->setPlaceholderText(tr("Password"));
    m_password->setEchoMode(QLineEdit::Password);
    m_password->setText(password);

    m_loginButton = new QPushButton(tr("Log In"));
    m_cancelButton = new QPushButton(tr("Cancel"));

    m_rememberPassword = new QCheckBox(tr("Remember password"));

    if (!username.isEmpty())
        setRemember(true);

    connect(m_loginButton,
            &QPushButton::clicked,
            this,
            &QDialog::accept);

    connect(m_cancelButton,
            &QPushButton::clicked,
            this,
            &QDialog::reject);

    auto *buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    buttonLayout->addWidget(m_loginButton);
    buttonLayout->addWidget(m_cancelButton);

    auto *form = new QVBoxLayout();
    form->setContentsMargins(10, 10, 10, 10);
    form->addWidget(m_username);
    form->addWidget(m_password);
    form->addWidget(m_rememberPassword);
    form->addStretch();
    form->addLayout(buttonLayout);

    auto *hlayout = new QHBoxLayout();
    hlayout->addStretch(1);
    hlayout->addLayout(form, 4);
    hlayout->addStretch(1);

    QPixmap splashPixmap(":/images/splash.svg");
    auto *splashLabel = new QLabel();
    splashLabel->setPixmap(splashPixmap);
    auto *vlayout = new QVBoxLayout();
    vlayout->setContentsMargins(0, 0, 0, 0);
    vlayout->setSpacing(10);
    vlayout->addWidget(splashLabel);
    vlayout->addLayout(hlayout);

    setLayout(vlayout);
}

QString ServerConnectionDialog::username()
{
    return m_username->text();
}

QString ServerConnectionDialog::password()
{
    return m_password->text();
}

bool ServerConnectionDialog::remember()
{
    return m_rememberPassword->isChecked();
}

void ServerConnectionDialog::setUsername(const QString &username)
{
    m_username->setText(username);
}

void ServerConnectionDialog::setPassword(const QString &password)
{
    m_password->setText(password);
}

void ServerConnectionDialog::setRemember(bool remember)
{
    m_rememberPassword->setChecked(remember);
}
