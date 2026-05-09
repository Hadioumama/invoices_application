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

    if (!Database::instance().connect("facturation.db")) {
        return -1;
    }
    Database::instance().initializeTables();

    bool authenticated = false;
    QString role;
    int clientId = -1;  // ← Variable pour stocker l'ID

    while (!authenticated)
    {
        LoginDialog login;
        QObject::connect(&login, &LoginDialog::createAccountRequested, [&]() {
            RegisterDialog registerDlg;
            if (registerDlg.exec() == QDialog::Accepted) {
                QMessageBox::information(nullptr, "Succès", "Compte créé. Veuillez vous connecter.");
            }
        });

        if (login.exec() != QDialog::Accepted) {
            return 0;
        }

        QString email = login.getEmail();
        QString password = login.getPassword();

        QSqlQuery query;
        query.prepare("SELECT id, role FROM clients WHERE email = :email AND mot_de_passe = :mdp");
        query.bindValue(":email", email);
        query.bindValue(":mdp", password);

        if (query.exec() && query.next()) {
            clientId = query.value(0).toInt();   // ✅ Stocker l'ID
            role = query.value(1).toString();      // Stocker le rôle
            authenticated = true;
        } else {
            QMessageBox::critical(nullptr, "Erreur", "Email ou mot de passe incorrect.");
        }
    }

    // Lancer l'interface
    if (role == "admin") {
        AdminWindow w;
        w.show();
        return a.exec();
    } else {
        if (clientId < 0) {  // Sécurité
            QMessageBox::critical(nullptr, "Erreur", "ID client invalide.");
            return -1;
        }
        ClientWindow w(clientId);  // ✅ Passe l'int, pas le QString !
        w.show();
        return a.exec();
    }
}