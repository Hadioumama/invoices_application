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
#include <QSvgRenderer>
#include <QPainterPath>
#include <QPainter>
#include <QIcon>
#include <QPixmap>
#include <QEvent>

class RoundedImageLabel : public QLabel
{
public:
    explicit RoundedImageLabel(QWidget *parent = nullptr) : QLabel(parent) {}
    void setSourcePixmap(const QPixmap &pix) { m_pix = pix; update(); }

protected:
    void paintEvent(QPaintEvent *) override
    {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);

        QPainterPath path;
        path.addRoundedRect(rect(), 24, 24);
        painter.setClipPath(path);

        if (!m_pix.isNull()) {
            QPixmap scaled = m_pix.scaled(size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
            int x = (width() - scaled.width()) / 2;
            int y = (height() - scaled.height()) / 2;
            painter.drawPixmap(x, y, scaled);
        } else {
            painter.fillRect(rect(), QColor("#021024"));
        }
    }

private:
    QPixmap m_pix;
};
LoginDialog::LoginDialog(QWidget *parent) : QDialog(parent),
    m_countdownTimer(nullptr), m_remainingSeconds(0), m_attemptCount(0)
{
    setupUI();
    applyStyles();
    m_sideImage = QPixmap(":/images/login_side.jpg");
     m_imagePanel->setSourcePixmap(m_sideImage);
}

void LoginDialog::paintEvent(QPaintEvent *event)
{
    QDialog::paintEvent(event);
}

static QIcon makeEyeIcon(bool slashed)
{
    QString color = "#9CA3AF";

    QString svgOpen = QString(R"(
        <svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24" fill="none" stroke="%1" stroke-width="2" stroke-linecap="round" stroke-linejoin="round">
            <path d="M1 12s4-8 11-8 11 8 11 8-4 8-11 8-11-8-11-8z"/>
            <circle cx="12" cy="12" r="3"/>
        </svg>
    )").arg(color);

    QString svgSlashed = QString(R"(
        <svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24" fill="none" stroke="%1" stroke-width="2" stroke-linecap="round" stroke-linejoin="round">
            <path d="M1 12s4-8 11-8 11 8 11 8-4 8-11 8-11-8-11-8z"/>
            <circle cx="12" cy="12" r="3"/>
            <line x1="3" y1="3" x2="21" y2="21"/>
        </svg>
    )").arg(color);

    QString svg = slashed ? svgSlashed : svgOpen;
    QSvgRenderer renderer(svg.toUtf8());
    QPixmap pixmap(22, 22);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    renderer.render(&painter);
    return QIcon(pixmap);
}

void LoginDialog::setupUI()
{
    setWindowTitle("Connexion");
    
  
    
    setWindowFlags(Qt::Widget);
     setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    // ===== LAYOUT PRINCIPAL AVEC CENTRAGE =====
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setAlignment(Qt::AlignCenter);  // ← CENTRE tout
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // ===== SPACER HAUT (centre verticalement) =====
    mainLayout->addStretch(1);

    // ===== CONTENEUR HORIZONTAL POUR CENTRAGE =====
    QHBoxLayout *centerLayout = new QHBoxLayout;
    centerLayout->setContentsMargins(0, 0, 0, 0);
    centerLayout->setSpacing(0);
    centerLayout->setAlignment(Qt::AlignCenter);
    centerLayout->addStretch(1);  // ← Élastique gauche

       // ===== CARTE SPLIT =====
    QFrame *card = new QFrame;
    card->setFixedSize(720, 460);
    // ← CORRIGÉ : border-radius uniforme de 24px comme l'image
    card->setStyleSheet(
        "QFrame {"
        "   background: white;"
        "   border-radius: 24px;"  // ← Augmenté de 20px à 24px
        "   border: none;"
        "}");

    QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(card);
    shadow->setBlurRadius(40);        // ← Légèrement augmenté pour un effet plus doux
    shadow->setColor(QColor(0, 0, 0, 50));  // ← Un peu plus transparent
    shadow->setOffset(0, 12);         // ← Ombre plus basse pour profondeur
    card->setGraphicsEffect(shadow);

    QHBoxLayout *cardLayout = new QHBoxLayout(card);
    cardLayout->setSpacing(0);
    // ← CORRIGÉ : margins à 0 pour que l'image touche les bords arrondis
    cardLayout->setContentsMargins(0, 0, 0, 0);

    // ===== PANNEAU GAUCHE — IMAGE =====
   // ✅ APRÈS
m_imagePanel = new RoundedImageLabel;
m_imagePanel->setFixedWidth(320);
m_imagePanel->setFixedHeight(460);   // même hauteur que la carte
cardLayout->addWidget(m_imagePanel);

    // ===== PANNEAU DROIT — FORMULAIRE =====
    QWidget *formPanel = new QWidget;
    formPanel->setStyleSheet("background: transparent;");

    QHBoxLayout *formPanelLayout = new QHBoxLayout(formPanel);
    formPanelLayout->setContentsMargins(0, 0, 0, 0);
    formPanelLayout->setSpacing(0);
    formPanelLayout->addStretch(1);  // ← Élastique gauche

    QWidget *formContainer = new QWidget;
    formContainer->setFixedWidth(340);
// ✅ APRÈS
QVBoxLayout *formLayout = new QVBoxLayout(formContainer);
formLayout->setContentsMargins(0, 20, 0, 40);   // ← 50 → 20 (décale tout le contenu vers le haut)
formLayout->setSpacing(0);
formLayout->setAlignment(Qt::AlignHCenter | Qt::AlignTop);
// ✅ APRÈS — taille réduite avec rendu net (DPI x2)
const int logoSize = 64;          // ← taille affichée, réduite
const int renderSize = 128;       // ← rendu interne 2x pour la netteté (HiDPI)

QLabel *logoImageLabel = new QLabel;
logoImageLabel->setAlignment(Qt::AlignCenter);
logoImageLabel->setFixedSize(logoSize, logoSize);

QPixmap logoPix(":/images/logo.png");
if (!logoPix.isNull()) {
    QPixmap scaled = logoPix.scaled(renderSize, renderSize,
        Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);

    QPixmap rounded(renderSize, renderSize);
    rounded.fill(Qt::transparent);
    QPainter painter(&rounded);
    painter.setRenderHint(QPainter::Antialiasing);
    QPainterPath path;
    path.addRoundedRect(0, 0, renderSize, renderSize, 14, 14);
    painter.setClipPath(path);
    int x = (scaled.width() - renderSize) / 2;
    int y = (scaled.height() - renderSize) / 2;
    painter.drawPixmap(-x, -y, scaled);

    rounded.setDevicePixelRatio(2.0);   // ← clé pour la netteté : Qt sait que c'est 2x
    logoImageLabel->setPixmap(rounded);
    logoImageLabel->setStyleSheet("background: transparent;");
} else {
    logoImageLabel->setStyleSheet(
        "font-size: 18px; font-weight: bold; color: #021024; background: transparent;");
}
// ✅ APRÈS — gardez seulement :
formLayout->addWidget(logoImageLabel, 0, Qt::AlignHCenter);
formLayout->addSpacing(20);   // espace direct vers le champ email
    // ===== EMAIL =====
    emailEdit = new QLineEdit;
    emailEdit->setPlaceholderText("Email address");
    emailEdit->setFixedHeight(42);
    emailEdit->setFixedWidth(320);
    formLayout->addWidget(emailEdit, 0, Qt::AlignHCenter);
    formLayout->addSpacing(14);

    // ===== MOT DE PASSE =====
    QFrame *pwdContainer = new QFrame;
    pwdContainer->setFixedHeight(42);
    pwdContainer->setFixedWidth(320);
    pwdContainer->setObjectName("pwdContainer");

    QHBoxLayout *pwdLayout = new QHBoxLayout(pwdContainer);
    pwdLayout->setSpacing(0);
    pwdLayout->setContentsMargins(14, 0, 6, 0);

    passwordEdit = new QLineEdit;
    passwordEdit->setEchoMode(QLineEdit::Password);
    passwordEdit->setPlaceholderText("Password");
    passwordEdit->setFixedHeight(40);
    passwordEdit->setFrame(false);

    togglePwdButton = new QPushButton;
    togglePwdButton->setIcon(makeEyeIcon(true));
    togglePwdButton->setIconSize(QSize(18, 18));
    togglePwdButton->setFixedSize(28, 28);
    togglePwdButton->setCursor(Qt::PointingHandCursor);
    togglePwdButton->setCheckable(true);

    pwdLayout->addWidget(passwordEdit);
    pwdLayout->addWidget(togglePwdButton);
    formLayout->addWidget(pwdContainer, 0, Qt::AlignHCenter);
    formLayout->addSpacing(8);

    // ===== MOT DE PASSE OUBLIÉ =====
    QHBoxLayout *forgotLayout = new QHBoxLayout;
    forgotLayout->setContentsMargins(0, 0, 0, 0);
    forgotLayout->addStretch();
    forgotButton = new QPushButton("Mot de passe oublié ?");
    forgotButton->setCursor(Qt::PointingHandCursor);
    forgotButton->setFlat(true);
    forgotLayout->addWidget(forgotButton);
    forgotLayout->addStretch();
    formLayout->addLayout(forgotLayout);
    formLayout->addSpacing(20);

    // ===== BOUTON CONNEXION =====
    loginButton = new QPushButton("Se connecter  →");
    loginButton->setFixedHeight(46);
    loginButton->setCursor(Qt::PointingHandCursor);
    loginButton->setFixedWidth(320);
    formLayout->addWidget(loginButton, 0, Qt::AlignHCenter);
    formLayout->addSpacing(16);

    // ===== SÉPARATEUR =====
    QHBoxLayout *sepLayout = new QHBoxLayout;
    sepLayout->setContentsMargins(0, 0, 0, 0);
    sepLayout->setSpacing(10);
    QWidget *lineLeft = new QWidget;
    lineLeft->setFixedHeight(1);
    lineLeft->setStyleSheet("background: #E2E8F0;");
    QLabel *ouLabel = new QLabel("ou");
    ouLabel->setStyleSheet("color: #A0AEC0; font-size: 12px; background: transparent;");
    QWidget *lineRight = new QWidget;
    lineRight->setFixedHeight(1);
    lineRight->setStyleSheet("background: #E2E8F0;");
    sepLayout->addWidget(lineLeft, 1);
    sepLayout->addWidget(ouLabel);
    sepLayout->addWidget(lineRight, 1);
    formLayout->addLayout(sepLayout);
    formLayout->addSpacing(14);

    // ===== BOUTON CRÉER COMPTE =====
    createButton = new QPushButton("Créer un compte");
    createButton->setFixedHeight(42);
    createButton->setCursor(Qt::PointingHandCursor);
    createButton->setFixedWidth(320);
    formLayout->addWidget(createButton, 0, Qt::AlignHCenter);
    formLayout->addStretch();

    formPanelLayout->addWidget(formContainer);
    formPanelLayout->addStretch(1);  // ← Élastique droite

    cardLayout->addWidget(formPanel, 1);

    // ===== AJOUTER LA CARTE AU CENTRE =====
    centerLayout->addWidget(card, 0, Qt::AlignCenter);
    centerLayout->addStretch(1);  // ← Élastique droite

    mainLayout->addLayout(centerLayout);
    
    // ===== SPACER BAS (centre verticalement) =====
    mainLayout->addStretch(1);

    // ===== CONNEXIONS =====
    connect(loginButton, &QPushButton::clicked, this, &LoginDialog::onLogin);
    connect(createButton, &QPushButton::clicked, this, &LoginDialog::onCreateAccountClicked);
    connect(forgotButton, &QPushButton::clicked, this, &LoginDialog::onForgotPassword);
    connect(togglePwdButton, &QPushButton::toggled, this, &LoginDialog::togglePasswordVisibility);
}
bool LoginDialog::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == passwordEdit) {
        QFrame *container = qobject_cast<QFrame*>(passwordEdit->parentWidget());
        if (container) {
            if (event->type() == QEvent::FocusIn) {
                container->setStyleSheet(R"(
                    QFrame#pwdContainer {
                        border: 1.5px solid #2B6CB0;
                        border-radius: 8px;
                        background: white;
                    }
                )");
            } else if (event->type() == QEvent::FocusOut) {
                container->setStyleSheet(R"(
                    QFrame#pwdContainer {
                        border: 1.5px solid #E2E8F0;
                        border-radius: 8px;
                        background: #F7FAFC;
                        
                    }
                    QFrame#pwdContainer:hover {
                        border: 1.5px solid #CBD5E0;
                    }
                )");
            }
        }
    }
    return QDialog::eventFilter(watched, event);
}
void LoginDialog::applyStyles()
{
    setStyleSheet("background: #094684;");

    QString inputStyle = R"(
        QLineEdit {
            border: 1.5px solid #E2E8F0;
        border-radius: 8px;
        background: #F7FAFC;
            padding: 0 12px;
            font-size: 13px;
           
            color: #2D3748;
            selection-background-color: #E2E8F0;
        }
        
        QLineEdit:hover {
            border: 1.5px solid #CBD5E0;
        }
    )";
    emailEdit->setStyleSheet(inputStyle);

    passwordEdit->setStyleSheet(R"(
        QLineEdit {
            border: none;
            background: transparent;
            font-size: 13px;
            color: #2D3748;
            selection-background-color: #2B6CB0;
        }
    )");

   // ✅ APRÈS
passwordEdit->parentWidget()->setStyleSheet(R"(
    QFrame#pwdContainer {
        border: 1.5px solid #E2E8F0;
        border-radius: 8px;
        background: #F7FAFC;
    }
    QFrame#pwdContainer:hover {
        border: 1.5px solid #CBD5E0;
    }
    QFrame#pwdContainer[focused="true"] {
        border: 1.5px solid #2B6CB0;
        background: white;
    }
)");

    togglePwdButton->setStyleSheet(R"(
        QPushButton {
            background: transparent;
            border: none;
            outline: none;
            font-size: 13px;
            color: #7DA0CA;
        }
        QPushButton:checked {
            color: #2B6CB0;
        }
        QPushButton:focus {
            border: none;
            outline: none;
        }
        QPushButton:pressed {
            background: transparent;
            border: none;
        }
    )");

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

    loginButton->setStyleSheet(R"(
        QPushButton {
            background: #3a5387;
            color: white;
            font-weight: bold;
            font-size: 14px;
            border: none;
            border-radius: 8px;
        }
        QPushButton:hover {
            background: #2D3748;
        }
        QPushButton:pressed {
            background: #0D1117;
        }
    )");

    createButton->setStyleSheet(R"(
        QPushButton {
            background: white;
            color: #566b90;
            font-weight: 600;
            font-size: 13px;
            border: 1.5px solid #c5cfdb;
            border-radius: 8px;
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
    togglePwdButton->setIcon(makeEyeIcon(!checked));
}

void LoginDialog::clearFields()
{
    emailEdit->clear();
    passwordEdit->clear();
    togglePwdButton->setChecked(false);
    togglePasswordVisibility(false);
}

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

    if (email.isEmpty() || pwd.isEmpty()) {
        QMessageBox::warning(this, "Erreur", "Veuillez remplir tous les champs.");
        return;
    }

    QSqlQuery q;
    q.prepare("SELECT id, role, mot_de_passe FROM clients WHERE LOWER(email) = LOWER(?)");
    q.addBindValue(email);

    if (!q.exec()) {
        QMessageBox::warning(this, "Erreur", "Problème de base de données.");
        return;
    }

    if (!q.next()) {
        QMessageBox::warning(this, "Erreur", "Email ou mot de passe incorrect.");
        passwordEdit->clear();
        return;
    }

    int userId = q.value(0).toInt();
    QString role = q.value(1).toString();
    QString dbHash = q.value(2).toString();

    QString hashedInput = hashPassword(pwd);
    bool match = (dbHash == hashedInput) || (dbHash == pwd);

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