#include <QApplication>
#include <QMessageBox>
#include <QDebug>
#include <QCryptographicHash>
#include "database/database.h"
#include "dialogs/logindialog.h"
#include "dialogs/registerdialog.h"
#include "mainwindow.h"
#include "views/adminwindow.h"
#include "views/clientwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    if (!Database::instance().connect("facturation.db"))
    {
        return -1;
    }
    Database::instance().initializeTables();

    bool authenticated = false;
    QString role;
    QString email;

    while (!authenticated)
    {
        LoginDialog login;
        // On connecte le signal pour ouvrir l'inscription
        QObject::connect(&login, &LoginDialog::createAccountRequested, [&]()
                         {
            RegisterDialog registerDlg;
            if (registerDlg.exec() == QDialog::Accepted) {
                QMessageBox::information(nullptr, "Succès", "Compte créé. Veuillez vous connecter.");
            } });

        if (login.exec() != QDialog::Accepted)
        {
            return 0; // L'utilisateur a fermé la fenêtre
        }

       email = login.getEmail();
QString password = login.getPassword(); // mot de passe en clair

QSqlQuery query;
query.prepare("SELECT role FROM clients WHERE email = :email AND mot_de_passe = :mdp");
query.bindValue(":email", email);
query.bindValue(":mdp", password);  // comparaison directe (en clair)

if (query.exec() && query.next()) {
    role = query.value("role").toString();
    authenticated = true;
} else {
    QMessageBox::critical(nullptr, "Erreur", "Email ou mot de passe incorrect.");
}
       
    }
    // Lancer l'interface appropriée
    if (role == "admin")
    {
        AdminWindow w;
        w.show();
        return a.exec();
    }
    else
    {
        ClientWindow w(email);
        w.show();
        return a.exec();
    }
}