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
#include <QProgressDialog>
#include "views/adminwindow.h"
#include "utils/emailsender.h"
#include "views/clientwindow.h"

LoginDialog::LoginDialog(QWidget *parent) : QDialog(parent),
    m_countdownTimer(nullptr), m_remainingSeconds(0), m_attemptCount(0)
{
    setWindowTitle("Connexion");
    
    // ✅ TAILLE FIXE ET COMPACTE
    setFixedSize(420, 380);
    
    // ✅ FOND CLAIR PROFESSIONNEL
    setStyleSheet("background: #F7FAFC;");
    
    // ✅ PAS DE BORDURE DE FENÊTRE (intégré dans MainWindow)
    setWindowFlags(Qt::Widget);  // Pas de Qt::Dialog, c'est un widget enfant

    // === TITRE ===
    QLabel *titleLabel = new QLabel("Connexion");
    titleLabel->setStyleSheet(
        "font-size: 22px; font-weight: bold; color: #2B6CB0;");
    titleLabel->setAlignment(Qt::AlignCenter);

    QLabel *subtitleLabel = new QLabel("Accedez a votre compte");
    subtitleLabel->setStyleSheet(
        "font-size: 12px; color: #718096;");
    subtitleLabel->setAlignment(Qt::AlignCenter);

    // === EMAIL ===
    QLabel *emailLabel = new QLabel("Email");
    emailLabel->setStyleSheet("font-size: 13px; font-weight: 600; color: #4A5568;");
    
    emailEdit = new QLineEdit;
    emailEdit->setPlaceholderText("exemple@gmail.com");
    emailEdit->setFixedHeight(38);
    emailEdit->setStyleSheet(
        "QLineEdit {"
        "  border: 2px solid #E2E8F0;"
        "  border-radius: 8px;"
        "  padding: 5px 12px;"
        "  font-size: 13px;"
        "  background: white;"
        "}"
        "QLineEdit:focus {"
        "  border: 2px solid #2B6CB0;"
        "}");

    // === MOT DE PASSE ===
    QLabel *passLabel = new QLabel("Mot de passe");
    passLabel->setStyleSheet("font-size: 13px; font-weight: 600; color: #4A5568;");
    
    passwordEdit = new QLineEdit;
    passwordEdit->setEchoMode(QLineEdit::Password);
    passwordEdit->setPlaceholderText("Votre mot de passe");
    passwordEdit->setFixedHeight(38);
    passwordEdit->setStyleSheet(
        "QLineEdit {"
        "  border: 2px solid #E2E8F0;"
        "  border-radius: 8px;"
        "  padding: 5px 12px;"
        "  font-size: 13px;"
        "  background: white;"
        "}"
        "QLineEdit:focus {"
        "  border: 2px solid #2B6CB0;"
        "}");

    // === MOT DE PASSE OUBLIÉ ===
    forgotButton = new QPushButton("Mot de passe oublie ?");
    forgotButton->setStyleSheet(
        "QPushButton {"
        "  background: transparent;"
        "  color: #2B6CB0;"
        "  border: none;"
        "  font-size: 12px;"
        "  text-decoration: underline;"
        "}"
        "QPushButton:hover { color: #1A365D; }");
    forgotButton->setCursor(Qt::PointingHandCursor);
    forgotButton->setFlat(true);

    // === BOUTON CONNEXION ===
    loginButton = new QPushButton("Se connecter");
    loginButton->setFixedHeight(42);
    loginButton->setCursor(Qt::PointingHandCursor);
    loginButton->setStyleSheet(
        "QPushButton {"
        "  background: #2B6CB0;"
        "  color: white;"
        "  font-weight: bold;"
        "  font-size: 14px;"
        "  border: none;"
        "  border-radius: 8px;"
        "}"
        "QPushButton:hover {"
        "  background: #1A365D;"
        "}"
        "QPushButton:pressed {"
        "  background: #2C5282;"
        "}");

    // === BOUTON CRÉER COMPTE ===
    createButton = new QPushButton("Creer un compte");
    createButton->setFixedHeight(38);
    createButton->setCursor(Qt::PointingHandCursor);
    createButton->setStyleSheet(
        "QPushButton {"
        "  background: transparent;"
        "  color: #4A5568;"
        "  border: 2px solid #CBD5E0;"
        "  font-size: 13px;"
        "  border-radius: 8px;"
        "}"
        "QPushButton:hover {"
        "  background: #EDF2F7;"
        "  border-color: #A0AEC0;"
        "}");

    // === LAYOUT ===
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setSpacing(8);
    layout->setContentsMargins(35, 25, 35, 25);

    layout->addWidget(titleLabel);
    layout->addWidget(subtitleLabel);
    layout->addSpacing(15);

    layout->addWidget(emailLabel);
    layout->addWidget(emailEdit);
    layout->addSpacing(5);

    layout->addWidget(passLabel);
    layout->addWidget(passwordEdit);
    layout->addWidget(forgotButton, 0, Qt::AlignRight);
    layout->addSpacing(15);

    layout->addWidget(loginButton);
    layout->addSpacing(8);
    layout->addWidget(createButton);

    // ✅ PAS DE STRETCH - le dialog a une taille fixe

    // === CONNEXIONS ===
    connect(loginButton, &QPushButton::clicked, this, &LoginDialog::onLogin);
    connect(createButton, &QPushButton::clicked, this, &LoginDialog::onCreateAccountClicked);
    connect(forgotButton, &QPushButton::clicked, this, &LoginDialog::onForgotPassword);
}
bool LoginDialog::isValidEmail(const QString &email)
{
    QRegularExpression regex(
        R"(^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,}$)");
    return regex.match(email).hasMatch();
}

bool LoginDialog::isGmailEmail(const QString &email)
{
    if (!isValidEmail(email))
        return false;
    QString domain = email.mid(email.lastIndexOf("@") + 1).toLower();
    return domain == "gmail.com";
}

QString LoginDialog::hashPassword(const QString &password)
{
    return QCryptographicHash::hash(
        password.toUtf8(),
        QCryptographicHash::Sha256).toHex();
}

// ✅ Génère un code à 6 chiffres (100000 - 999999)
QString LoginDialog::generateVerificationCode()
{
    int code = QRandomGenerator::global()->bounded(100000, 1000000);
    return QString::number(code);
}

bool LoginDialog::sendVerificationCode(const QString &email, const QString &code)
{
    EmailSender sender;
    return sender.sendSmtp(email, 
        "Code de vérification - FacturationApp",
        QString("Votre code de vérification est : %1\n"
                "Ce code est valide pendant 10 minutes.\n\n"
                "Si vous n'avez pas demandé cette réinitialisation, "
                "ignorez cet email.").arg(code));
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

    // Vérifier que l'email existe
    QSqlQuery check;
    check.prepare("SELECT id, nom FROM clients WHERE email = ?");
    check.addBindValue(email);

    if (!check.exec() || !check.next()) {
        QMessageBox::warning(this, "Erreur",
            "Cet email n\'existe pas dans notre système.");
        return;
    }

    // Générer et envoyer le code
    m_pendingEmail = email;
    m_verificationCode = generateVerificationCode();
    m_attemptCount = 0;

    if (!sendVerificationCode(email, m_verificationCode)) {
        QMessageBox::critical(this, "Erreur",
            "Impossible d\'envoyer le code de vérification.\n"
            "Veuillez réessayer plus tard.");
        return;
    }

    // Afficher le dialog de vérification
    showVerificationDialog(email);
}

void LoginDialog::showVerificationDialog(const QString &email)
{
    QDialog verifyDialog(this);
    verifyDialog.setWindowTitle("🔐 Vérification");
    verifyDialog.setMinimumSize(300, 200);

    QVBoxLayout *layout = new QVBoxLayout(&verifyDialog);
    layout->setSpacing(12);
    layout->setContentsMargins(20, 20, 20, 20);

    QLabel *infoLabel = new QLabel(
        QString("Un code à 6 chiffres a été envoyé à :\n%1\n\n"
                "Veuillez saisir le code ci-dessous.").arg(email));
    infoLabel->setStyleSheet("color:#4A5568; font-size:12px;");
    infoLabel->setWordWrap(true);
    infoLabel->setAlignment(Qt::AlignCenter);

    QLineEdit *codeEdit = new QLineEdit;
    codeEdit->setPlaceholderText("000000");
    codeEdit->setMaxLength(6);
    codeEdit->setAlignment(Qt::AlignCenter);
    codeEdit->setStyleSheet(
        "font-size:18px; letter-spacing:8px; padding:10px;"
        "border:2px solid #CBD5E0; border-radius:6px;");

    // N'accepter que les chiffres
    QRegularExpressionValidator *validator = 
        new QRegularExpressionValidator(QRegularExpression("^[0-9]{6}$"), this);
    codeEdit->setValidator(validator);

    QLabel *timerLabel = new QLabel("⏱️ Code valide pendant : 10:00");
    timerLabel->setStyleSheet("color:#718096; font-size:11px;");
    timerLabel->setAlignment(Qt::AlignCenter);

    QPushButton *verifyBtn = new QPushButton("✅ Vérifier");
    verifyBtn->setStyleSheet(
        "background:#27AE60; color:white; font-weight:bold; padding:10px;");

    QPushButton *resendBtn = new QPushButton("🔄 Renvoyer le code");
    resendBtn->setStyleSheet(
        "background:transparent; color:#3182CE; border:none;");
    resendBtn->setEnabled(false);

    QPushButton *cancelBtn = new QPushButton("❌ Annuler");
    cancelBtn->setStyleSheet(
        "background:#E53E3E; color:white; padding:8px;");

    layout->addWidget(infoLabel);
    layout->addSpacing(10);
    layout->addWidget(codeEdit);
    layout->addWidget(timerLabel);
    layout->addSpacing(10);
    layout->addWidget(verifyBtn);
    layout->addWidget(resendBtn, 0, Qt::AlignCenter);
    layout->addWidget(cancelBtn);

    // Timer compte à rebours
    m_remainingSeconds = 600;  // 10 minutes

    if (m_countdownTimer) {
        delete m_countdownTimer;
    }
    m_countdownTimer = new QTimer(this);

    connect(m_countdownTimer, &QTimer::timeout, [&]() {
        m_remainingSeconds--;
        int minutes = m_remainingSeconds / 60;
        int seconds = m_remainingSeconds % 60;
        timerLabel->setText(QString("⏱️ Code valide pendant : %1:%2")
            .arg(minutes).arg(seconds, 2, 10, QChar('0')));

        if (m_remainingSeconds <= 0) {
            m_countdownTimer->stop();
            verifyBtn->setEnabled(false);
            timerLabel->setText("❌ Code expiré");
            timerLabel->setStyleSheet("color:#E53E3E; font-size:11px;");
        }

        // Activer le bouton renvoyer après 60 secondes
        if (m_remainingSeconds <= 540) {  // Après 1 minute
            resendBtn->setEnabled(true);
        }
    });
    m_countdownTimer->start(1000);  // Mise à jour chaque seconde

    // Connexions
    connect(verifyBtn, &QPushButton::clicked, [&]() {
        QString enteredCode = codeEdit->text().trimmed();

        if (enteredCode.length() != 6) {
            QMessageBox::warning(&verifyDialog, "Erreur",
                "Veuillez saisir les 6 chiffres du code.");
            return;
        }

        m_attemptCount++;

        if (enteredCode != m_verificationCode) {
            if (m_attemptCount >= 3) {
                QMessageBox::critical(&verifyDialog, "Trop de tentatives",
                    "Vous avez dépassé le nombre de tentatives autorisées.\n"
                    "Veuillez demander un nouveau code.");
                verifyDialog.reject();
                return;
            }

            QMessageBox::warning(&verifyDialog, "Code incorrect",
                QString("Le code saisi est incorrect.\n"
                        "Tentative %1/3").arg(m_attemptCount));
            codeEdit->clear();
            codeEdit->setFocus();
            return;
        }

        // ✅ Code correct !
        m_countdownTimer->stop();
        verifyDialog.accept();

        // Afficher le dialog de changement de mot de passe
        showPasswordResetDialog(email);
    });

    connect(resendBtn, &QPushButton::clicked, [&]() {
        m_verificationCode = generateVerificationCode();
        sendVerificationCode(email, m_verificationCode);
        m_attemptCount = 0;
        m_remainingSeconds = 600;
        verifyBtn->setEnabled(true);
        timerLabel->setText("⏱️ Code valide pendant : 10:00");
        timerLabel->setStyleSheet("color:#718096; font-size:11px;");
        resendBtn->setEnabled(false);
        codeEdit->clear();
        codeEdit->setFocus();
        QMessageBox::information(&verifyDialog, "Code renvoyé",
            "Un nouveau code a été envoyé.");
    });

    connect(cancelBtn, &QPushButton::clicked, [&]() {
        m_countdownTimer->stop();
        verifyDialog.reject();
    });

    verifyDialog.exec();
}

void LoginDialog::showPasswordResetDialog(const QString &email)
{
    QDialog resetDialog(this);
    resetDialog.setWindowTitle("🔑 Nouveau mot de passe");
    resetDialog.setMinimumSize(350, 250);

    QVBoxLayout *layout = new QVBoxLayout(&resetDialog);
    layout->setSpacing(12);
    layout->setContentsMargins(20, 20, 20, 20);

    QLabel *titleLabel = new QLabel("Définissez votre nouveau mot de passe");
    titleLabel->setStyleSheet("font-size:14px; font-weight:bold; color:#2B6CB0;");
    titleLabel->setAlignment(Qt::AlignCenter);

    QLineEdit *newPwdEdit = new QLineEdit;
    newPwdEdit->setEchoMode(QLineEdit::Password);
    newPwdEdit->setPlaceholderText("Nouveau mot de passe (min 6 caractères)...");

    QLineEdit *confirmPwdEdit = new QLineEdit;
    confirmPwdEdit->setEchoMode(QLineEdit::Password);
    confirmPwdEdit->setPlaceholderText("Confirmer le mot de passe...");

    QLabel *strengthLabel = new QLabel("");
    strengthLabel->setAlignment(Qt::AlignCenter);

    QPushButton *saveBtn = new QPushButton("💾 Enregistrer");
    saveBtn->setStyleSheet(
        "background:#27AE60; color:white; font-weight:bold; padding:10px;");
    saveBtn->setEnabled(false);

    layout->addWidget(titleLabel);
    layout->addSpacing(10);
    layout->addWidget(new QLabel("Nouveau mot de passe :"));
    layout->addWidget(newPwdEdit);
    layout->addWidget(new QLabel("Confirmation :"));
    layout->addWidget(confirmPwdEdit);
    layout->addWidget(strengthLabel);
    layout->addSpacing(10);
    layout->addWidget(saveBtn);

    // Vérification de force du mot de passe
    auto checkPassword = [&]() {
        QString pwd = newPwdEdit->text();
        QString confirm = confirmPwdEdit->text();

        if (pwd.length() < 6) {
            strengthLabel->setText("❌ Trop court (min 6 caractères)");
            strengthLabel->setStyleSheet("color:#E53E3E;");
            saveBtn->setEnabled(false);
            return;
        }

        bool hasUpper = pwd.contains(QRegularExpression("[A-Z]"));
        bool hasLower = pwd.contains(QRegularExpression("[a-z]"));
        bool hasDigit = pwd.contains(QRegularExpression("[0-9]"));
        bool hasSpecial = pwd.contains(QRegularExpression("[!@#$%^&*]"));

        int strength = (hasUpper + hasLower + hasDigit + hasSpecial);

        if (strength < 2) {
            strengthLabel->setText("⚠️ Faible - Ajoutez majuscules, chiffres ou symboles");
            strengthLabel->setStyleSheet("color:#DD6B20;");
        } else if (strength < 4) {
            strengthLabel->setText("✅ Moyen");
            strengthLabel->setStyleSheet("color:#3182CE;");
        } else {
            strengthLabel->setText("💪 Fort");
            strengthLabel->setStyleSheet("color:#27AE60;");
        }

        if (pwd != confirm) {
            strengthLabel->setText("❌ Les mots de passe ne correspondent pas");
            strengthLabel->setStyleSheet("color:#E53E3E;");
            saveBtn->setEnabled(false);
            return;
        }

        saveBtn->setEnabled(true);
    };

    connect(newPwdEdit, &QLineEdit::textChanged, checkPassword);
    connect(confirmPwdEdit, &QLineEdit::textChanged, checkPassword);

    connect(saveBtn, &QPushButton::clicked, [&]() {
        QString newPwd = newPwdEdit->text();
        QString hashedPwd = hashPassword(newPwd);

        QSqlQuery update;
        update.prepare("UPDATE clients SET mot_de_passe = ? WHERE email = ?");
        update.addBindValue(hashedPwd);
        update.addBindValue(email);

        if (!update.exec()) {
            QMessageBox::critical(&resetDialog, "Erreur",
                "Impossible de mettre à jour le mot de passe.");
            return;
        }

        QMessageBox::information(&resetDialog, "✅ Succès",
            "Votre mot de passe a été modifié avec succès !\n"
            "Vous pouvez maintenant vous connecter.");
        resetDialog.accept();
    });

    resetDialog.exec();
}
void LoginDialog::onLogin()
{
    QString email = emailEdit->text().trimmed().toLower();
    QString pwd   = passwordEdit->text();

    if (email.isEmpty() || pwd.isEmpty()) {
        QMessageBox::warning(this, "Erreur",
            "Veuillez remplir tous les champs.");
        return;
    }

    // Chercher l'utilisateur par email uniquement
    QSqlQuery q;
    q.prepare("SELECT id, role, mot_de_passe "
              "FROM clients WHERE email = ?");
    q.addBindValue(email);

    if (!q.exec() || !q.next()) {
        QMessageBox::warning(this, "Erreur",
            "Email ou mot de passe incorrect.");
        passwordEdit->clear();
        return;
    }

    int     userId = q.value(0).toInt();
    QString role   = q.value(1).toString();
    QString dbHash = q.value(2).toString();

    // Comparer : mot de passe en clair OU hashé
    QString hashedInput = hashPassword(pwd);
    bool match = (dbHash == hashedInput) ||
                 (dbHash == pwd); // mot de passe en clair

    if (!match) {
        QMessageBox::warning(this, "Erreur",
            "Email ou mot de passe incorrect.");
        passwordEdit->clear();
        passwordEdit->setFocus();
        return;
    }

    // Succès
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