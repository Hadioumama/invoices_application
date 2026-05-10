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
#include <QCryptographicHash>
#include <QFrame>
#include <QGraphicsDropShadowEffect>

RegisterDialog::RegisterDialog(QWidget *parent)
    : QDialog(parent)
{
    setupUI();
    applyStyles();
}

RegisterDialog::~RegisterDialog() {}

void RegisterDialog::setupUI()
{
    setWindowTitle("Créer un compte");
    setFixedSize(480, 620);
    setWindowFlags(Qt::Widget);

    // Layout principal avec marges
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    // ===== HEADER =====
    QWidget *headerWidget = new QWidget(this);
    headerWidget->setFixedHeight(100);
    headerWidget->setStyleSheet("background: #2B6CB0;");
    
    QVBoxLayout *headerLayout = new QVBoxLayout(headerWidget);
    headerLayout->setAlignment(Qt::AlignCenter);
    
    QLabel *titleLabel = new QLabel("Créer un compte");
    titleLabel->setStyleSheet(
        "color: white; font-size: 24px; font-weight: bold; background: transparent;");
    titleLabel->setAlignment(Qt::AlignCenter);
    
    QLabel *subtitleLabel = new QLabel("Rejoignez FacturationApp");
    subtitleLabel->setStyleSheet(
        "color: #E2E8F0; font-size: 13px; background: transparent;");
    subtitleLabel->setAlignment(Qt::AlignCenter);
    
    headerLayout->addWidget(titleLabel);
    headerLayout->addWidget(subtitleLabel);
    mainLayout->addWidget(headerWidget);

    // ===== CONTENU =====
    QWidget *contentWidget = new QWidget(this);
    contentWidget->setStyleSheet("background: white;");
    
    QVBoxLayout *contentLayout = new QVBoxLayout(contentWidget);
    contentLayout->setSpacing(14);
    contentLayout->setContentsMargins(40, 30, 40, 30);

    // -- Nom & Prénom (ligne côte à côte) --
    QHBoxLayout *nameLayout = new QHBoxLayout;
    nameLayout->setSpacing(12);
    
    QVBoxLayout *nomLayout = new QVBoxLayout;
    nomLayout->setSpacing(4);
    QLabel *nomLabel = new QLabel("Nom");
    nomLabel->setStyleSheet("font-size: 13px; font-weight: 600; color: #4A5568;");
    lineEditNom = new QLineEdit;
    lineEditNom->setPlaceholderText("Votre nom");
    lineEditNom->setFixedHeight(40);
    nomLayout->addWidget(nomLabel);
    nomLayout->addWidget(lineEditNom);
    
    QVBoxLayout *prenomLayout = new QVBoxLayout;
    prenomLayout->setSpacing(4);
    QLabel *prenomLabel = new QLabel("Prénom");
    prenomLabel->setStyleSheet("font-size: 13px; font-weight: 600; color: #4A5568;");
    lineEditPrenom = new QLineEdit;
    lineEditPrenom->setPlaceholderText("Votre prénom");
    lineEditPrenom->setFixedHeight(40);
    prenomLayout->addWidget(prenomLabel);
    prenomLayout->addWidget(lineEditPrenom);
    
    nameLayout->addLayout(nomLayout);
    nameLayout->addLayout(prenomLayout);
    contentLayout->addLayout(nameLayout);

    // -- Email --
    QLabel *emailLabel = new QLabel("Email");
    emailLabel->setStyleSheet("font-size: 13px; font-weight: 600; color: #4A5568;");
    lineEditEmail = new QLineEdit;
    lineEditEmail->setPlaceholderText("exemple@gmail.com");
    lineEditEmail->setFixedHeight(40);
    contentLayout->addWidget(emailLabel);
    contentLayout->addWidget(lineEditEmail);

    // -- Adresse --
    QLabel *adresseLabel = new QLabel("Adresse");
    adresseLabel->setStyleSheet("font-size: 13px; font-weight: 600; color: #4A5568;");
    lineEditAdresse = new QLineEdit;
    lineEditAdresse->setPlaceholderText("Votre adresse complète");
    lineEditAdresse->setFixedHeight(40);
    contentLayout->addWidget(adresseLabel);
    contentLayout->addWidget(lineEditAdresse);

    // -- Téléphone --
    QLabel *telLabel = new QLabel("Téléphone");
    telLabel->setStyleSheet("font-size: 13px; font-weight: 600; color: #4A5568;");
    lineEditTelephone = new QLineEdit;
    lineEditTelephone->setPlaceholderText("+212 6XX XXX XXX");
    lineEditTelephone->setFixedHeight(40);
    contentLayout->addWidget(telLabel);
    contentLayout->addWidget(lineEditTelephone);

    // -- Mot de passe --
    QLabel *mdpLabel = new QLabel("Mot de passe");
    mdpLabel->setStyleSheet("font-size: 13px; font-weight: 600; color: #4A5568;");
    lineEditMotDePasse = new QLineEdit;
    lineEditMotDePasse->setPlaceholderText("Min. 6 caractères");
    lineEditMotDePasse->setEchoMode(QLineEdit::Password);
    lineEditMotDePasse->setFixedHeight(40);
    contentLayout->addWidget(mdpLabel);
    contentLayout->addWidget(lineEditMotDePasse);

    // -- Confirmation avec checkbox --
    QHBoxLayout *confirmLayout = new QHBoxLayout;
    confirmLayout->setSpacing(8);
    
    lineEditConfirmation = new QLineEdit;
    lineEditConfirmation->setPlaceholderText("Confirmer le mot de passe");
    lineEditConfirmation->setEchoMode(QLineEdit::Password);
    lineEditConfirmation->setFixedHeight(40);
    
    showPasswordCheck = new QCheckBox("Afficher");
    showPasswordCheck->setStyleSheet("color: #718096; font-size: 12px;");
    
    confirmLayout->addWidget(lineEditConfirmation, 1);
    confirmLayout->addWidget(showPasswordCheck);
    contentLayout->addWidget(new QLabel("Confirmation"));
    contentLayout->itemAt(contentLayout->count()-1)->widget()->setStyleSheet(
        "font-size: 13px; font-weight: 600; color: #4A5568;");
    contentLayout->addLayout(confirmLayout);

    // -- Type --
    QLabel *typeLabel = new QLabel("Type de compte");
    typeLabel->setStyleSheet("font-size: 13px; font-weight: 600; color: #4A5568;");
    comboBoxType = new QComboBox;
    comboBoxType->addItem("Personne");
    comboBoxType->addItem("Entreprise");
    comboBoxType->setFixedHeight(40);
    contentLayout->addWidget(typeLabel);
    contentLayout->addWidget(comboBoxType);

    // -- Champs entreprise (cachés par défaut) --
    QWidget *entrepriseWidget = new QWidget;
    QVBoxLayout *entrepriseLayout = new QVBoxLayout(entrepriseWidget);
    entrepriseLayout->setSpacing(12);
    entrepriseLayout->setContentsMargins(0, 0, 0, 0);
    
    QLabel *nomEntLabel = new QLabel("Nom entreprise");
    nomEntLabel->setStyleSheet("font-size: 13px; font-weight: 600; color: #4A5568;");
    lineEditNomEntreprise = new QLineEdit;
    lineEditNomEntreprise->setPlaceholderText("Nom de l'entreprise");
    lineEditNomEntreprise->setFixedHeight(40);
    
    QLabel *iceLabel = new QLabel("ICE");
    iceLabel->setStyleSheet("font-size: 13px; font-weight: 600; color: #4A5568;");
    lineEditICE = new QLineEdit;
    lineEditICE->setPlaceholderText("Identifiant Commun de l'Entreprise");
    lineEditICE->setFixedHeight(40);
    
    entrepriseLayout->addWidget(nomEntLabel);
    entrepriseLayout->addWidget(lineEditNomEntreprise);
    entrepriseLayout->addWidget(iceLabel);
    entrepriseLayout->addWidget(lineEditICE);
    
    entrepriseWidget->setVisible(false);
    contentLayout->addWidget(entrepriseWidget);

    // -- Status label (messages d'erreur) --
    statusLabel = new QLabel("");
    statusLabel->setAlignment(Qt::AlignCenter);
    statusLabel->setStyleSheet("color: #E53E3E; font-size: 12px; padding: 5px;");
    contentLayout->addWidget(statusLabel);

    // -- Bouton S'enregistrer --
    pushButtonEnregistrer = new QPushButton("S'enregistrer");
    pushButtonEnregistrer->setFixedHeight(44);
    pushButtonEnregistrer->setCursor(Qt::PointingHandCursor);
    contentLayout->addWidget(pushButtonEnregistrer);

    // -- Lien vers connexion --
    QHBoxLayout *loginLinkLayout = new QHBoxLayout;
    loginLinkLayout->setAlignment(Qt::AlignCenter);
    
    QLabel *alreadyLabel = new QLabel("Déjà un compte ?");
    alreadyLabel->setStyleSheet("color: #718096; font-size: 13px;");
    
    QPushButton *loginLinkBtn = new QPushButton("Se connecter");
    loginLinkBtn->setStyleSheet(
        "QPushButton { background: transparent; color: #2B6CB0; "
        "border: none; font-size: 13px; font-weight: 600; text-decoration: underline; }"
        "QPushButton:hover { color: #1A365D; }");
    loginLinkBtn->setCursor(Qt::PointingHandCursor);
    loginLinkBtn->setFlat(true);
    
    loginLinkLayout->addWidget(alreadyLabel);
    loginLinkLayout->addWidget(loginLinkBtn);
    contentLayout->addLayout(loginLinkLayout);

    mainLayout->addWidget(contentWidget, 1);

    // ===== CONNEXIONS =====
    connect(comboBoxType, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [this, entrepriseWidget](int index) {
        entrepriseWidget->setVisible(index == 1); // 1 = Entreprise
    });
    
    connect(showPasswordCheck, &QCheckBox::toggled, this, &RegisterDialog::togglePasswordVisibility);
    connect(pushButtonEnregistrer, &QPushButton::clicked, this, &RegisterDialog::on_enregistrerClicked);
    connect(loginLinkBtn, &QPushButton::clicked, this, &QDialog::reject);
}

void RegisterDialog::applyStyles()
{
    // Style global des QLineEdit
    QString lineEditStyle = R"(
        QLineEdit {
            border: 2px solid #E2E8F0;
            border-radius: 8px;
            padding: 5px 14px;
            font-size: 14px;
            background: #F7FAFC;
            color: #2D3748;
        }
        QLineEdit:focus {
            border: 2px solid #2B6CB0;
            background: white;
        }
        QLineEdit:hover {
            border: 2px solid #CBD5E0;
        }
    )";
    
    lineEditNom->setStyleSheet(lineEditStyle);
    lineEditPrenom->setStyleSheet(lineEditStyle);
    lineEditEmail->setStyleSheet(lineEditStyle);
    lineEditAdresse->setStyleSheet(lineEditStyle);
    lineEditTelephone->setStyleSheet(lineEditStyle);
    lineEditMotDePasse->setStyleSheet(lineEditStyle);
    lineEditConfirmation->setStyleSheet(lineEditStyle);
    lineEditNomEntreprise->setStyleSheet(lineEditStyle);
    lineEditICE->setStyleSheet(lineEditStyle);

    // Style QComboBox
    comboBoxType->setStyleSheet(R"(
        QComboBox {
            border: 2px solid #E2E8F0;
            border-radius: 8px;
            padding: 5px 14px;
            font-size: 14px;
            background: #F7FAFC;
            color: #2D3748;
        }
        QComboBox:focus {
            border: 2px solid #2B6CB0;
        }
        QComboBox::drop-down {
            border: none;
            width: 30px;
        }
        QComboBox::down-arrow {
            image: none;
            border-left: 5px solid transparent;
            border-right: 5px solid transparent;
            border-top: 6px solid #718096;
        }
        QComboBox QAbstractItemView {
            border: 1px solid #E2E8F0;
            border-radius: 8px;
            background: white;
            selection-background-color: #2B6CB0;
            selection-color: white;
            padding: 5px;
        }
    )");

    // Style bouton principal
    pushButtonEnregistrer->setStyleSheet(R"(
        QPushButton {
            background: #2B6CB0;
            color: white;
            font-weight: bold;
            font-size: 15px;
            border: none;
            border-radius: 8px;
        }
        QPushButton:hover {
            background: #1A365D;
        }
        QPushButton:pressed {
            background: #2C5282;
        }
        QPushButton:disabled {
            background: #A0AEC0;
        }
    )");
}

void RegisterDialog::togglePasswordVisibility(bool checked)
{
    lineEditMotDePasse->setEchoMode(checked ? QLineEdit::Normal : QLineEdit::Password);
    lineEditConfirmation->setEchoMode(checked ? QLineEdit::Normal : QLineEdit::Password);
}

QString RegisterDialog::hashPassword(const QString &password)
{
    QByteArray hash = QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Sha256);
    return hash.toHex();
}

bool RegisterDialog::validateInputs()
{
    statusLabel->clear();
    
    if (lineEditNom->text().trimmed().isEmpty() || 
        lineEditPrenom->text().trimmed().isEmpty()) {
        statusLabel->setText("❌ Nom et prénom sont obligatoires");
        return false;
    }
    
    QString email = lineEditEmail->text().trimmed();
    if (email.isEmpty()) {
        statusLabel->setText("❌ L'email est obligatoire");
        return false;
    }
    if (!email.endsWith("@gmail.com", Qt::CaseInsensitive)) {
        statusLabel->setText("❌ L'email doit être une adresse Gmail");
        return false;
    }
    
    QString mdp = lineEditMotDePasse->text();
    QString confirm = lineEditConfirmation->text();
    
    if (mdp.length() < 6) {
        statusLabel->setText("❌ Le mot de passe doit contenir au moins 6 caractères");
        return false;
    }
    if (mdp != confirm) {
        statusLabel->setText("❌ Les mots de passe ne correspondent pas");
        return false;
    }
    
    if (comboBoxType->currentText() == "Entreprise") {
        if (lineEditNomEntreprise->text().trimmed().isEmpty()) {
            statusLabel->setText("❌ Le nom de l'entreprise est obligatoire");
            return false;
        }
    }
    
    return true;
}

void RegisterDialog::on_enregistrerClicked()
{
    if (!validateInputs()) {
        return;
    }

    Client client;
    client.nom = lineEditNom->text().trimmed();
    client.prenom = lineEditPrenom->text().trimmed();
    client.email = lineEditEmail->text().trimmed().toLower();
    client.adresse = lineEditAdresse->text().trimmed();
    client.telephone = lineEditTelephone->text().trimmed();
    client.type = (comboBoxType->currentText() == "Entreprise") ? "entreprise" : "personne";
    client.nomEntreprise = lineEditNomEntreprise->text().trimmed();
    client.ice = lineEditICE->text().trimmed();
    
    // Hash du mot de passe
    QString mdp = lineEditMotDePasse->text();
    client.motDePasse = hashPassword(mdp);

    // Insertion en base de données
    QSqlQuery query;
    query.prepare("INSERT INTO clients (nom, prenom, email, adresse, telephone, "
                  "mot_de_passe, type, ice, nom_entreprise, role) "
                  "VALUES (:nom, :prenom, :email, :adresse, :telephone, "
                  ":mot_de_passe, :type, :ice, :nom_entreprise, 'client')");
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
        statusLabel->setText("❌ Erreur : " + query.lastError().text());
        return;
    }

    client.id = query.lastInsertId().toInt();
    
    // Envoi d'email de bienvenue (asynchrone)
    EmailSender *sender = new EmailSender(this);
    sender->sendWelcomeEmail(client.email, client.prenom + " " + client.nom);
    
    // ✅ Message de succès et émission du signal
    QMessageBox::information(this, "Succès", 
        QString("Compte créé avec succès !\nVous pouvez maintenant vous connecter."));
    
    emit registerSuccess();
    accept();
}

void RegisterDialog::on_typeChanged(const QString &type)
{
    // Géré dans setupUI via lambda
    Q_UNUSED(type)
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