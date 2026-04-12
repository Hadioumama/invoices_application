#include "logindialog.h"
#include <QLabel>
#include <QVBoxLayout>

LoginDialog::LoginDialog(QWidget *parent) : QDialog(parent)
{
    setWindowTitle("Connexion");
    setMinimumSize(300, 200);

    QLabel *emailLabel = new QLabel("Email :");
    QLabel *passLabel = new QLabel("Mot de passe :");
    emailEdit = new QLineEdit;
    passwordEdit = new QLineEdit;
    passwordEdit->setEchoMode(QLineEdit::Password);
    loginButton = new QPushButton("Se connecter");
    createButton = new QPushButton("Créer un compte");

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(emailLabel);
    layout->addWidget(emailEdit);
    layout->addWidget(passLabel);
    layout->addWidget(passwordEdit);
    layout->addWidget(loginButton);
    layout->addWidget(createButton);

    connect(loginButton, &QPushButton::clicked, this, &LoginDialog::onLoginClicked);
    connect(createButton, &QPushButton::clicked, this, &LoginDialog::onCreateAccountClicked);
}

void LoginDialog::onLoginClicked()
{
    accept();
}

void LoginDialog::onCreateAccountClicked()
{
    emit createAccountRequested();
}

QString LoginDialog::getEmail() const { return emailEdit->text().trimmed(); }
QString LoginDialog::getPassword() const { return passwordEdit->text().trimmed(); }