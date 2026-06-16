#include "entreprise_config_widget.h"
#include "utils/entreprise_config_manager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QFileDialog>
#include <QColorDialog>
#include <QMessageBox>
#include <QPixmap>

EntrepriseConfigWidget::EntrepriseConfigWidget(QWidget *parent)
    : QWidget(parent), currentThemeColor(QColor("#ecf0f7"))
{
    setupUI();
    setConfig(EntrepriseConfigManager::instance()->loadConfig());
}

void EntrepriseConfigWidget::setupUI()
{
    setStyleSheet("background:#F1F5F9;");

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(28, 22, 28, 20);
    mainLayout->setSpacing(16);

    auto *form = new QFormLayout();
    form->setSpacing(12);

    QString inputStyle =
        "QLineEdit{"
        "border:1px solid #CBD5E0;"
        "border-radius:8px;"
        "padding:8px 12px;"
        "font-size:13px;"
        "background:white;"
        "min-height:34px;"
        "}"
        "QLineEdit:focus{"
        "border:2px solid #0a245c;"
        "background:white;"
        "}";

    nomEdit = new QLineEdit(this);
    ribEdit = new QLineEdit(this);
    nomEdit->setStyleSheet(inputStyle);
    ribEdit->setStyleSheet(inputStyle);
    form->addRow("Nom de la Banque :", nomEdit);
    form->addRow("RIB :", ribEdit);

    QString btnStyle =
        "QPushButton{"
        "background:white;"
        "color:#2D3748;"
        "border-radius:8px;"
        "padding:8px 16px;"
        "font-size:13px;"
        "font-weight:600;"
        "}"
        "QPushButton:hover{background:#EDF2F7;}"
        "QPushButton:focus{border:2px solid #2563EB;}";

    // Logo
    logoBtn = new QPushButton("Choisir logo", this);
    logoBtn->setStyleSheet(btnStyle);
    clearLogoBtn = new QPushButton("🗑️ Supprimer", this);
    clearLogoBtn->setStyleSheet(
        "QPushButton{background:#E53E3E;color:white;font-weight:bold;"
        "border:none;border-radius:5px;padding:4px 10px;}"
        "QPushButton:hover{background:#C53030;}");
    clearLogoBtn->setFixedHeight(30);
    logoPreview = new QLabel(this);
    logoPreview->setFixedSize(80, 80);
    logoPreview->setScaledContents(true);
    logoPreview->setStyleSheet("border:1px solid #E2E8F0;border-radius:4px;");
    auto *logoLayout = new QHBoxLayout();
    logoLayout->addWidget(logoBtn);
    logoLayout->addWidget(clearLogoBtn);
    logoLayout->addWidget(logoPreview);
    form->addRow("Logo :", logoLayout);

    // Signature
    signatureBtn = new QPushButton("Choisir signature", this);
    signatureBtn->setStyleSheet(btnStyle);
    clearSignatureBtn = new QPushButton("🗑️ Supprimer", this);
    clearSignatureBtn->setStyleSheet(
        "QPushButton{background:#E53E3E;color:white;font-weight:bold;"
        "border:none;border-radius:5px;padding:4px 10px;}"
        "QPushButton:hover{background:#C53030;}");
    clearSignatureBtn->setFixedHeight(30);
    signaturePreview = new QLabel(this);
    signaturePreview->setFixedSize(80, 80);
    signaturePreview->setScaledContents(true);
    signaturePreview->setStyleSheet("border:1px solid #E2E8F0;border-radius:4px;");
    auto *sigLayout = new QHBoxLayout();
    sigLayout->addWidget(signatureBtn);
    sigLayout->addWidget(clearSignatureBtn);
    sigLayout->addWidget(signaturePreview);
    form->addRow("Signature :", sigLayout);

    // Couleur
    colorBtn = new QPushButton("Choisir couleur", this);
    colorBtn->setStyleSheet(btnStyle);
    colorPreview = new QLabel(this);
    colorPreview->setFixedSize(40, 40);
    auto *colorLayout = new QHBoxLayout();
    colorLayout->addWidget(colorBtn);
    colorLayout->addWidget(colorPreview);
    form->addRow("Couleur thème :", colorLayout);

    mainLayout->addLayout(form);

    // Boutons
    saveBtn   = new QPushButton("Enregistrer", this);
    cancelBtn = new QPushButton("Annuler", this);
        // Style Enregistrer (BLEU)
    saveBtn->setStyleSheet(
        "QPushButton{"
        "background:white;"           // ← BACKGROUND BLEU
        "color:#4A5568;"
        "font-weight:bold;"
        "border:none;"
        "border-radius:8px;"
        "padding:10px 24px;"
        "font-size:14px;"
        "}"
        "QPushButton:hover{background:#1D4ED8;}"
        "QPushButton:pressed{background:#1E40AF;}"
    );

    // Style Annuler (BLANC/GRIS)
    cancelBtn->setStyleSheet(
        "QPushButton{"
        "background:white;"             // ← BACKGROUND BLANC
        "color:#4A5568;"
        "font-weight:600;"
        "border:2px solid #E2E8F0;"    // ← Bordure grise
        "border-radius:8px;"
        "padding:10px 24px;"
        "font-size:14px;"
        "}"
        "QPushButton:hover{background:#F7FAFC;}"
        "QPushButton:pressed{background:#EDF2F7;}"
    );
    auto *btnLayout = new QHBoxLayout();
    btnLayout->addWidget(saveBtn);
    btnLayout->addWidget(cancelBtn);
    mainLayout->addLayout(btnLayout);

    connect(logoBtn,      &QPushButton::clicked, this, &EntrepriseConfigWidget::pickLogo);
    connect(signatureBtn, &QPushButton::clicked, this, &EntrepriseConfigWidget::pickSignature);
    connect(colorBtn,     &QPushButton::clicked, this, &EntrepriseConfigWidget::pickColor);
    connect(saveBtn,      &QPushButton::clicked, this, &EntrepriseConfigWidget::saveConfig);
    connect(cancelBtn,    &QPushButton::clicked, this, &EntrepriseConfigWidget::backToDashboard);
    connect(clearLogoBtn, &QPushButton::clicked,
            this, &EntrepriseConfigWidget::clearLogo);
    connect(clearSignatureBtn, &QPushButton::clicked,
            this, &EntrepriseConfigWidget::clearSignature);
}
void EntrepriseConfigWidget::pickLogo()
{
    QString path = QFileDialog::getOpenFileName(this, "Choisir logo",
                   "", "Images (*.png *.jpg *.jpeg *.bmp)");
    if (!path.isEmpty()) {
        currentLogoPath = path;
        logoPreview->setPixmap(QPixmap(path));
    }
}
void EntrepriseConfigWidget::clearLogo()
{
    currentLogoPath.clear();
    logoPreview->clear();
    logoPreview->setStyleSheet(
        "border:1px solid #E2E8F0;border-radius:4px;"
        "background:#F7FAFC;");
}
void EntrepriseConfigWidget::pickSignature()
{
    QString path = QFileDialog::getOpenFileName(this, "Choisir signature",
                   "", "Images (*.png *.jpg *.jpeg *.bmp)");
    if (!path.isEmpty()) {
        currentSignaturePath = path;
        signaturePreview->setPixmap(QPixmap(path));
    }
}
void EntrepriseConfigWidget::clearSignature()
{
    currentSignaturePath.clear();
    signaturePreview->clear();
    signaturePreview->setStyleSheet(
        "border:1px solid #E2E8F0;border-radius:4px;"
        "background:#F7FAFC;");
}

void EntrepriseConfigWidget::pickColor()
{
    QColor color = QColorDialog::getColor(currentThemeColor, this, "Choisir couleur");
    if (color.isValid()) {
        currentThemeColor = color;
        colorPreview->setStyleSheet(
            QString("background-color: %1; border: 1px solid #ccc;").arg(color.name()));
    }
}

void EntrepriseConfigWidget::saveConfig()
{
    if (nomEdit->text().trimmed().isEmpty() || ribEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "Champs requis", "Nom et RIB sont obligatoires.");
        return;
    }
    EntrepriseConfig cfg = getConfig();
    EntrepriseConfigManager::instance()->saveConfig(cfg);
    QMessageBox::information(this, "Succès", "Configuration enregistrée.");
    emit configSaved(cfg);
}

EntrepriseConfig EntrepriseConfigWidget::getConfig() const
{
    EntrepriseConfig cfg;
    cfg.nom           = nomEdit->text().trimmed();
    cfg.rib           = ribEdit->text().trimmed();
    cfg.logoPath      = currentLogoPath;
    cfg.signaturePath = currentSignaturePath;
    cfg.themeCouleur  = currentThemeColor;
    cfg.configured    = true;
    return cfg;
}

void EntrepriseConfigWidget::setConfig(const EntrepriseConfig &cfg)
{
    nomEdit->setText(cfg.nom);
    ribEdit->setText(cfg.rib);
    currentLogoPath      = cfg.logoPath;
    currentSignaturePath = cfg.signaturePath;
    currentThemeColor    = cfg.themeCouleur;

   if (!cfg.logoPath.isEmpty()) {
    logoPreview->setPixmap(QPixmap(cfg.logoPath));
} else {
    logoPreview->clear();
    logoPreview->setStyleSheet(
        "border:1px solid #E2E8F0;border-radius:4px;"
        "background:#F7FAFC;");
}

if (!cfg.signaturePath.isEmpty()) {
    signaturePreview->setPixmap(QPixmap(cfg.signaturePath));
} else {
    signaturePreview->clear();
    signaturePreview->setStyleSheet(
        "border:1px solid #E2E8F0;border-radius:4px;"
        "background:#F7FAFC;");
}

    colorPreview->setStyleSheet(
        QString("background-color: %1; border: 1px solid #ccc;")
        .arg(cfg.themeCouleur.name()));
}

void EntrepriseConfigWidget::styleFileBtn(QPushButton *btn)
{
    btn->setStyleSheet("padding: 6px 12px;");
}