#include "dialogs/registerdialog.h"
#include "database/database.h"
#include "utils/emailsender.h"

#include <QMessageBox>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QCheckBox>
#include <QFrame>
#include <QCryptographicHash>

// ============================================================
// CONSTRUCTEUR / DESTRUCTEUR
// ============================================================

RegisterDialog::RegisterDialog(QWidget *parent)
    : QDialog(parent)
{
    setupUI();
    applyStyles();
}

RegisterDialog::~RegisterDialog() = default;

// ============================================================
// UI SETUP - STRUCTURÉ ET ESPACÉ
// ============================================================

void RegisterDialog::setupUI()
{
    setWindowTitle("Créer un compte");
    setFixedSize(520, 720);           // ✅ Plus grand pour éviter la compression
    setWindowFlags(Qt::Dialog | Qt::WindowCloseButtonHint);

    // ===== LAYOUT PRINCIPAL =====
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    // ========================================================
    // 1. HEADER (inchangé)
    // ========================================================
    QWidget *headerWidget = new QWidget(this);
    headerWidget->setFixedHeight(100);
    headerWidget->setStyleSheet("background-color: #2B6CB0;");

    QVBoxLayout *headerLayout = new QVBoxLayout(headerWidget);
    headerLayout->setAlignment(Qt::AlignCenter);
    headerLayout->setSpacing(4);

    m_titleLabel = new QLabel("Créer un compte", headerWidget);
    m_titleLabel->setStyleSheet(
        "color: white; font-size: 24px; font-weight: bold; background: transparent;");
    m_titleLabel->setAlignment(Qt::AlignCenter);

    m_subtitleLabel = new QLabel("Rejoignez FacturationApp", headerWidget);
    m_subtitleLabel->setStyleSheet(
        "color: #E2E8F0; font-size: 13px; background: transparent;");
    m_subtitleLabel->setAlignment(Qt::AlignCenter);

    headerLayout->addWidget(m_titleLabel);
    headerLayout->addWidget(m_subtitleLabel);
    mainLayout->addWidget(headerWidget);

    // ========================================================
    // 2. CONTENU - FORMULAIRE AVEC ESPACEMENT
    // ========================================================
    QWidget *contentWidget = new QWidget(this);
    contentWidget->setStyleSheet("background-color: white;");

    // ✅ QFormLayout pour un alignement label/champ parfait
    QFormLayout *formLayout = new QFormLayout(contentWidget);
    formLayout->setSpacing(0);           // Pas d'espace entre label et champ (géré manuellement)
    formLayout->setContentsMargins(35, 25, 35, 25);
    formLayout->setLabelAlignment(Qt::AlignLeft);
    formLayout->setFormAlignment(Qt::AlignLeft | Qt::AlignTop);
    formLayout->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);
    formLayout->setRowWrapPolicy(QFormLayout::DontWrapRows);

    // --------------------------------------------------------
    // 2.1 SECTION : IDENTITÉ
    // --------------------------------------------------------
    // Ligne Nom + Prénom (2 colonnes)
    QHBoxLayout *identityLayout = new QHBoxLayout;
    identityLayout->setSpacing(14);

    // Nom
    QVBoxLayout *nomCol = new QVBoxLayout;
    nomCol->setSpacing(6);
    nomCol->setContentsMargins(0, 0, 0, 0);
    QLabel *nomLabel = new QLabel("Nom *");
    nomLabel->setStyleSheet("font-size: 13px; font-weight: 600; color: #4A5568;");
    m_lineEditNom = new QLineEdit;
    m_lineEditNom->setPlaceholderText("Votre nom");
    m_lineEditNom->setFixedHeight(42);
    nomCol->addWidget(nomLabel);
    nomCol->addWidget(m_lineEditNom);

    // Prénom
    QVBoxLayout *prenomCol = new QVBoxLayout;
    prenomCol->setSpacing(6);
    prenomCol->setContentsMargins(0, 0, 0, 0);
    QLabel *prenomLabel = new QLabel("Prénom *");
    prenomLabel->setStyleSheet("font-size: 13px; font-weight: 600; color: #4A5568;");
    m_lineEditPrenom = new QLineEdit;
    m_lineEditPrenom->setPlaceholderText("Votre prénom");
    m_lineEditPrenom->setFixedHeight(42);
    prenomCol->addWidget(prenomLabel);
    prenomCol->addWidget(m_lineEditPrenom);

    identityLayout->addLayout(nomCol, 1);
    identityLayout->addLayout(prenomCol, 1);

    // Ajouter la ligne identité au form
    QWidget *identityRow = new QWidget;
    identityRow->setLayout(identityLayout);
    formLayout->addRow(identityRow);

    // ✅ ESPACE entre sections
    formLayout->addItem(new QSpacerItem(0, 16, QSizePolicy::Minimum, QSizePolicy::Fixed));

    // --------------------------------------------------------
    // 2.2 SECTION : CONTACT
    // --------------------------------------------------------
    // Email
    QLabel *emailLabel = new QLabel("Email *");
    emailLabel->setStyleSheet("font-size: 13px; font-weight: 600; color: #4A5568;");
    m_lineEditEmail = new QLineEdit;
    m_lineEditEmail->setPlaceholderText("exemple@gmail.com");
    m_lineEditEmail->setFixedHeight(42);
    formLayout->addRow(emailLabel, m_lineEditEmail);
    formLayout->addItem(new QSpacerItem(0, 12, QSizePolicy::Minimum, QSizePolicy::Fixed));

    // Téléphone
    QLabel *telLabel = new QLabel("Téléphone");
    telLabel->setStyleSheet("font-size: 13px; font-weight: 600; color: #4A5568;");
    m_lineEditTelephone = new QLineEdit;
    m_lineEditTelephone->setPlaceholderText("+212 6XX XXX XXX");
    m_lineEditTelephone->setFixedHeight(42);
    formLayout->addRow(telLabel, m_lineEditTelephone);
    formLayout->addItem(new QSpacerItem(0, 12, QSizePolicy::Minimum, QSizePolicy::Fixed));

    // Adresse
    QLabel *adresseLabel = new QLabel("Adresse");
    adresseLabel->setStyleSheet("font-size: 13px; font-weight: 600; color: #4A5568;");
    m_lineEditAdresse = new QLineEdit;
    m_lineEditAdresse->setPlaceholderText("Votre adresse complète");
    m_lineEditAdresse->setFixedHeight(42);
    formLayout->addRow(adresseLabel, m_lineEditAdresse);

    // ✅ ESPACE
    formLayout->addItem(new QSpacerItem(0, 16, QSizePolicy::Minimum, QSizePolicy::Fixed));

    // --------------------------------------------------------
    // 2.3 SECTION : SÉCURITÉ
    // --------------------------------------------------------
    // Mot de passe
    QLabel *mdpLabel = new QLabel("Mot de passe *");
    mdpLabel->setStyleSheet("font-size: 13px; font-weight: 600; color: #4A5568;");
    m_lineEditMotDePasse = new QLineEdit;
    m_lineEditMotDePasse->setPlaceholderText("Min. 6 caractères");
    m_lineEditMotDePasse->setEchoMode(QLineEdit::Password);
    m_lineEditMotDePasse->setFixedHeight(42);
    formLayout->addRow(mdpLabel, m_lineEditMotDePasse);
    formLayout->addItem(new QSpacerItem(0, 12, QSizePolicy::Minimum, QSizePolicy::Fixed));

    // Confirmation + Checkbox
    QHBoxLayout *confirmRow = new QHBoxLayout;
    confirmRow->setSpacing(10);

    m_lineEditConfirmation = new QLineEdit;
    m_lineEditConfirmation->setPlaceholderText("Confirmer le mot de passe");
    m_lineEditConfirmation->setEchoMode(QLineEdit::Password);
    m_lineEditConfirmation->setFixedHeight(42);

    m_showPasswordCheck = new QCheckBox("Afficher");
    m_showPasswordCheck->setStyleSheet("color: #718096; font-size: 12px; padding-top: 2px;");

    confirmRow->addWidget(m_lineEditConfirmation, 1);
    confirmRow->addWidget(m_showPasswordCheck);

    QLabel *confirmLabel = new QLabel("Confirmation *");
    confirmLabel->setStyleSheet("font-size: 13px; font-weight: 600; color: #4A5568;");
    formLayout->addRow(confirmLabel, confirmRow);

    // ✅ ESPACE
    formLayout->addItem(new QSpacerItem(0, 16, QSizePolicy::Minimum, QSizePolicy::Fixed));

  
    // --------------------------------------------------------
// 2.4 SECTION : TYPE DE COMPTE - FLÈCHE TRIANGLE
// --------------------------------------------------------
QLabel *typeLabel = new QLabel("Type de compte *");
typeLabel->setStyleSheet("font-size: 13px; font-weight: 600; color: #4A5568;");

m_comboBoxType = new QComboBox;
m_comboBoxType->addItem("Personne", "personne");
m_comboBoxType->addItem("Entreprise", "entreprise");
m_comboBoxType->setFixedHeight(42);

// Style avec padding droit pour laisser la place au triangle
m_comboBoxType->setStyleSheet(R"(
    QComboBox {
        border: 2px solid #E2E8F0;
        border-radius: 8px;
        padding: 5px 36px 5px 14px;
        font-size: 14px;
        background: #F7FAFC;
        color: #2D3748;
        min-height: 20px;
    }
    QComboBox:focus {
        border: 2px solid #2B6CB0;
    }
    QComboBox::drop-down {
        border: none;
        width: 30px;
        background: transparent;
    }
    QComboBox::down-arrow {
        image: none;
        border: none;
        width: 0px;
        height: 0px;
    }
    QComboBox QAbstractItemView {
        border: 1px solid #E2E8F0;
        border-radius: 8px;
        background: white;
        selection-background-color: #2B6CB0;
        selection-color: white;
        padding: 5px;
        outline: none;
    }
)");

formLayout->addRow(typeLabel, m_comboBoxType);

    // --------------------------------------------------------
    // 2.5 SECTION : ENTREPRISE (Conditionnelle)
    // --------------------------------------------------------
    m_entrepriseWidget = new QWidget;
    QFormLayout *entrepriseForm = new QFormLayout(m_entrepriseWidget);
    entrepriseForm->setSpacing(0);
    entrepriseForm->setContentsMargins(0, 12, 0, 0);
    entrepriseForm->setLabelAlignment(Qt::AlignLeft);

    // Nom entreprise
    QLabel *nomEntLabel = new QLabel("Nom entreprise *");
    nomEntLabel->setStyleSheet("font-size: 13px; font-weight: 600; color: #4A5568;");
    m_lineEditNomEntreprise = new QLineEdit;
    m_lineEditNomEntreprise->setPlaceholderText("Nom de l'entreprise");
    m_lineEditNomEntreprise->setFixedHeight(42);
    entrepriseForm->addRow(nomEntLabel, m_lineEditNomEntreprise);
    entrepriseForm->addItem(new QSpacerItem(0, 12, QSizePolicy::Minimum, QSizePolicy::Fixed));

    // ICE
    QLabel *iceLabel = new QLabel("ICE");
    iceLabel->setStyleSheet("font-size: 13px; font-weight: 600; color: #4A5568;");
    m_lineEditICE = new QLineEdit;
    m_lineEditICE->setPlaceholderText("Identifiant Commun de l'Entreprise");
    m_lineEditICE->setFixedHeight(42);
    entrepriseForm->addRow(iceLabel, m_lineEditICE);

    m_entrepriseWidget->setVisible(false);
    formLayout->addRow(m_entrepriseWidget);

    // ✅ ESPACE avant actions
    formLayout->addItem(new QSpacerItem(0, 20, QSizePolicy::Minimum, QSizePolicy::Fixed));

    // --------------------------------------------------------
    // 2.6 SECTION : FEEDBACK & ACTIONS
    // --------------------------------------------------------
    // Status
    m_statusLabel = new QLabel("");
    m_statusLabel->setAlignment(Qt::AlignCenter);
    m_statusLabel->setStyleSheet("color: #E53E3E; font-size: 12px; padding: 8px;");
    m_statusLabel->setWordWrap(true);
    formLayout->addRow(m_statusLabel);

    // Bouton S'enregistrer
    m_pushButtonEnregistrer = new QPushButton("S'enregistrer");
    m_pushButtonEnregistrer->setFixedHeight(46);
    m_pushButtonEnregistrer->setCursor(Qt::PointingHandCursor);
    formLayout->addRow(m_pushButtonEnregistrer);

    // ✅ ESPACE entre boutons
    formLayout->addItem(new QSpacerItem(0, 10, QSizePolicy::Minimum, QSizePolicy::Fixed));

    // Bouton Annuler
    m_pushButtonAnnuler = new QPushButton("Annuler");
    m_pushButtonAnnuler->setFixedHeight(42);
    m_pushButtonAnnuler->setCursor(Qt::PointingHandCursor);
    m_pushButtonAnnuler->setStyleSheet(R"(
        QPushButton {
            background: transparent;
            color: #718096;
            font-weight: 600;
            font-size: 14px;
            border: 2px solid #E2E8F0;
            border-radius: 8px;
        }
        QPushButton:hover {
            background: #F7FAFC;
            color: #4A5568;
            border-color: #CBD5E0;
        }
    )");
    formLayout->addRow(m_pushButtonAnnuler);

    // ✅ ESPACE
    formLayout->addItem(new QSpacerItem(0, 12, QSizePolicy::Minimum, QSizePolicy::Fixed));

    // Lien connexion
    QHBoxLayout *loginLinkLayout = new QHBoxLayout;
    loginLinkLayout->setAlignment(Qt::AlignCenter);

    QLabel *alreadyLabel = new QLabel("Déjà un compte ?");
    alreadyLabel->setStyleSheet("color: #718096; font-size: 13px;");

    QPushButton *loginLinkBtn = new QPushButton("Se connecter");
    loginLinkBtn->setStyleSheet(R"(
        QPushButton {
            background: transparent;
            color: #2B6CB0;
            border: none;
            font-size: 13px;
            font-weight: 600;
            text-decoration: underline;
        }
        QPushButton:hover {
            color: #1A365D;
        }
    )");
    loginLinkBtn->setCursor(Qt::PointingHandCursor);
    loginLinkBtn->setFlat(true);

    loginLinkLayout->addWidget(alreadyLabel);
    loginLinkLayout->addWidget(loginLinkBtn);

    QWidget *linkRow = new QWidget;
    linkRow->setLayout(loginLinkLayout);
    formLayout->addRow(linkRow);

    mainLayout->addWidget(contentWidget, 1);

    // ========================================================
    // 3. CONNEXIONS
    // ========================================================
    connect(m_comboBoxType, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [this](int index) {
        bool isEntreprise = (m_comboBoxType->itemData(index).toString() == "entreprise");
        m_entrepriseWidget->setVisible(isEntreprise);
        // ✅ Ajuster la taille quand on montre/cache
        adjustSize();
    });

    connect(m_showPasswordCheck, &QCheckBox::toggled,
            this, &RegisterDialog::togglePasswordVisibility);

    connect(m_pushButtonEnregistrer, &QPushButton::clicked,
            this, &RegisterDialog::on_enregistrerClicked);

    connect(m_pushButtonAnnuler, &QPushButton::clicked,
            this, &QDialog::reject);

    connect(loginLinkBtn, &QPushButton::clicked,
            this, &QDialog::reject);
}

// ============================================================
// STYLES
// ============================================================

void RegisterDialog::applyStyles()
{
    const QString lineEditStyle = R"(
        QLineEdit {
            border: 2px solid #E2E8F0;
            border-radius: 8px;
            padding: 5px 14px;
            font-size: 14px;
            background: #F7FAFC;
            color: #2D3748;
            min-height: 20px;
        }
        QLineEdit:focus {
            border: 2px solid #2B6CB0;
            background: white;
        }
        QLineEdit:hover {
            border: 2px solid #CBD5E0;
        }
    )";

    QList<QLineEdit*> lineEdits = {
        m_lineEditNom, m_lineEditPrenom, m_lineEditEmail,
        m_lineEditTelephone, m_lineEditAdresse, m_lineEditMotDePasse,
        m_lineEditConfirmation, m_lineEditNomEntreprise, m_lineEditICE
    };
    for (QLineEdit *le : lineEdits) {
        le->setStyleSheet(lineEditStyle);
    }

    m_comboBoxType->setStyleSheet(R"(
        QComboBox {
            border: 2px solid #E2E8F0;
            border-radius: 8px;
            padding: 5px 14px;
            font-size: 14px;
            background: #F7FAFC;
            color: #2D3748;
            min-height: 20px;
        }
        QComboBox:focus {
            border: 2px solid #2B6CB0;
        }
        QComboBox::drop-down {
            border: none;
            width: 30px;
        }
        QComboBox::down-arrow {
            width: 0;
            height: 0;
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

    m_pushButtonEnregistrer->setStyleSheet(R"(
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

// ============================================================
// SLOTS
// ============================================================

void RegisterDialog::togglePasswordVisibility(bool checked)
{
    QLineEdit::EchoMode mode = checked ? QLineEdit::Normal : QLineEdit::Password;
    m_lineEditMotDePasse->setEchoMode(mode);
    m_lineEditConfirmation->setEchoMode(mode);
}

// ============================================================
// VALIDATION
// ============================================================

bool RegisterDialog::validateInputs()
{
    m_statusLabel->clear();

    if (m_lineEditNom->text().trimmed().isEmpty() ||
        m_lineEditPrenom->text().trimmed().isEmpty()) {
        m_statusLabel->setText("❌ Nom et prénom sont obligatoires");
        return false;
    }

    QString email = m_lineEditEmail->text().trimmed();
    if (email.isEmpty()) {
        m_statusLabel->setText("❌ L'email est obligatoire");
        return false;
    }
    if (!email.endsWith("@gmail.com", Qt::CaseInsensitive)) {
        m_statusLabel->setText("❌ L'email doit être une adresse Gmail");
        return false;
    }

    QString mdp = m_lineEditMotDePasse->text();
    QString confirm = m_lineEditConfirmation->text();

    if (mdp.length() < 6) {
        m_statusLabel->setText("❌ Le mot de passe doit contenir au moins 6 caractères");
        return false;
    }
    if (mdp != confirm) {
        m_statusLabel->setText("❌ Les mots de passe ne correspondent pas");
        return false;
    }

    if (m_comboBoxType->currentData().toString() == "entreprise") {
        if (m_lineEditNomEntreprise->text().trimmed().isEmpty()) {
            m_statusLabel->setText("❌ Le nom de l'entreprise est obligatoire");
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
    client.nom = m_lineEditNom->text().trimmed();
    client.prenom = m_lineEditPrenom->text().trimmed();
    client.email = m_lineEditEmail->text().trimmed().toLower();
    client.adresse = m_lineEditAdresse->text().trimmed();
    client.telephone = m_lineEditTelephone->text().trimmed();
    client.type = m_comboBoxType->currentData().toString();
    client.nomEntreprise = m_lineEditNomEntreprise->text().trimmed();
    client.ice = m_lineEditICE->text().trimmed();
    client.motDePasse = hashPassword(m_lineEditMotDePasse->text());
    client.role = "client";

    QSqlQuery query;
    query.prepare("INSERT INTO clients (nom, prenom, email, adresse, telephone, "
                  "mot_de_passe, type, ice, nom_entreprise, role) "
                  "VALUES (:nom, :prenom, :email, :adresse, :telephone, "
                  ":mot_de_passe, :type, :ice, :nom_entreprise, :role)");

    query.bindValue(":nom", client.nom);
    query.bindValue(":prenom", client.prenom);
    query.bindValue(":email", client.email);
    query.bindValue(":adresse", client.adresse);
    query.bindValue(":telephone", client.telephone);
    query.bindValue(":mot_de_passe", client.motDePasse);
    query.bindValue(":type", client.type);
    query.bindValue(":ice", client.ice.isEmpty() ? QVariant() : client.ice);
    query.bindValue(":nom_entreprise", client.nomEntreprise.isEmpty() ? QVariant() : client.nomEntreprise);
    query.bindValue(":role", client.role);

    if (!query.exec()) {
        m_statusLabel->setText("❌ Erreur : " + query.lastError().text());
        return;
    }

    client.id = query.lastInsertId().toInt();

    EmailSender *sender = new EmailSender(this);
    sender->sendWelcomeEmail(client.email, client.prenom + " " + client.nom);

    QMessageBox::information(this, "Succès",
        QString("Compte créé avec succès !\nVous pouvez maintenant vous connecter."));

    emit registerSuccess(client);
    accept();
}

// ============================================================
// UTILITAIRES
// ============================================================

QString RegisterDialog::hashPassword(const QString &password)
{
    return QString(QCryptographicHash::hash(
        password.toUtf8(), QCryptographicHash::Sha256).toHex());
}

Client RegisterDialog::getClient() const
{
    Client client;
    client.nom = m_lineEditNom->text().trimmed();
    client.prenom = m_lineEditPrenom->text().trimmed();
    client.email = m_lineEditEmail->text().trimmed();
    client.adresse = m_lineEditAdresse->text().trimmed();
    client.telephone = m_lineEditTelephone->text().trimmed();
    client.type = m_comboBoxType->currentData().toString();
    client.nomEntreprise = m_lineEditNomEntreprise->text().trimmed();
    client.ice = m_lineEditICE->text().trimmed();
    client.role = "client";
    return client;
}