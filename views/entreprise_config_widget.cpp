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
    : QWidget(parent), currentThemeColor(QColor("#2563EB"))
{
    setupUI();
    setConfig(EntrepriseConfigManager::instance()->loadConfig());
}

void EntrepriseConfigWidget::setupUI()
{
    auto *mainLayout = new QVBoxLayout(this);

    auto *form = new QFormLayout();

    nomEdit = new QLineEdit(this);
    ribEdit = new QLineEdit(this);
    form->addRow("Nom entreprise :", nomEdit);
    form->addRow("RIB :", ribEdit);

    // Logo
    logoBtn = new QPushButton("Choisir logo", this);
    logoPreview = new QLabel(this);
    logoPreview->setFixedSize(80, 80);
    logoPreview->setScaledContents(true);
    auto *logoLayout = new QHBoxLayout();
    logoLayout->addWidget(logoBtn);
    logoLayout->addWidget(logoPreview);
    form->addRow("Logo :", logoLayout);

    // Signature
    signatureBtn = new QPushButton("Choisir signature", this);
    signaturePreview = new QLabel(this);
    signaturePreview->setFixedSize(80, 80);
    signaturePreview->setScaledContents(true);
    auto *sigLayout = new QHBoxLayout();
    sigLayout->addWidget(signatureBtn);
    sigLayout->addWidget(signaturePreview);
    form->addRow("Signature :", sigLayout);

    // Couleur
    colorBtn = new QPushButton("Choisir couleur", this);
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
    auto *btnLayout = new QHBoxLayout();
    btnLayout->addWidget(saveBtn);
    btnLayout->addWidget(cancelBtn);
    mainLayout->addLayout(btnLayout);

    connect(logoBtn,      &QPushButton::clicked, this, &EntrepriseConfigWidget::pickLogo);
    connect(signatureBtn, &QPushButton::clicked, this, &EntrepriseConfigWidget::pickSignature);
    connect(colorBtn,     &QPushButton::clicked, this, &EntrepriseConfigWidget::pickColor);
    connect(saveBtn,      &QPushButton::clicked, this, &EntrepriseConfigWidget::saveConfig);
    connect(cancelBtn,    &QPushButton::clicked, this, &EntrepriseConfigWidget::backToDashboard);
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

void EntrepriseConfigWidget::pickSignature()
{
    QString path = QFileDialog::getOpenFileName(this, "Choisir signature",
                   "", "Images (*.png *.jpg *.jpeg *.bmp)");
    if (!path.isEmpty()) {
        currentSignaturePath = path;
        signaturePreview->setPixmap(QPixmap(path));
    }
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

    if (!cfg.logoPath.isEmpty())
        logoPreview->setPixmap(QPixmap(cfg.logoPath));
    if (!cfg.signaturePath.isEmpty())
        signaturePreview->setPixmap(QPixmap(cfg.signaturePath));

    colorPreview->setStyleSheet(
        QString("background-color: %1; border: 1px solid #ccc;")
        .arg(cfg.themeCouleur.name()));
}

void EntrepriseConfigWidget::styleFileBtn(QPushButton *btn)
{
    btn->setStyleSheet("padding: 6px 12px;");
}