// dialogs/registerdialog.cpp
#include "dialogs/registerdialog.h"
#include "database/database.h"
#include "utils/emailsender.h"
#include <QMessageBox>
#include <QSqlQuery>
#include <QDebug>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QCheckBox>
#include <QHBoxLayout>
#include <QCheckBox>
#include <QHBoxLayout>

#include <QCryptographicHash>


QString hashMotDePasse(const QString &mdp) {
    QByteArray hash = QCryptographicHash::hash(mdp.toUtf8(), QCryptographicHash::Sha256);
    return hash.toHex();
}



RegisterDialog::RegisterDialog(QWidget *parent)
    : QDialog(parent)
{
    // Création des widgets
    lineEditNom = new QLineEdit(this);
    lineEditPrenom = new QLineEdit(this);
    lineEditEmail = new QLineEdit(this);
    lineEditAdresse = new QLineEdit(this);
    lineEditTelephone = new QLineEdit(this);
    lineEditMotDePasse = new QLineEdit(this);
    lineEditMotDePasse->setEchoMode(QLineEdit::Normal); // masque la saisie
    lineEditConfirmation = new QLineEdit(this);
 QWidget *confirmationWidget = new QWidget(this);
QHBoxLayout *confirmationLayout = new QHBoxLayout(confirmationWidget);
confirmationLayout->setContentsMargins(0, 0, 0, 0);
lineEditConfirmation = new QLineEdit(confirmationWidget);
lineEditConfirmation->setEchoMode(QLineEdit::Password);
QCheckBox *showCheckBox = new QCheckBox("Afficher", confirmationWidget);
confirmationLayout->addWidget(lineEditConfirmation);
confirmationLayout->addWidget(showCheckBox);  
connect(showCheckBox, &QCheckBox::toggled, [this](bool checked) {
    lineEditConfirmation->setEchoMode(checked ? QLineEdit::Normal : QLineEdit::Password);
});
 comboBoxType = new QComboBox(this);
    comboBoxType->addItem("Personne");
    comboBoxType->addItem("Entreprise");
    lineEditNomEntreprise = new QLineEdit(this);
    lineEditICE = new QLineEdit(this);
    pushButtonEnregistrer = new QPushButton("S'enregistrer", this);
    pushButtonAnnuler = new QPushButton("Annuler", this);

    // Masquer les champs entreprise par défaut
    lineEditNomEntreprise->setVisible(false);
    lineEditICE->setVisible(false);

    // Disposition des champs avec QFormLayout
    QFormLayout *formLayout = new QFormLayout;
    formLayout->addRow("Nom :", lineEditNom);
    formLayout->addRow("Prénom :", lineEditPrenom);
    formLayout->addRow("Email :", lineEditEmail);
    formLayout->addRow("Adresse :", lineEditAdresse);
    formLayout->addRow("Téléphone :", lineEditTelephone);
    formLayout->addRow("Mot de passe :", lineEditMotDePasse);
   formLayout->addRow("Confirmation :", confirmationWidget); 
      formLayout->addRow("Type :", comboBoxType);
    formLayout->addRow("Nom entreprise :", lineEditNomEntreprise);
    formLayout->addRow("ICE :", lineEditICE);

    // Boutons
    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->addStretch();
    buttonLayout->addWidget(pushButtonEnregistrer);
    buttonLayout->addWidget(pushButtonAnnuler);

    // Layout principal
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(formLayout);
    mainLayout->addLayout(buttonLayout);

    // Connexions
    connect(comboBoxType, &QComboBox::currentTextChanged, this, &RegisterDialog::on_typeChanged);
    connect(pushButtonEnregistrer, &QPushButton::clicked, this, &RegisterDialog::on_enregistrerClicked);
    connect(pushButtonAnnuler, &QPushButton::clicked, this, &QDialog::reject);

    setWindowTitle("Inscription client");
    setMinimumSize(400, 300);
}

RegisterDialog::~RegisterDialog()
{
    // Rien à libérer explicitement (les enfants de QDialog seront détruits automatiquement)
}

void RegisterDialog::on_typeChanged(const QString &type)
{
    bool isEntreprise = (type == "Entreprise");
    lineEditNomEntreprise->setVisible(isEntreprise);
    lineEditICE->setVisible(isEntreprise);
}

void RegisterDialog::on_enregistrerClicked()
{
    Client client;
    client.nom = lineEditNom->text().trimmed();
    client.prenom = lineEditPrenom->text().trimmed();
    client.email = lineEditEmail->text().trimmed();
    client.adresse = lineEditAdresse->text().trimmed();
    client.telephone = lineEditTelephone->text().trimmed();
    client.type = (comboBoxType->currentText() == "Entreprise") ? "entreprise" : "personne";

    if (client.type == "entreprise") {
        client.nomEntreprise = lineEditNomEntreprise->text().trimmed();
        client.ice = lineEditICE->text().trimmed();
        if (client.nomEntreprise.isEmpty()) {
            QMessageBox::warning(this, "Champ manquant", "Veuillez saisir le nom de l'entreprise.");
            return;
        }
    }
// Validation du domaine email (doit se terminer par @gmail.com)
if (!client.email.endsWith("@gmail.com", Qt::CaseInsensitive)) {
    QMessageBox::warning(this, "Email invalide", "L'email doit être une adresse Gmail (@gmail.com).");
    return;
}
    if (!client.isValid()) {
        QMessageBox::warning(this, "Champ manquant", "Nom, prénom et email sont obligatoires.");
        return;
    }
// Récupération des mots de passe
QString mdp = lineEditMotDePasse->text().trimmed();
QString mdpConfirm = lineEditConfirmation->text().trimmed();

if (mdp.isEmpty()) {
    QMessageBox::warning(this, "Champ manquant", "Veuillez saisir un mot de passe.");
    return;
}
if (mdp != mdpConfirm) {
    QMessageBox::warning(this, "Erreur", "Les mots de passe ne correspondent pas.");
    return;
}
client.motDePasse = mdp;   
    // Insertion en base de données
    QSqlQuery query;
query.prepare("INSERT INTO clients (nom, prenom, email, adresse, telephone, mot_de_passe, type, ice, nom_entreprise) "
              "VALUES (:nom, :prenom, :email, :adresse, :telephone, :mot_de_passe, :type, :ice, :nom_entreprise)");
    query.bindValue(":nom", client.nom);
    query.bindValue(":prenom", client.prenom);
    query.bindValue(":email", client.email);
    query.bindValue(":adresse", client.adresse);
    query.bindValue(":telephone", client.telephone);
    query.bindValue(":mot_de_passe", client.motDePasse);   
    query.bindValue(":type", client.type);
    query.bindValue(":ice", client.ice.isEmpty() ? QVariant() : client.ice);
    query.bindValue(":nom_entreprise", client.nomEntreprise.isEmpty() ? QVariant() : client.nomEntreprise);

    if (!query.exec()) {
        QMessageBox::critical(this, "Erreur", "Échec de l'enregistrement : " + query.lastError().text());
        return;
    }

    client.id = query.lastInsertId().toInt();
    QMessageBox::information(this, "Succès", QString("Client enregistré avec l'ID %1.").arg(client.id));

    // Envoi d'email de bienvenue (asynchrone)
    qDebug() << "Envoi d'email à" << client.email;
    EmailSender *sender = new EmailSender(this);
    sender->sendWelcomeEmail(client.email, client.prenom + " " + client.nom);

    accept(); // ferme la boîte de dialogue

}

Client RegisterDialog::getClient() const
{
    Client client;
    client.nom = lineEditNom->text().trimmed();
    client.prenom = lineEditPrenom->text().trimmed();
    client.email = lineEditEmail->text().trimmed();
    client.adresse = lineEditAdresse->text().trimmed();
    client.telephone = lineEditTelephone->text().trimmed();
    client.type = (comboBoxType->currentText() == "Entreprise") ? "entreprise" : "personne";
    client.nomEntreprise = lineEditNomEntreprise->text().trimmed();
    client.ice = lineEditICE->text().trimmed();
    return client;
}