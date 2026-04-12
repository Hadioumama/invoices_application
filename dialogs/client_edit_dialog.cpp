#include "dialogs/client_edit_dialog.h"
#include <QLabel>
#include <QComboBox>
#include <QLineEdit> 
#include <QFormLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QSqlQuery>
#include <QSqlError>
#include <QPushButton>
#include <QCryptographicHash>

ClientEditDialog::ClientEditDialog(QWidget *parent) : QDialog(parent), currentId(-1)
{
    setWindowTitle("Client");

    lineEditNom = new QLineEdit;
    lineEditPrenom = new QLineEdit;
    lineEditEmail = new QLineEdit;
    lineEditAdresse = new QLineEdit;
    lineEditTelephone = new QLineEdit;
    lineEditMotDePasse = new QLineEdit;
    lineEditMotDePasse->setEchoMode(QLineEdit::Password);
    comboBoxType = new QComboBox;
    comboBoxType->addItem("Personne");
    comboBoxType->addItem("Entreprise");
    lineEditNomEntreprise = new QLineEdit;
    lineEditICE = new QLineEdit;

    lineEditNomEntreprise->setVisible(false);
    lineEditICE->setVisible(false);

    QFormLayout *formLayout = new QFormLayout;
    formLayout->addRow("Nom :", lineEditNom);
    formLayout->addRow("Prénom :", lineEditPrenom);
    formLayout->addRow("Email :", lineEditEmail);
    formLayout->addRow("Adresse :", lineEditAdresse);
    formLayout->addRow("Téléphone :", lineEditTelephone);
    formLayout->addRow("Mot de passe :", lineEditMotDePasse);
    formLayout->addRow("Type :", comboBoxType);
    formLayout->addRow("Nom entreprise :", lineEditNomEntreprise);
    formLayout->addRow("ICE :", lineEditICE);

    saveButton = new QPushButton("Enregistrer");
    cancelButton = new QPushButton("Annuler");
    QHBoxLayout *btnLayout = new QHBoxLayout;
    btnLayout->addStretch();
    btnLayout->addWidget(saveButton);
    btnLayout->addWidget(cancelButton);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(formLayout);
    mainLayout->addLayout(btnLayout);

    connect(comboBoxType, &QComboBox::currentTextChanged, this, &ClientEditDialog::on_typeChanged);
    connect(saveButton, &QPushButton::clicked, this, &ClientEditDialog::onSave);
    connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);
}

void ClientEditDialog::on_typeChanged(const QString &type)
{
    bool isEntreprise = (type == "Entreprise");
    lineEditNomEntreprise->setVisible(isEntreprise);
    lineEditICE->setVisible(isEntreprise);
}

void ClientEditDialog::setClientId(int id)
{
    currentId = id;
    if (id != -1) loadClient(id);
}

void ClientEditDialog::loadClient(int id)
{
    QSqlQuery query;
    query.prepare("SELECT nom, prenom, email, adresse, telephone, type, ice, nom_entreprise FROM clients WHERE id = ?");
    query.addBindValue(id);
    if (!query.exec() || !query.next()) {
        QMessageBox::critical(this, "Erreur", "Client introuvable.");
        return;
    }
    lineEditNom->setText(query.value("nom").toString());
    lineEditPrenom->setText(query.value("prenom").toString());
    lineEditEmail->setText(query.value("email").toString());
    lineEditAdresse->setText(query.value("adresse").toString());
    lineEditTelephone->setText(query.value("telephone").toString());
    QString type = query.value("type").toString();
    comboBoxType->setCurrentText((type == "entreprise") ? "Entreprise" : "Personne");
    lineEditNomEntreprise->setText(query.value("nom_entreprise").toString());
    lineEditICE->setText(query.value("ice").toString());
    lineEditMotDePasse->clear();
    on_typeChanged(comboBoxType->currentText());
}

void ClientEditDialog::saveClient()
{
    if (lineEditNom->text().trimmed().isEmpty() || lineEditPrenom->text().trimmed().isEmpty() || lineEditEmail->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "Champs manquants", "Nom, prénom et email sont obligatoires.");
        return;
    }
    QString type = (comboBoxType->currentText() == "Entreprise") ? "entreprise" : "personne";
    if (type == "entreprise" && lineEditNomEntreprise->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "Champ manquant", "Nom de l'entreprise obligatoire.");
        return;
    }

    QSqlQuery query;
    if (currentId == -1) {
        if (lineEditMotDePasse->text().isEmpty()) {
            QMessageBox::warning(this, "Mot de passe", "Le mot de passe est obligatoire pour un nouveau client.");
            return;
        }
        QString hashedPwd = QCryptographicHash::hash(lineEditMotDePasse->text().toUtf8(), QCryptographicHash::Sha256).toHex();
        query.prepare("INSERT INTO clients (nom, prenom, email, adresse, telephone, mot_de_passe, type, ice, nom_entreprise) "
                      "VALUES (:nom, :prenom, :email, :adresse, :telephone, :mdp, :type, :ice, :nom_entreprise)");
        query.bindValue(":mdp", hashedPwd);
    } else {
        if (lineEditMotDePasse->text().isEmpty()) {
            query.prepare("UPDATE clients SET nom=:nom, prenom=:prenom, email=:email, adresse=:adresse, telephone=:telephone, "
                          "type=:type, ice=:ice, nom_entreprise=:nom_entreprise WHERE id=:id");
        } else {
            QString hashedPwd = QCryptographicHash::hash(lineEditMotDePasse->text().toUtf8(), QCryptographicHash::Sha256).toHex();
            query.prepare("UPDATE clients SET nom=:nom, prenom=:prenom, email=:email, adresse=:adresse, telephone=:telephone, "
                          "mot_de_passe=:mdp, type=:type, ice=:ice, nom_entreprise=:nom_entreprise WHERE id=:id");
            query.bindValue(":mdp", hashedPwd);
        }
        query.bindValue(":id", currentId);
    }
    query.bindValue(":nom", lineEditNom->text().trimmed());
    query.bindValue(":prenom", lineEditPrenom->text().trimmed());
    query.bindValue(":email", lineEditEmail->text().trimmed());
    query.bindValue(":adresse", lineEditAdresse->text().trimmed());
    query.bindValue(":telephone", lineEditTelephone->text().trimmed());
    query.bindValue(":type", type);
    query.bindValue(":ice", lineEditICE->text().trimmed());
    query.bindValue(":nom_entreprise", lineEditNomEntreprise->text().trimmed());

    if (!query.exec()) {
        QMessageBox::critical(this, "Erreur", "Échec sauvegarde : " + query.lastError().text());
        return;
    }
    accept();
}

void ClientEditDialog::onSave()
{
    saveClient();
}