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
#include <QHBoxLayout>
#include <QCheckBox>
#include <QFrame>
#include <QScrollArea>
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
// UI SETUP - CARTE CENTRÉE PROFESSIONNELLE
// ============================================================

void RegisterDialog::setupUI()
{
    setWindowTitle("Créer un compte");
    setMinimumSize(520, 560);   // ← MODIFIÉ : 600→560
    resize(580, 680);           // ← MODIFIÉ : 720→680
    setWindowFlags(Qt::Dialog | Qt::WindowCloseButtonHint);

    // ===== FOND GRIS CLAIR =====
    setStyleSheet("background-color: #F0F2F5;");

    // ===== LAYOUT PRINCIPAL (centre la carte) =====
    QVBoxLayout *outerLayout = new QVBoxLayout(this);
    outerLayout->setContentsMargins(20, 20, 20, 20);
    outerLayout->setAlignment(Qt::AlignCenter);

    // ===== CARTE BLANCHE =====
    QFrame *card = new QFrame(this);
    card->setObjectName("registerCard");
    card->setStyleSheet(R"(
        #registerCard {
            background-color: white;
            border-radius: 16px;
            border: 1px solid #E2E8F0;
        }
    )");
    card->setMaximumWidth(520);
    card->setMinimumWidth(460);

    QVBoxLayout *cardLayout = new QVBoxLayout(card);
    cardLayout->setSpacing(0);
    cardLayout->setContentsMargins(0, 0, 0, 0);

    // ========================================================
    // 1. HEADER (arrondi en haut)
    // ========================================================
    QWidget *headerWidget = new QWidget(card);
    headerWidget->setFixedHeight(90);
    headerWidget->setStyleSheet(R"(
        background-color: #2B6CB0;
        border-top-left-radius: 16px;
        border-top-right-radius: 16px;
    )");

    QVBoxLayout *headerLayout = new QVBoxLayout(headerWidget);
    headerLayout->setAlignment(Qt::AlignCenter);
    headerLayout->setSpacing(4);
    headerLayout->setContentsMargins(20, 12, 20, 12);

    m_titleLabel = new QLabel("Créer un compte", headerWidget);
    m_titleLabel->setStyleSheet(
        "color: white; font-size: 22px; font-weight: bold; background: transparent;");
    m_titleLabel->setAlignment(Qt::AlignCenter);

    m_subtitleLabel = new QLabel("Rejoignez FacturationApp", headerWidget);
    m_subtitleLabel->setStyleSheet(
        "color: #E2E8F0; font-size: 12px; background: transparent;");
    m_subtitleLabel->setAlignment(Qt::AlignCenter);

    headerLayout->addWidget(m_titleLabel);
    headerLayout->addWidget(m_subtitleLabel);
    cardLayout->addWidget(headerWidget);

    // ========================================================
    // 2. CONTENU SCROLLABLE
    // ========================================================
    QScrollArea *scrollArea = new QScrollArea(card);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollArea->setStyleSheet(R"(
        QScrollArea { background: transparent; border: none; }
        QScrollBar:vertical {
            background: transparent;
            width: 6px;
            margin: 0px;
        }
        QScrollBar::handle:vertical {
            background: #CBD5E0;
            border-radius: 3px;
            min-height: 30px;
        }
        QScrollBar::handle:vertical:hover { background: #A0AEC0; }
        QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {
            height: 0px;
        }
    )");

    QWidget *scrollContent = new QWidget;
    QVBoxLayout *formLayout = new QVBoxLayout(scrollContent);
    formLayout->setSpacing(0);
    formLayout->setContentsMargins(32, 24, 32, 16);
    formLayout->setAlignment(Qt::AlignTop);

    // --------------------------------------------------------
    // HELPER : Créer un champ (label + input)
    // --------------------------------------------------------
    auto addField = [&](const QString &labelText, const QString &placeholder,
                      QLineEdit *&fieldRef, bool isPassword = false) {
        QLabel *label = new QLabel(labelText);
        label->setStyleSheet("font-size: 13px; font-weight: 600; color: #4A5568; margin-bottom: 4px;");
        formLayout->addWidget(label);

        fieldRef = new QLineEdit;
        fieldRef->setPlaceholderText(placeholder);
        fieldRef->setFixedHeight(42);
        if (isPassword) fieldRef->setEchoMode(QLineEdit::Password);
        formLayout->addWidget(fieldRef);
    };

    // --------------------------------------------------------
    // 2.1 NOM + PRÉNOM (côte à côte)
    // --------------------------------------------------------
    QHBoxLayout *identityLayout = new QHBoxLayout;
    identityLayout->setSpacing(12);

    // Nom
    QVBoxLayout *nomLayout = new QVBoxLayout;
    nomLayout->setSpacing(4);
    QLabel *nomLabel = new QLabel("Nom *");
    nomLabel->setStyleSheet("font-size: 13px; font-weight: 600; color: #4A5568;");
    m_lineEditNom = new QLineEdit;
    m_lineEditNom->setPlaceholderText("Votre nom");
    m_lineEditNom->setFixedHeight(42);
    nomLayout->addWidget(nomLabel);
    nomLayout->addWidget(m_lineEditNom);
    identityLayout->addLayout(nomLayout, 1);

    // Prénom
    QVBoxLayout *prenomLayout = new QVBoxLayout;
    prenomLayout->setSpacing(4);
    QLabel *prenomLabel = new QLabel("Prénom *");
    prenomLabel->setStyleSheet("font-size: 13px; font-weight: 600; color: #4A5568;");
    m_lineEditPrenom = new QLineEdit;
    m_lineEditPrenom->setPlaceholderText("Votre prénom");
    m_lineEditPrenom->setFixedHeight(42);
    prenomLayout->addWidget(prenomLabel);
    prenomLayout->addWidget(m_lineEditPrenom);
    identityLayout->addLayout(prenomLayout, 1);

    formLayout->addLayout(identityLayout);
    formLayout->addSpacing(14);

    // --------------------------------------------------------
    // 2.2 CONTACT
    // --------------------------------------------------------
    addField("Email *", "exemple@gmail.com", m_lineEditEmail);
    formLayout->addSpacing(14);

    addField("Téléphone", "+212 6XX XXX XXX", m_lineEditTelephone);
    formLayout->addSpacing(14);

    addField("Adresse", "Votre adresse complète", m_lineEditAdresse);
    formLayout->addSpacing(14);

    // --------------------------------------------------------
    // 2.3 SÉCURITÉ
    // --------------------------------------------------------
    addField("Mot de passe *", "Min. 6 caractères", m_lineEditMotDePasse, true);
    formLayout->addSpacing(14);

    // Confirmation + Checkbox
    QLabel *confirmLabel = new QLabel("Confirmation *");
    confirmLabel->setStyleSheet("font-size: 13px; font-weight: 600; color: #4A5568; margin-bottom: 4px;");
    formLayout->addWidget(confirmLabel);

    QHBoxLayout *confirmLayout = new QHBoxLayout;
    confirmLayout->setSpacing(10);

    m_lineEditConfirmation = new QLineEdit;
    m_lineEditConfirmation->setPlaceholderText("Confirmer le mot de passe");
    m_lineEditConfirmation->setEchoMode(QLineEdit::Password);
    m_lineEditConfirmation->setFixedHeight(42);

    m_showPasswordCheck = new QCheckBox("Afficher");
    m_showPasswordCheck->setStyleSheet("color: #718096; font-size: 12px;");

    confirmLayout->addWidget(m_lineEditConfirmation, 1);
    confirmLayout->addWidget(m_showPasswordCheck);
    formLayout->addLayout(confirmLayout);
    formLayout->addSpacing(14);

    // --------------------------------------------------------
    // 2.4 TYPE DE COMPTE
    // --------------------------------------------------------
    QLabel *typeLabel = new QLabel("Type de compte *");
    typeLabel->setStyleSheet("font-size: 13px; font-weight: 600; color: #4A5568; margin-bottom: 4px;");
    formLayout->addWidget(typeLabel);

    m_comboBoxType = new QComboBox;
    m_comboBoxType->addItem("Personne", "personne");
    m_comboBoxType->addItem("Entreprise", "entreprise");
    m_comboBoxType->setFixedHeight(42);

    m_comboBoxType->setStyleSheet(R"(
        QComboBox {
            border: 2px solid #E2E8F0;
            border-radius: 10px;
            padding: 5px 14px;
            font-size: 14px;
            background: #F7FAFC;
            color: #2D3748;
            min-height: 20px;
        }
        QComboBox:focus {
            border: 2px solid #2B6CB0;
            background: white;
        }
        QComboBox::drop-down {
            border: none;
            width: 36px;
            background: transparent;
        }
        QComboBox::down-arrow {
            image: none;
            border-left: 5px solid transparent;
            border-right: 5px solid transparent;
            border-top: 7px solid #718096;
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

    formLayout->addWidget(m_comboBoxType);
    formLayout->addSpacing(14);

    // --------------------------------------------------------
    // 2.5 ENTREPRISE (Conditionnelle) — MODIFIÉ
    // --------------------------------------------------------
    m_entrepriseWidget = new QWidget;
    QVBoxLayout *entrepriseLayout = new QVBoxLayout(m_entrepriseWidget);
    entrepriseLayout->setSpacing(0);
    entrepriseLayout->setContentsMargins(0, 0, 0, 0);

    // Label + champ Nom entreprise (encapsulés dans le widget)
    QLabel *nomEntLabel = new QLabel("Nom entreprise *");
    nomEntLabel->setStyleSheet("font-size: 13px; font-weight: 600; color: #4A5568; margin-bottom: 4px;");
    entrepriseLayout->addWidget(nomEntLabel);

    m_lineEditNomEntreprise = new QLineEdit;
    m_lineEditNomEntreprise->setPlaceholderText("Nom de l'entreprise");
    m_lineEditNomEntreprise->setFixedHeight(42);
    entrepriseLayout->addWidget(m_lineEditNomEntreprise);
    entrepriseLayout->addSpacing(14);

    // Label + champ ICE (encapsulés dans le widget)
    QLabel *iceLabel = new QLabel("ICE");
    iceLabel->setStyleSheet("font-size: 13px; font-weight: 600; color: #4A5568; margin-bottom: 4px;");
    entrepriseLayout->addWidget(iceLabel);

    m_lineEditICE = new QLineEdit;
    m_lineEditICE->setPlaceholderText("Identifiant Commun de l'Entreprise");
    m_lineEditICE->setFixedHeight(42);
    entrepriseLayout->addWidget(m_lineEditICE);

    m_entrepriseWidget->setVisible(false);
    formLayout->addWidget(m_entrepriseWidget);

    // --------------------------------------------------------
    // 2.6 LIEN CONNEXION
    // --------------------------------------------------------
    formLayout->addSpacing(8);

    QHBoxLayout *loginLinkLayout = new QHBoxLayout;
    loginLinkLayout->setAlignment(Qt::AlignCenter);

    QLabel *alreadyLabel = new QLabel("Déjà un compte ?");
    alreadyLabel->setStyleSheet("color: #718096; font-size: 13px;");

    QPushButton *loginLinkBtn = new QPushButton("Se connecter");
    loginLinkBtn->setStyleSheet(R"(
        QPushButton { background: transparent; color: #2B6CB0; border: none;
            font-size: 13px; font-weight: 600; text-decoration: underline; }
        QPushButton:hover { color: #1A365D; }
    )");
    loginLinkBtn->setCursor(Qt::PointingHandCursor);
    loginLinkBtn->setFlat(true);

    loginLinkLayout->addWidget(alreadyLabel);
    loginLinkLayout->addWidget(loginLinkBtn);
    formLayout->addLayout(loginLinkLayout);

    scrollArea->setWidget(scrollContent);
    cardLayout->addWidget(scrollArea, 1);

       // ========================================================
    // 3. FOOTER FIXE DANS LA CARTE (boutons toujours visibles)
    // ========================================================
    QWidget *footerWidget = new QWidget(card);
    footerWidget->setStyleSheet("background-color: white; border-bottom-left-radius: 16px; border-bottom-right-radius: 16px;");
    footerWidget->setFixedHeight(80);  // Hauteur ajustée pour layout horizontal

    QVBoxLayout *footerLayout = new QVBoxLayout(footerWidget);
    footerLayout->setContentsMargins(32, 8, 32, 16);
    footerLayout->setSpacing(8);

    // Label de statut (erreur)
    m_statusLabel = new QLabel("");
    m_statusLabel->setAlignment(Qt::AlignCenter);
    m_statusLabel->setStyleSheet("color: #E53E3E; font-size: 12px; padding: 2px;");
    m_statusLabel->setWordWrap(true);
    footerLayout->addWidget(m_statusLabel);

    // LAYOUT HORIZONTAL pour les boutons côte à côte
    QHBoxLayout *buttonsLayout = new QHBoxLayout;
    buttonsLayout->setSpacing(12);

    // Bouton Annuler (gauche)
    m_pushButtonAnnuler = new QPushButton("Annuler");
    m_pushButtonAnnuler->setFixedHeight(40);
    m_pushButtonAnnuler->setMinimumWidth(120);
    m_pushButtonAnnuler->setCursor(Qt::PointingHandCursor);
    m_pushButtonAnnuler->setStyleSheet(R"(
        QPushButton {
            background: white;
            color: #4A5568;
            font-weight: 600;
            font-size: 14px;
            border: 2px solid #E2E8F0;
            border-radius: 10px;
        }
        QPushButton:hover {
            background: #F7FAFC;
            color: #2D3748;
            border-color: #CBD5E0;
        }
        QPushButton:pressed {
            background: #EDF2F7;
        }
    )");

    // Bouton S'enregistrer (droite)
    m_pushButtonEnregistrer = new QPushButton("S'enregistrer");
    m_pushButtonEnregistrer->setFixedHeight(40);
    m_pushButtonEnregistrer->setMinimumWidth(120);
    m_pushButtonEnregistrer->setCursor(Qt::PointingHandCursor);

    buttonsLayout->addWidget(m_pushButtonAnnuler, 1);   // stretch = 1
    buttonsLayout->addWidget(m_pushButtonEnregistrer, 1); // stretch = 1

    footerLayout->addLayout(buttonsLayout);

    cardLayout->addWidget(footerWidget);

    // ===== AJOUTER LA CARTE AU LAYOUT PRINCIPAL =====
    outerLayout->addWidget(card, 0, Qt::AlignCenter);

    // ========================================================
    // 4. CONNEXIONS
    // ========================================================
    connect(m_comboBoxType, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [this](int index) {
        Q_UNUSED(index)
        bool isEntreprise = (m_comboBoxType->currentData().toString() == "entreprise");
        m_entrepriseWidget->setVisible(isEntreprise);
    });

    connect(m_showPasswordCheck, &QCheckBox::toggled,
            this, &RegisterDialog::togglePasswordVisibility);

    connect(m_pushButtonEnregistrer, &QPushButton::clicked,
            this, &RegisterDialog::on_enregistrerClicked);

    connect(m_pushButtonAnnuler, &QPushButton::clicked,
            this, &RegisterDialog::goToLogin);

    connect(loginLinkBtn, &QPushButton::clicked,
            this, &RegisterDialog::goToLogin);
}

// ============================================================
// STYLES
// ============================================================

void RegisterDialog::applyStyles()
{
    const QString fieldStyle = R"(
        QLineEdit {
            border: 2px solid #E2E8F0;
            border-radius: 10px;
            padding: 5px 14px;
            font-size: 14px;
            background: #F7FAFC;
            color: #2D3748;
            min-height: 18px;
        }
        QLineEdit:focus {
            border: 2px solid #2B6CB0;
            background: white;
        }
        QLineEdit:hover {
            border: 2px solid #CBD5E0;
        }
    )";

    QList<QLineEdit*> allFields = {
        m_lineEditNom, m_lineEditPrenom, m_lineEditEmail,
        m_lineEditTelephone, m_lineEditAdresse, m_lineEditMotDePasse,
        m_lineEditConfirmation, m_lineEditNomEntreprise, m_lineEditICE
    };
    for (QLineEdit *field : allFields) {
        field->setStyleSheet(fieldStyle);
    }

    m_pushButtonEnregistrer->setStyleSheet(R"(
        QPushButton {
            background: #2B6CB0; color: white; font-weight: bold;
            font-size: 15px; border: none; border-radius: 10px;
        }
        QPushButton:hover { background: #1A365D; }
        QPushButton:pressed { background: #2C5282; }
        QPushButton:disabled { background: #A0AEC0; }
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
    emit goToLogin();
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