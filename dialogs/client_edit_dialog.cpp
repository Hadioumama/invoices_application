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
#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QString>

ClientEditDialog::ClientEditDialog(QWidget *parent) 
    : QDialog(parent), currentId(-1)
{
    setWindowTitle("Gestion Client");
    setMinimumWidth(480);
    resize(480, 520);

    setStyleSheet(
        "QDialog { background: white; }"
        "QGroupBox {"
        "  font-weight: bold; font-size: 12px;"
        "  border: 1px solid #CBD5E0;"
        "  border-radius: 6px;"
        "  margin-top: 8px; padding-top: 8px;"
        "  color: #123f6f;"
        "}"
        "QGroupBox::title {"
        "  subcontrol-origin: margin; left: 10px;"
        "}"
        "QLineEdit, QComboBox {"
        "  border: 1px solid #CBD5E0;"
        "  border-radius: 4px;"
        "  padding: 4px 8px;"
        "  background: white;"
        "  min-height: 20px;"
        "  font-size: 11px;"
        "}"
        "QLineEdit:focus { border: 1px solid #3182CE; }"
        "QLabel { font-size: 10px; color: #4A5568; }"
    );

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(10);
    mainLayout->setContentsMargins(16, 16, 16, 16);

    // ── TITRE ──────────────────────────────────────────────
    QLabel *titleLabel = new QLabel("👤 Informations Client");
    titleLabel->setStyleSheet(
        "font-size:15px;font-weight:bold;color:#1B2A3B;"
        "padding-bottom:8px;border-bottom:2px solid #3182CE;"
        "margin-bottom:4px;");
    mainLayout->addWidget(titleLabel);

    // ── GROUPE INFOS PERSONNELLES ───────────────────────────
    QGroupBox *persoGroup = new QGroupBox("Informations Personnelles");
    QFormLayout *persoForm = new QFormLayout(persoGroup);
    persoForm->setSpacing(8);
    persoForm->setLabelAlignment(Qt::AlignRight);

    lineEditNom = new QLineEdit;
    lineEditNom->setPlaceholderText("Entrez le nom...");
    lineEditPrenom = new QLineEdit;
    lineEditPrenom->setPlaceholderText("Entrez le prénom...");
    lineEditEmail = new QLineEdit;
    lineEditEmail->setPlaceholderText("exemple@email.com");
    lineEditTelephone = new QLineEdit;
    lineEditTelephone->setPlaceholderText("+212 6XX XXX XXX");
    lineEditAdresse = new QLineEdit;
    lineEditAdresse->setPlaceholderText("Adresse complète...");
    lineEditMotDePasse = new QLineEdit;
    lineEditMotDePasse->setEchoMode(QLineEdit::Password);
    lineEditMotDePasse->setPlaceholderText("Laisser vide pour ne pas modifier");

    persoForm->addRow("Nom :*", lineEditNom);
    persoForm->addRow("Prénom :*", lineEditPrenom);
    persoForm->addRow("Email :*", lineEditEmail);
    persoForm->addRow("Téléphone :", lineEditTelephone);
    persoForm->addRow("Adresse :", lineEditAdresse);
    persoForm->addRow("Mot de passe :", lineEditMotDePasse);
    mainLayout->addWidget(persoGroup);

    // ── GROUPE TYPE ─────────────────────────────────────────
    QGroupBox *typeGroup = new QGroupBox("Type de Client");
    QFormLayout *typeForm = new QFormLayout(typeGroup);
    typeForm->setSpacing(8);
    typeForm->setLabelAlignment(Qt::AlignRight);

    comboBoxType = new QComboBox;
    comboBoxType->addItem("👤 Personne", "personne");
    comboBoxType->addItem("🏢 Entreprise", "entreprise");
    typeForm->addRow("Type :", comboBoxType);

    lineEditNomEntreprise = new QLineEdit;
    lineEditNomEntreprise->setPlaceholderText("Nom de l'entreprise...");
    lineEditICE = new QLineEdit;
    lineEditICE->setPlaceholderText("Numéro ICE...");

    labelNomEntreprise = new QLabel("Nom entreprise :");
    labelICE = new QLabel("ICE :");

    typeForm->addRow(labelNomEntreprise, lineEditNomEntreprise);
    typeForm->addRow(labelICE, lineEditICE);

    lineEditNomEntreprise->setVisible(false);
    lineEditICE->setVisible(false);
    labelNomEntreprise->setVisible(false);
    labelICE->setVisible(false);

    mainLayout->addWidget(typeGroup);

    // ── BOUTONS ─────────────────────────────────────────────
    QHBoxLayout *btnLayout = new QHBoxLayout;
    cancelButton = new QPushButton("Annuler");
    saveButton = new QPushButton("💾 Enregistrer");
    cancelButton->setFixedHeight(36);
    saveButton->setFixedHeight(36);
    cancelButton->setStyleSheet(
        "padding:0 20px;border-radius:4px;"
        "border:1px solid #CBD5E0;background:white;font-size:11px;");
    saveButton->setStyleSheet(
        "background:#27AE60;color:white;font-weight:bold;"
        "padding:0 20px;border-radius:4px;border:none;font-size:11px;");
    btnLayout->addStretch();
    btnLayout->addWidget(cancelButton);
    btnLayout->addWidget(saveButton);
    mainLayout->addLayout(btnLayout);

    connect(comboBoxType, &QComboBox::currentTextChanged,
            this, &ClientEditDialog::on_typeChanged);
    connect(saveButton, &QPushButton::clicked,
            this, &ClientEditDialog::onSave);
    connect(cancelButton, &QPushButton::clicked,
            this, &QDialog::reject);
}
void ClientEditDialog::on_typeChanged(const QString &type)
{
    bool isEntreprise = type.contains("Entreprise");
    lineEditNomEntreprise->setVisible(isEntreprise);
    lineEditICE->setVisible(isEntreprise);
    labelNomEntreprise->setVisible(isEntreprise);
    labelICE->setVisible(isEntreprise);
    adjustSize();
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