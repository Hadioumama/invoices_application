#include "logindialog.h"
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QCryptographicHash>
#include <QSqlQuery>
#include <QRegularExpression>
#include <QInputDialog>
#include <QRandomGenerator>
#include <QTimer>
#include <QSqlError>
#include <QGraphicsDropShadowEffect>
#include <QFrame>

LoginDialog::LoginDialog(QWidget *parent) : QDialog(parent),
    m_countdownTimer(nullptr), m_remainingSeconds(0), m_attemptCount(0)
{
    setupUI();
    applyStyles();
}

void LoginDialog::setupUI()
{
    setWindowTitle("Connexion");
    setFixedSize(420, 580);
    setWindowFlags(Qt::Widget);

    // Layout principal
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setAlignment(Qt::AlignCenter);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    // ===== CARTE CENTRALE =====
    QFrame *card = new QFrame(this);
    card->setFixedSize(380, 520);
    card->setStyleSheet("background: white; border-radius: 16px;");
    
    // Ombre portée
    QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(card);
    shadow->setBlurRadius(25);
    shadow->setColor(QColor(0, 0, 0, 30));
    shadow->setOffset(0, 4);
    card->setGraphicsEffect(shadow);

    QVBoxLayout *cardLayout = new QVBoxLayout(card);
    cardLayout->setSpacing(0);
    cardLayout->setContentsMargins(35, 35, 35, 30);
   cardLayout->setAlignment(Qt::AlignVCenter | Qt::AlignHCenter);

    // ===== LOGO (sans emoji, utilisation d'un QLabel stylisé) =====
    QLabel *logoLabel = new QLabel("FA");
    logoLabel->setFixedSize(56, 56);
    logoLabel->setAlignment(Qt::AlignCenter);
    logoLabel->setStyleSheet(
        "background: #2B6CB0; color: white; font-size: 22px; "
        "font-weight: bold; border-radius: 12px;");
    
    // Container pour centrer le logo
    QHBoxLayout *logoContainer = new QHBoxLayout;
    logoContainer->addStretch();
    logoContainer->addWidget(logoLabel);
    logoContainer->addStretch();
    cardLayout->addLayout(logoContainer);
    cardLayout->addSpacing(16);

    // ===== TITRE =====
    QLabel *titleLabel = new QLabel("Connexion");
    titleLabel->setStyleSheet(
        "font-size: 26px; font-weight: bold; color: #1A202C; background: transparent;");
    titleLabel->setAlignment(Qt::AlignCenter);
    cardLayout->addWidget(titleLabel);

    // ===== SOUS-TITRE =====
    QLabel *subtitleLabel = new QLabel("Accédez à votre espace");
    subtitleLabel->setStyleSheet(
        "font-size: 14px; color: #718096; background: transparent;");
    subtitleLabel->setAlignment(Qt::AlignCenter);
    cardLayout->addWidget(subtitleLabel);
    cardLayout->addSpacing(28);

    // ===== EMAIL =====
    QLabel *emailLabel = new QLabel("Adresse email");
    emailLabel->setStyleSheet(
        "font-size: 13px; font-weight: 600; color: #4A5568; background: transparent; "
        "margin-bottom: 6px;");
    cardLayout->addWidget(emailLabel);

    emailEdit = new QLineEdit;
    emailEdit->setPlaceholderText("votre.email@gmail.com");
    emailEdit->setFixedHeight(44);
    cardLayout->addWidget(emailEdit);
    cardLayout->addSpacing(16);

    // ===== MOT DE PASSE =====
    QLabel *passLabel = new QLabel("Mot de passe");
    passLabel->setStyleSheet(
        "font-size: 13px; font-weight: 600; color: #4A5568; background: transparent; "
        "margin-bottom: 6px;");
    cardLayout->addWidget(passLabel);

    // Container mot de passe + bouton œil
    QWidget *pwdContainer = new QWidget;
    pwdContainer->setFixedHeight(44);
    QHBoxLayout *pwdLayout = new QHBoxLayout(pwdContainer);
    pwdLayout->setSpacing(0);
    pwdLayout->setContentsMargins(0, 0, 0, 0);

    passwordEdit = new QLineEdit;
    passwordEdit->setEchoMode(QLineEdit::Password);
    passwordEdit->setPlaceholderText("••••••••");
    passwordEdit->setFixedHeight(44);
    
    togglePwdButton = new QPushButton("👁");
    togglePwdButton->setFixedSize(40, 44);
    togglePwdButton->setCursor(Qt::PointingHandCursor);
    togglePwdButton->setCheckable(true);
    
    pwdLayout->addWidget(passwordEdit);
    pwdLayout->addWidget(togglePwdButton);
    cardLayout->addWidget(pwdContainer);

    // ===== MOT DE PASSE OUBLIÉ =====
    QHBoxLayout *forgotLayout = new QHBoxLayout;
    forgotLayout->addStretch();
    
    forgotButton = new QPushButton("Mot de passe oublié ?");
    forgotButton->setCursor(Qt::PointingHandCursor);
    forgotButton->setFlat(true);
    forgotLayout->addWidget(forgotButton);
    cardLayout->addLayout(forgotLayout);
    cardLayout->addSpacing(20);

    // ===== BOUTON CONNEXION =====
    loginButton = new QPushButton("Se connecter");
    loginButton->setFixedHeight(46);
    loginButton->setCursor(Qt::PointingHandCursor);
    cardLayout->addWidget(loginButton);
    cardLayout->addSpacing(18);

    // ===== SÉPARATEUR =====
    QHBoxLayout *sepLayout = new QHBoxLayout;
    sepLayout->setSpacing(12);
    
    QWidget *lineLeft = new QWidget;
    lineLeft->setFixedHeight(1);
    lineLeft->setStyleSheet("background: #E2E8F0;");
    
    QLabel *ouLabel = new QLabel("ou");
    ouLabel->setStyleSheet("color: #A0AEC0; font-size: 13px; background: transparent;");
    
    QWidget *lineRight = new QWidget;
    lineRight->setFixedHeight(1);
    lineRight->setStyleSheet("background: #E2E8F0;");
    
    sepLayout->addWidget(lineLeft, 1);
    sepLayout->addWidget(ouLabel);
    sepLayout->addWidget(lineRight, 1);
    cardLayout->addLayout(sepLayout);
    cardLayout->addSpacing(14);

    // ===== BOUTON CRÉER COMPTE =====
    createButton = new QPushButton("Créer un compte");
    createButton->setFixedHeight(44);
    createButton->setCursor(Qt::PointingHandCursor);
    cardLayout->addWidget(createButton);
    cardLayout->addStretch();

    // ===== FOOTER =====
    QLabel *footerLabel = new QLabel("© 2026 FacturationApp");
footerLabel->setStyleSheet(
    "color: #A0AEC0; font-size: 11px; background: transparent; "
    "margin-top: 8px; padding-bottom: 4px;");
    footerLabel->setAlignment(Qt::AlignCenter);
    cardLayout->addWidget(footerLabel);

    // Centrer la carte dans la fenêtre
    mainLayout->addStretch();
    mainLayout->addWidget(card, 0, Qt::AlignCenter);
    mainLayout->addStretch();

    // ===== CONNEXIONS =====
    connect(loginButton, &QPushButton::clicked, this, &LoginDialog::onLogin);
    connect(createButton, &QPushButton::clicked, this, &LoginDialog::onCreateAccountClicked);
    connect(forgotButton, &QPushButton::clicked, this, &LoginDialog::onForgotPassword);
    connect(togglePwdButton, &QPushButton::toggled, this, &LoginDialog::togglePasswordVisibility);
}

void LoginDialog::applyStyles()
{
    // Fond de la fenêtre
    setStyleSheet("background: #EDF2F7;");

    // Style des labels
    QString labelStyle = "font-size: 13px; font-weight: 600; color: #4A5568; background: transparent;";

    // Style QLineEdit standard
    QString inputStyle = R"(
        QLineEdit {
            border: 1.5px solid #E2E8F0;
            border-radius: 10px;
            padding: 0 14px;
            font-size: 14px;
            background: #F7FAFC;
            color: #2D3748;
            selection-background-color: #2B6CB0;
        }
        QLineEdit:focus {
            border: 1.5px solid #2B6CB0;
            background: white;
        }
        QLineEdit:hover {
            border: 1.5px solid #CBD5E0;
        }
    )";
    
    emailEdit->setStyleSheet(inputStyle);
    passwordEdit->setStyleSheet(inputStyle);

    // Style bouton œil
    togglePwdButton->setStyleSheet(R"(
        QPushButton {
            background: transparent;
            border: 1.5px solid #E2E8F0;
            border-left: none;
            border-radius: 0 10px 10px 0;
            font-size: 14px;
            color: #718096;
        }
        QPushButton:hover {
            background: #F7FAFC;
            color: #4A5568;
        }
        QPushButton:checked {
            background: #EDF2F7;
            color: #2B6CB0;
        }
    )");

    // Bouton mot de passe oublié
    forgotButton->setStyleSheet(R"(
        QPushButton {
            background: transparent;
            color: #2B6CB0;
            border: none;
            font-size: 12px;
            font-weight: 500;
            padding: 4px 0;
        }
        QPushButton:hover {
            color: #1A365D;
            text-decoration: underline;
        }
    )");

    // Bouton connexion principal
    loginButton->setStyleSheet(R"(
        QPushButton {
            background: #2B6CB0;
            color: white;
            font-weight: bold;
            font-size: 15px;
            border: none;
            border-radius: 10px;
        }
        QPushButton:hover {
            background: #1A365D;
        }
        QPushButton:pressed {
            background: #2C5282;
        }
    )");

    // Bouton créer compte
    createButton->setStyleSheet(R"(
        QPushButton {
            background: white;
            color: #4A5568;
            font-weight: 600;
            font-size: 14px;
            border: 1.5px solid #E2E8F0;
            border-radius: 10px;
        }
        QPushButton:hover {
            background: #F7FAFC;
            border-color: #CBD5E0;
        }
        QPushButton:pressed {
            background: #EDF2F7;
        }
    )");
}

void LoginDialog::togglePasswordVisibility(bool checked)
{
    passwordEdit->setEchoMode(checked ? QLineEdit::Normal : QLineEdit::Password);
    togglePwdButton->setText(checked ? "🙈" : "👁");
}

void LoginDialog::clearFields()
{
    emailEdit->clear();
    passwordEdit->clear();
    togglePwdButton->setChecked(false);
    togglePasswordVisibility(false);
}

// ===== MÉTHODES EXISTANTES =====

bool LoginDialog::isValidEmail(const QString &email)
{
    QRegularExpression regex(R"(^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,}$)");
    return regex.match(email).hasMatch();
}

bool LoginDialog::isGmailEmail(const QString &email)
{
    if (!isValidEmail(email)) return false;
    QString domain = email.mid(email.lastIndexOf("@") + 1).toLower();
    return domain == "gmail.com";
}

QString LoginDialog::hashPassword(const QString &password)
{
    return QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Sha256).toHex();
}

QString LoginDialog::generateVerificationCode()
{
    int code = QRandomGenerator::global()->bounded(100000, 1000000);
    return QString::number(code);
}

bool LoginDialog::sendVerificationCode(const QString &email, const QString &code)
{
    Q_UNUSED(email)
    Q_UNUSED(code)
    return true;
}

void LoginDialog::onForgotPassword()
{
    QString email = emailEdit->text().trimmed().toLower();

    if (email.isEmpty()) {
        QMessageBox::warning(this, "Erreur",
            "Veuillez saisir votre email dans le champ ci-dessus.");
        emailEdit->setFocus();
        return;
    }

    if (!isGmailEmail(email)) {
        QMessageBox::warning(this, "Erreur",
            "Seules les adresses @gmail.com peuvent réinitialiser leur mot de passe.");
        return;
    }

    QSqlQuery check;
    check.prepare("SELECT id, nom FROM clients WHERE email = ?");
    check.addBindValue(email);

    if (!check.exec() || !check.next()) {
        QMessageBox::warning(this, "Erreur",
            "Cet email n'existe pas dans notre système.");
        return;
    }

    m_pendingEmail = email;
    m_verificationCode = generateVerificationCode();
    m_attemptCount = 0;

    if (!sendVerificationCode(email, m_verificationCode)) {
        QMessageBox::critical(this, "Erreur",
            "Impossible d'envoyer le code de vérification.");
        return;
    }

    showVerificationDialog(email);
}

void LoginDialog::showVerificationDialog(const QString &email)
{
    QDialog verifyDialog(this);
    verifyDialog.setWindowTitle("Vérification");
    verifyDialog.setFixedSize(320, 220);

    QVBoxLayout *layout = new QVBoxLayout(&verifyDialog);
    layout->setSpacing(12);
    layout->setContentsMargins(20, 20, 20, 20);

    QLabel *infoLabel = new QLabel(
        QString("Un code à 6 chiffres a été envoyé à :\n%1").arg(email));
    infoLabel->setStyleSheet("color:#4A5568; font-size:12px;");
    infoLabel->setWordWrap(true);
    infoLabel->setAlignment(Qt::AlignCenter);

    QLineEdit *codeEdit = new QLineEdit;
    codeEdit->setPlaceholderText("000000");
    codeEdit->setMaxLength(6);
    codeEdit->setAlignment(Qt::AlignCenter);
    codeEdit->setStyleSheet(
        "font-size:18px; letter-spacing:8px; padding:10px;"
        "border:2px solid #CBD5E0; border-radius:8px;");

    QPushButton *verifyBtn = new QPushButton("Vérifier");
    verifyBtn->setStyleSheet(
        "background:#2B6CB0; color:white; font-weight:bold; padding:10px; border-radius:8px;");

    layout->addWidget(infoLabel);
    layout->addWidget(codeEdit);
    layout->addWidget(verifyBtn);

    connect(verifyBtn, &QPushButton::clicked, [&]() {
        if (codeEdit->text() == m_verificationCode) {
            verifyDialog.accept();
            showPasswordResetDialog(email);
        } else {
            QMessageBox::warning(&verifyDialog, "Erreur", "Code incorrect.");
        }
    });

    verifyDialog.exec();
}

void LoginDialog::showPasswordResetDialog(const QString &email)
{
    QDialog resetDialog(this);
    resetDialog.setWindowTitle("Nouveau mot de passe");
    resetDialog.setFixedSize(350, 200);

    QVBoxLayout *layout = new QVBoxLayout(&resetDialog);
    layout->setSpacing(12);
    layout->setContentsMargins(20, 20, 20, 20);

    QLineEdit *newPwdEdit = new QLineEdit;
    newPwdEdit->setEchoMode(QLineEdit::Password);
    newPwdEdit->setPlaceholderText("Nouveau mot de passe (min 6 caractères)");

    QLineEdit *confirmPwdEdit = new QLineEdit;
    confirmPwdEdit->setEchoMode(QLineEdit::Password);
    confirmPwdEdit->setPlaceholderText("Confirmer le mot de passe");

    QPushButton *saveBtn = new QPushButton("Enregistrer");
    saveBtn->setStyleSheet(
        "background:#2B6CB0; color:white; font-weight:bold; padding:10px; border-radius:8px;");

    layout->addWidget(new QLabel("Nouveau mot de passe :"));
    layout->addWidget(newPwdEdit);
    layout->addWidget(new QLabel("Confirmation :"));
    layout->addWidget(confirmPwdEdit);
    layout->addWidget(saveBtn);

    connect(saveBtn, &QPushButton::clicked, [&]() {
        if (newPwdEdit->text().length() < 6) {
            QMessageBox::warning(&resetDialog, "Erreur", "Minimum 6 caractères.");
            return;
        }
        if (newPwdEdit->text() != confirmPwdEdit->text()) {
            QMessageBox::warning(&resetDialog, "Erreur", "Les mots de passe ne correspondent pas.");
            return;
        }

        QSqlQuery update;
        update.prepare("UPDATE clients SET mot_de_passe = ? WHERE email = ?");
        update.addBindValue(hashPassword(newPwdEdit->text()));
        update.addBindValue(email);

        if (update.exec()) {
            QMessageBox::information(&resetDialog, "Succès",
                "Votre mot de passe a été modifié !");
            resetDialog.accept();
        }
    });

    resetDialog.exec();
}

void LoginDialog::onLogin()
{
    QString email = emailEdit->text().trimmed().toLower();
    QString pwd = passwordEdit->text();

    qDebug() << "=== TENTATIVE LOGIN ===";
    qDebug() << "Email saisi:" << email;
    qDebug() << "Password saisi:" << pwd;

    if (email.isEmpty() || pwd.isEmpty()) {
        QMessageBox::warning(this, "Erreur", "Veuillez remplir tous les champs.");
        return;
    }

    QSqlQuery q;
    q.prepare("SELECT id, role, mot_de_passe FROM clients WHERE LOWER(email) = LOWER(?)");
    q.addBindValue(email);

    if (!q.exec()) {
        qDebug() << "ERREUR SQL:" << q.lastError().text();
        QMessageBox::warning(this, "Erreur", "Problème de base de données.");
        return;
    }

    if (!q.next()) {
        qDebug() << "Email NON TROUVÉ:" << email;
        QMessageBox::warning(this, "Erreur", "Email ou mot de passe incorrect.");
        passwordEdit->clear();
        return;
    }

    int userId = q.value(0).toInt();
    QString role = q.value(1).toString();
    QString dbHash = q.value(2).toString();

    qDebug() << "User trouvé - ID:" << userId << "Role:" << role;
    qDebug() << "Hash BDD:" << dbHash;
    qDebug() << "Hash saisi:" << hashPassword(pwd);

    QString hashedInput = hashPassword(pwd);
    bool match = (dbHash == hashedInput) || (dbHash == pwd);

    qDebug() << "Match:" << match;

    if (!match) {
        QMessageBox::warning(this, "Erreur", "Email ou mot de passe incorrect.");
        passwordEdit->clear();
        passwordEdit->setFocus();
        return;
    }

    if (dbHash == pwd) {
        QString newHash = hashPassword(pwd);
        QSqlQuery update;
        update.prepare("UPDATE clients SET mot_de_passe = ? WHERE id = ?");
        update.addBindValue(newHash);
        update.addBindValue(userId);
        update.exec();
    }

    qDebug() << "EMISSION loginSuccess - ID:" << userId << "Role:" << role;
    emit loginSuccess(userId, role);
}

void LoginDialog::onCreateAccountClicked()
{
    emit createAccountRequested();
}

QString LoginDialog::getEmail() const {
    return emailEdit->text().trimmed().toLower();
}

QString LoginDialog::getPassword() const {
    return passwordEdit->text();
}