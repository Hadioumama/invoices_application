#include "logindialog.h"
#include <QLabel>
#include <QVBoxLayout>
#include <QMessageBox>
#include <QCryptographicHash>
#include <QSqlQuery>
#include "views/adminwindow.h"
#include "views/clientwindow.h"

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

    // ✅ Connecter au slot onLogin (logique complète)
    connect(loginButton, &QPushButton::clicked, this, &LoginDialog::onLogin);
    connect(createButton, &QPushButton::clicked, this, &LoginDialog::onCreateAccountClicked);
}

void LoginDialog::onLogin()
{
    QString email = emailEdit->text().trimmed();
    QString pwd   = passwordEdit->text();

    if (email.isEmpty() || pwd.isEmpty()) {
        QMessageBox::warning(this, "Erreur", "Veuillez remplir tous les champs.");
        return;
    }

    // Hash du mot de passe
    QString hashedPwd = QCryptographicHash::hash(
        pwd.toUtf8(),
        QCryptographicHash::Sha256).toHex();

    QSqlQuery q;
    q.prepare(
        "SELECT id, role FROM clients "
        "WHERE email = ? AND "
        "(mot_de_passe = ? OR mot_de_passe = ?)");
    q.addBindValue(email);
    q.addBindValue(hashedPwd);
    q.addBindValue(pwd); // mot de passe en clair (admin123)

    if (!q.exec() || !q.next()) {
        QMessageBox::warning(this, "Erreur",
            "Email ou mot de passe incorrect.");
        return;
    }

    int    clientId = q.value(0).toInt();
    QString role    = q.value(1).toString();

    if (role == "admin") {
        AdminWindow *w = new AdminWindow;
        w->show();
        accept();
    } else {
        ClientWindow *w = new ClientWindow(clientId);
        w->show();
        accept();
    }
}

void LoginDialog::onCreateAccountClicked()
{
    emit createAccountRequested();
}

QString LoginDialog::getEmail() const { return emailEdit->text().trimmed(); }
QString LoginDialog::getPassword() const { return passwordEdit->text().trimmed(); }