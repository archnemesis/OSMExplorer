//
// Created by Robin on 11/5/2024.
//

#include "ServerConnectionDialog.h"
#include "config.h"

#include <QFormLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSettings>
#include <QLabel>
#include <QMessageBox>

#include "mainwindow.h"
#include "Application/ExplorerApplication.h"

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
    m_errorLabel = new QLabel();
    m_errorLabel->setStyleSheet("color:#CC0000;font-weight:bold;");
    m_errorLabel->setVisible(false);

    m_registerLabel = new QLabel("<a href=\"" OSM_REGISTRATION_LINK "\">" + tr("Create account") + "</a>");
    m_registerLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);
    m_registerLabel->setOpenExternalLinks(true);

    m_forgotPasswordLabel = new QLabel("<a href=\"" OSM_FORGOT_PASSWORD_LINK "\">" + tr("Forgot password?") + "</a>");
    m_forgotPasswordLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);
    m_forgotPasswordLabel->setOpenExternalLinks(true);

    if (!username.isEmpty())
        setRemember(true);

    connect(m_loginButton,
            &QPushButton::clicked,
            this,
            &ServerConnectionDialog::doLogin);

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
    form->addWidget(m_errorLabel);
    form->addWidget(m_username);
    form->addWidget(m_password);
    form->addWidget(m_rememberPassword);
    form->addStretch();
    form->addLayout(buttonLayout);
    form->addWidget(m_registerLabel);
    form->addWidget(m_forgotPasswordLabel);

    auto *hlayout = new QHBoxLayout();
    hlayout->addStretch(1);
    hlayout->addLayout(form, 1);
    hlayout->addStretch(1);

    QPixmap splashPixmap(":/images/splash.png");
    auto *splashLabel = new QLabel();
    splashLabel->setPixmap(splashPixmap);
    splashLabel->setMinimumWidth(500);
    splashLabel->setFixedWidth(500);
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

void ServerConnectionDialog::doLogin()
{
    m_errorLabel->setVisible(false);

    ExplorerApplication::serverInterface()->login(
        m_username->text(),
        m_password->text(),
        [this](const QString& token) {
            accept();
        },
        [this](ServerInterface::RequestError error) {
            switch (error) {
            case ServerInterface::RequestFailedError:
            case ServerInterface::InvalidRequestError:
                QMessageBox::critical(
                    this,
                    tr("Server Error"),
                    tr("There was an error performing the request. Please try again."));
                break;
            case ServerInterface::AuthenticationError:
                m_errorLabel->setText(tr("Username and/or password invalid."));
                m_errorLabel->setVisible(true);
                break;
            case ServerInterface::UserCancelledError:
                return;
            default:
                break;
            }

            // todo: show error message
        });
}
