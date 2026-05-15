#include "invoicecreatedialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QDateEdit>
#include <QComboBox>
#include <QTableWidget>
#include <QPushButton>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QMessageBox>
#include <QSqlQuery>
#include <QSqlError>
#include <QDate>
#include <QHeaderView>
#include <QFileDialog>
#include <QPixmap>
#include <QDebug>
#include <QFrame>
#include <QScrollArea>

InvoiceCreateDialog::InvoiceCreateDialog(int invoiceId, QWidget *parent)
    : QDialog(parent), m_invoiceId(invoiceId), m_isEditMode(invoiceId > 0)
{
    setupUI();
    if (m_isEditMode)
        loadInvoiceLines();
}

// ============================================================
// HELPERS COMPACTS
// ============================================================

QFrame* InvoiceCreateDialog::createCard()
{
    QFrame *card = new QFrame;
    card->setStyleSheet(R"(
        QFrame {
            background-color: white;
          
            border-radius: 10px;
        }
    )");
    QVBoxLayout *cardLayout = new QVBoxLayout(card);
    cardLayout->setContentsMargins(16, 14, 16, 14);
    cardLayout->setSpacing(8);
    return card;
}

QLineEdit* InvoiceCreateDialog::createStyledLineEdit(const QString &placeholder)
{
    QLineEdit *edit = new QLineEdit;
    edit->setPlaceholderText(placeholder);
    edit->setStyleSheet(R"(
        QLineEdit {
            border: 1px solid #E2E8F0;
            border-radius: 10px;
            padding: 6px 10px;
            font-size: 13px;
            background: #F7FAFC;
            color: #010b1b;
        }
        QLineEdit:focus { border: 1px solid #053970; background: white; }
        QLineEdit:hover { border: 1px solid #033870; }
        QLineEdit:read-only { background: #EDF2F7; color: #010914; }
    )");
    edit->setFixedHeight(34);
    return edit;
}

QLabel* InvoiceCreateDialog::createFieldLabel(const QString &text)
{
    QLabel *label = new QLabel(text);
    label->setStyleSheet("font-size: 12px; font-weight: 600; color: #020d22;");
    return label;
}

QLabel* InvoiceCreateDialog::createSectionTitle(const QString &title, const QString &icon)
{
    QLabel *label = new QLabel(QString(" %1 %2").arg(icon, title));
    label->setStyleSheet(R"(
        color: #2B6CB0;
        font-size: 14px;
        font-weight: bold;
        margin-bottom: 4px;
        margin-left: 4px;
    )");
    return label;
}

// ============================================================
// UI SETUP — TITRES EN DEHORS DES CARTES (style Image 2)
// ============================================================
void InvoiceCreateDialog::setupUI()
{
    setWindowTitle(m_isEditMode ? "Modifier la Facture" : "Créer une Facture");
    setMinimumSize(900, 650);
    resize(960, 700);
    setStyleSheet("QDialog { background-color: #F5F7FA; }");

    // ===== LAYOUT PRINCIPAL DIRECT (pas de layout intermédiaire vide) =====
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 16, 20, 16);
    mainLayout->setSpacing(8);

    // ===== SCROLL AREA =====
    QScrollArea *scrollArea = new QScrollArea(this);  // ← parent = this
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setStyleSheet(R"(
        QScrollArea { background: #F5F7FA; border: none; }
        QScrollBar:vertical { background: transparent; width: 5px; margin: 0px; }
        QScrollBar::handle:vertical { background: #6198d3; border-radius: 2px; min-height: 10px; }
        QScrollBar::handle:vertical:hover { background: #A0AEC0; }
        QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0px; }
    )");

    // ===== CONTENU DU SCROLL =====
    QWidget *scrollContent = new QWidget;
    QVBoxLayout *contentLayout = new QVBoxLayout(scrollContent);
    contentLayout->setSpacing(6);
    contentLayout->setContentsMargins(0, 0, 0, 0);
    contentLayout->setAlignment(Qt::AlignTop);

    // ========================================================
    // 1. SECTION — INFORMATIONS FACTURE
    // ========================================================
    contentLayout->addWidget(createSectionTitle("Informations Facture", "📄"));

    QFrame *factureCard = createCard();
    QVBoxLayout *factureLayout = qobject_cast<QVBoxLayout*>(factureCard->layout());

    // Ligne 1 : Numéro + Type
    QHBoxLayout *row1 = new QHBoxLayout;
    row1->setSpacing(10);

    QVBoxLayout *numLayout = new QVBoxLayout;
    numLayout->setSpacing(2);
    numLayout->addWidget(createFieldLabel("Numéro"));
    numeroEdit = createStyledLineEdit("FAC-2026-0001");
    numLayout->addWidget(numeroEdit);
    row1->addLayout(numLayout, 1);

    QVBoxLayout *typeLayout = new QVBoxLayout;
    typeLayout->setSpacing(2);
    typeLayout->addWidget(createFieldLabel("Type"));
    typeCombo = new QComboBox;
    typeCombo->addItems({"Facture", "Devis"});
    typeCombo->setFixedHeight(34);
    typeCombo->setStyleSheet(R"(
        QComboBox {
            border: 1px solid #E2E8F0;
            border-radius: 6px;
            padding: 4px 10px;
            font-size: 13px;
            background: #F7FAFC;
            color: #020914;
        }
        QComboBox:focus { border: 1px solid #2B6CB0; }
        QComboBox::drop-down { border: none; width: 24px; }
        QComboBox::down-arrow {
            image: none;
            border-left: 4px solid transparent;
            border-right: 4px solid transparent;
            border-top: 5px solid #4068a3;
            width: 0px; height: 0px;
        }
    )");
    typeLayout->addWidget(typeCombo);
    row1->addLayout(typeLayout, 1);

    factureLayout->addLayout(row1);

    // Ligne 2 : Date création + Date échéance + Statut
    QHBoxLayout *row2 = new QHBoxLayout;
    row2->setSpacing(10);

    QVBoxLayout *dateCreaLayout = new QVBoxLayout;
    dateCreaLayout->setSpacing(2);
    dateCreaLayout->addWidget(createFieldLabel("Date création"));
    dateCreationEdit = new QDateEdit(QDate::currentDate());
    dateCreationEdit->setCalendarPopup(true);
    dateCreationEdit->setFixedHeight(34);
    dateCreationEdit->setStyleSheet(R"(
        QDateEdit {
            border: 1px solid #E2E8F0;
            border-radius: 6px;
            padding: 4px 10px;
            font-size: 13px;
            background: #F7FAFC;
        }
        QDateEdit:focus { border: 1px solid #2B6CB0; }
    )");
    dateCreaLayout->addWidget(dateCreationEdit);
    row2->addLayout(dateCreaLayout, 1);

    QVBoxLayout *dateEchLayout = new QVBoxLayout;
    dateEchLayout->setSpacing(2);
    dateEchLayout->addWidget(createFieldLabel("Date échéance"));
    dateEcheanceEdit = new QDateEdit(QDate::currentDate().addDays(30));
    dateEcheanceEdit->setCalendarPopup(true);
    dateEcheanceEdit->setFixedHeight(34);
    dateEcheanceEdit->setStyleSheet(R"(
        QDateEdit {
            border: 1px solid #E2E8F0;
            border-radius: 6px;
            padding: 4px 10px;
            font-size: 13px;
            background: #F7FAFC;
        }
        QDateEdit:focus { border: 1px solid #2B6CB0; }
    )");
    dateEchLayout->addWidget(dateEcheanceEdit);
    row2->addLayout(dateEchLayout, 1);

    QVBoxLayout *statusLayout = new QVBoxLayout;
    statusLayout->setSpacing(2);
    statusLayout->addWidget(createFieldLabel("Statut"));
    statusCombo = new QComboBox;
    statusCombo->addItems({"Brouillon", "Envoyée", "Payée", "Annulée"});
    statusCombo->setFixedHeight(34);
    statusCombo->setStyleSheet(R"(
        QComboBox {
            border: 1px solid #E2E8F0;
            border-radius: 6px;
            padding: 4px 10px;
            font-size: 13px;
            background: #F7FAFC;
            color: #2D3748;
        }
        QComboBox:focus { border: 1px solid #2B6CB0; }
        QComboBox::drop-down { border: none; width: 24px; }
        QComboBox::down-arrow {
            image: none;
            border-left: 4px solid transparent;
            border-right: 4px solid transparent;
            border-top: 5px solid  #2B6CB0;
            width: 0px; height: 0px;
        }
    )");
    statusLayout->addWidget(statusCombo);
    row2->addLayout(statusLayout, 1);

    factureLayout->addLayout(row2);
    contentLayout->addWidget(factureCard);

    // ========================================================
    // 2. SECTION — PERSONNALISATION
    // ========================================================
    contentLayout->addSpacing(4);
    contentLayout->addWidget(createSectionTitle("Personnalisation (Entreprise)", "🎨"));

    QFrame *persoCard = createCard();
    QVBoxLayout *persoLayout = qobject_cast<QVBoxLayout*>(persoCard->layout());

    // Logo + Signature sur la même ligne
    QHBoxLayout *persoRow = new QHBoxLayout;
    persoRow->setSpacing(10);

    // Logo
    QVBoxLayout *logoLayout = new QVBoxLayout;
    logoLayout->setSpacing(2);
    logoLayout->addWidget(createFieldLabel("Logo entreprise"));
    QHBoxLayout *logoInputRow = new QHBoxLayout;
    logoInputRow->setSpacing(6);
    logoPathEdit = createStyledLineEdit("Chemin du logo...");
    logoPathEdit->setReadOnly(true);
    logoInputRow->addWidget(logoPathEdit);
    logoBtn = new QPushButton("📁");
    logoBtn->setFixedSize(30, 30);
    logoBtn->setCursor(Qt::PointingHandCursor);
    logoBtn->setStyleSheet(R"(
        QPushButton {
            background: #EDF2F7;
            color: #4A5568;
            font-size: 12px;
            border: 1px solid #E2E8F0;
            border-radius: 6px;
        }
        QPushButton:hover { background: #E2E8F0; }
    )");
    logoInputRow->addWidget(logoBtn);
    logoLayout->addLayout(logoInputRow);
    persoRow->addLayout(logoLayout, 1);

    // Signature
    QVBoxLayout *signLayout = new QVBoxLayout;
    signLayout->setSpacing(2);
    signLayout->addWidget(createFieldLabel("Signature"));
    QHBoxLayout *signInputRow = new QHBoxLayout;
    signInputRow->setSpacing(6);
    signaturePathEdit = createStyledLineEdit("Chemin de la signature...");
    signaturePathEdit->setReadOnly(true);
    signInputRow->addWidget(signaturePathEdit);
    signatureBtn = new QPushButton("📁");
    signatureBtn->setFixedSize(30, 30);
    signatureBtn->setCursor(Qt::PointingHandCursor);
    signatureBtn->setStyleSheet(R"(
        QPushButton {
            background: #EDF2F7;
            color: #4A5568;
            font-size: 12px;
            border: 1px solid #E2E8F0;
            border-radius: 6px;
        }
        QPushButton:hover { background: #E2E8F0; }
    )");
    signInputRow->addWidget(signatureBtn);
    signLayout->addLayout(signInputRow);
    persoRow->addLayout(signLayout, 1);

    persoLayout->addLayout(persoRow);
    contentLayout->addWidget(persoCard);

    // ========================================================
    // 3. SECTION — INFORMATIONS CLIENT
    // ========================================================
    contentLayout->addSpacing(4);
    contentLayout->addWidget(createSectionTitle("Informations Client", "👤"));

    QFrame *clientCard = createCard();
    QVBoxLayout *clientLayout = qobject_cast<QVBoxLayout*>(clientCard->layout());

    // Combo client + Nom (même ligne)
    QHBoxLayout *clientRow1 = new QHBoxLayout;
    clientRow1->setSpacing(10);

    QVBoxLayout *comboLayout = new QVBoxLayout;
    comboLayout->setSpacing(2);
    comboLayout->addWidget(createFieldLabel("Client"));
    clientComboBox = new QComboBox;
    clientComboBox->setEditable(true);
    clientComboBox->addItem("-- Nouveau client --", -1);
    clientComboBox->setFixedHeight(34);
    clientComboBox->setStyleSheet(R"(
        QComboBox {
            border: 1px solid #E2E8F0;
            border-radius: 6px;
            padding: 4px 10px;
            font-size: 13px;
            background: #F7FAFC;
            color: #2D3748;
        }
        QComboBox:focus { border: 1px solid #2B6CB0; }
        QComboBox::drop-down { border: none; width: 24px; }
        QComboBox::down-arrow {
            image: none;
            border-left: 4px solid transparent;
            border-right: 4px solid transparent;
            border-top: 5px solid 2B6CB0;
            width: 0px; height: 0px;
        }
    )");

    QSqlQuery qc("SELECT id, nom, prenom, adresse, telephone, email "
                 "FROM clients WHERE role = 'client' ORDER BY nom");
    while (qc.next()) {
        QString display = qc.value(1).toString() + " " + qc.value(2).toString();
        int idx = clientComboBox->count();
        clientComboBox->addItem(display, qc.value(0).toInt());
        clientComboBox->setItemData(idx, qc.value(3).toString(), Qt::UserRole + 1);
        clientComboBox->setItemData(idx, qc.value(4).toString(), Qt::UserRole + 2);
        clientComboBox->setItemData(idx, qc.value(5).toString(), Qt::UserRole + 3);
    }
    comboLayout->addWidget(clientComboBox);
    clientRow1->addLayout(comboLayout, 1);

    QVBoxLayout *nomLayout = new QVBoxLayout;
    nomLayout->setSpacing(2);
    nomLayout->addWidget(createFieldLabel("Nom / Entreprise"));
    clientNomEdit = createStyledLineEdit("Nom complet...");
    nomLayout->addWidget(clientNomEdit);
    clientRow1->addLayout(nomLayout, 1);

    clientLayout->addLayout(clientRow1);

    // Adresse
    QVBoxLayout *adrLayout = new QVBoxLayout;
    adrLayout->setSpacing(2);
    adrLayout->addWidget(createFieldLabel("Adresse"));
    clientAdresseEdit = createStyledLineEdit("Adresse complète...");
    adrLayout->addWidget(clientAdresseEdit);
    clientLayout->addLayout(adrLayout);

    // Téléphone + Email
    QHBoxLayout *contactRow = new QHBoxLayout;
    contactRow->setSpacing(10);

    QVBoxLayout *telLayout = new QVBoxLayout;
    telLayout->setSpacing(2);
    telLayout->addWidget(createFieldLabel("Téléphone"));
    clientTelEdit = createStyledLineEdit("+212 6XX XXX XXX");
    telLayout->addWidget(clientTelEdit);
    contactRow->addLayout(telLayout, 1);

    QVBoxLayout *emailLayout = new QVBoxLayout;
    emailLayout->setSpacing(2);
    emailLayout->addWidget(createFieldLabel("Email"));
    clientEmailEdit = createStyledLineEdit("email@exemple.com");
    emailLayout->addWidget(clientEmailEdit);
    contactRow->addLayout(emailLayout, 1);

    clientLayout->addLayout(contactRow);
    contentLayout->addWidget(clientCard);

    // ========================================================
    // 4. SECTION — ARTICLES
    // ========================================================
    contentLayout->addSpacing(4);
    contentLayout->addWidget(createSectionTitle("Articles de la facture", "🛒"));

    QFrame *articlesCard = createCard();
    QVBoxLayout *articlesLayout = qobject_cast<QVBoxLayout*>(articlesCard->layout());

    // Table compacte
    linesTable = new QTableWidget(0, 5);
    linesTable->setHorizontalHeaderLabels({"Désignation", "Qté", "Prix HT", "TVA %", "Total HT"});
    linesTable->horizontalHeader()->setStretchLastSection(true);
    linesTable->horizontalHeader()->setDefaultAlignment(Qt::AlignCenter);
    linesTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    linesTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    linesTable->setSelectionMode(QAbstractItemView::SingleSelection);
    linesTable->setColumnWidth(0, 300);
    linesTable->setColumnWidth(1, 55);
    linesTable->setColumnWidth(2, 85);
    linesTable->setColumnWidth(3, 65);
    linesTable->setAlternatingRowColors(true);
    linesTable->setStyleSheet(R"(
        QTableWidget {
            border: 1px solid #3d7bcd;
            border-radius: 6px;
            background: white;
            gridline-color: #EDF2F7;
        }
        QHeaderView::section {
            background-color: #2B6CB0;
            color: white;
            font-weight: bold;
            font-size: 12px;
            padding: 5px;
            border: none;
        }
        QTableWidget::item {
            padding: 4px;
            border-bottom: 1px solid #EDF2F7;
            font-size: 12px;
        }
        QTableWidget::item:selected {
            background-color: #EBF8FF;
            color: #2B6CB0;
        }
        QTableWidget::item:alternate {
            background-color: #F7FAFC;
        }
    )");
    linesTable->setMinimumHeight(80);
    linesTable->setMaximumHeight(120);
    articlesLayout->addWidget(linesTable);

    // ===== NOUVELLE LIGNE =====
    QFrame *newLineCard = new QFrame;
    newLineCard->setStyleSheet(R"(
        QFrame {
            background-color: #EBF8FF;
            border: 1px solid #BEE3F8;
            border-radius: 8px;
        }
    )");

    QHBoxLayout *newLineLayout = new QHBoxLayout(newLineCard);
    newLineLayout->setContentsMargins(10, 6, 10, 6);
    newLineLayout->setSpacing(8);

    // Désignation
    QVBoxLayout *desLayout = new QVBoxLayout;
    desLayout->setSpacing(1);
    QLabel *desLabel = new QLabel("Désignation:");
    desLabel->setStyleSheet("font-size: 11px; font-weight: 600; color: #030a17;");
    desLayout->addWidget(desLabel);
    designationEdit = new QComboBox;
    designationEdit->setEditable(true);
    designationEdit->addItem("-- Saisie manuelle --", -1);
    designationEdit->setMinimumWidth(180);
    designationEdit->setFixedHeight(28);
    designationEdit->setStyleSheet(R"(
        QComboBox {
            border: 1px solid #90CDF4;
            border-radius: 6px;
            padding: 2px 8px;
            font-size: 12px;
            background: white;
            color: #2D3748;
        }
        QComboBox:focus { border: 1px solid #2B6CB0; }
        QComboBox::drop-down { border: none; width: 20px; }
        QComboBox::down-arrow {
            image: none;
            border-left: 3px solid transparent;
            border-right: 3px solid transparent;
            border-top: 5px solid #2B6CB0;
            width: 0px; height: 0px;
        }
    )");
    desLayout->addWidget(designationEdit);
    newLineLayout->addLayout(desLayout, 2);

    // Qté
    QVBoxLayout *qteLayout = new QVBoxLayout;
    qteLayout->setSpacing(1);
    QLabel *qteLabel = new QLabel("Qté:");
    qteLabel->setStyleSheet("font-size: 11px; font-weight: 600; color: #010a1a;");
    qteLayout->addWidget(qteLabel);
    quantitySpinBox = new QSpinBox;
    quantitySpinBox->setMinimum(1);
    quantitySpinBox->setMaximum(9999);
    quantitySpinBox->setValue(1);
    quantitySpinBox->setFixedWidth(50);
    quantitySpinBox->setFixedHeight(28);
    quantitySpinBox->setStyleSheet(R"(
        QSpinBox {
            border: 1px solid #90CDF4;
            border-radius: 6px;
            padding: 2px;
            font-size: 12px;
            background: white;
        }
        QSpinBox:focus { border: 1px solid #2B6CB0; }
    )");
    qteLayout->addWidget(quantitySpinBox);
    newLineLayout->addLayout(qteLayout);

    // Prix HT
    QVBoxLayout *prixLayout = new QVBoxLayout;
    prixLayout->setSpacing(1);
    QLabel *prixLabel = new QLabel("Prix HT:");
    prixLabel->setStyleSheet("font-size: 11px; font-weight: 600; color: #020c1e;");
    prixLayout->addWidget(prixLabel);
    priceHTSpinBox = new QDoubleSpinBox;
    priceHTSpinBox->setMaximum(999999);
    priceHTSpinBox->setDecimals(2);
    priceHTSpinBox->setFixedWidth(85);
    priceHTSpinBox->setFixedHeight(28);
    priceHTSpinBox->setSuffix(" MAD");
    priceHTSpinBox->setStyleSheet(R"(
        QDoubleSpinBox {
            border: 1px solid #90CDF4;
            border-radius: 6px;
            padding: 2px;
            font-size: 12px;
            background: white;
        }
        QDoubleSpinBox:focus { border: 1px solid #2B6CB0; }
    )");
    prixLayout->addWidget(priceHTSpinBox);
    newLineLayout->addLayout(prixLayout);

    // TVA
    QVBoxLayout *tvaLayout = new QVBoxLayout;
    tvaLayout->setSpacing(1);
    QLabel *tvaLabel = new QLabel("TVA %:");
    tvaLabel->setStyleSheet("font-size: 11px; font-weight: 600; color: #010c1f;");
    tvaLayout->addWidget(tvaLabel);
    taxRateSpinBox = new QDoubleSpinBox;
    taxRateSpinBox->setValue(20.0);
    taxRateSpinBox->setMaximum(100);
    taxRateSpinBox->setDecimals(2);
    taxRateSpinBox->setFixedWidth(65);
    taxRateSpinBox->setFixedHeight(28);
    taxRateSpinBox->setSuffix(" %");
    taxRateSpinBox->setStyleSheet(R"(
        QDoubleSpinBox {
            border: 1px solid #90CDF4;
            border-radius: 6px;
            padding: 2px;
            font-size: 12px;
            background: white;
        }
        QDoubleSpinBox:focus { border: 1px solid #2B6CB0; }
    )");
    tvaLayout->addWidget(taxRateSpinBox);
    newLineLayout->addLayout(tvaLayout);

       // Boutons — VERSION AVEC TEXTE
    addLineBtn = new QPushButton("➕ Ajouter");
    addLineBtn->setFixedHeight(32);
    addLineBtn->setFixedWidth(100);
    addLineBtn->setCursor(Qt::PointingHandCursor);
    addLineBtn->setStyleSheet(R"(
        QPushButton {
            background-color: #2B6CB0;
            color: white;
            font-size: 12px;
            font-weight: bold;
            border: none;
            border-radius: 6px;
            padding: 4px 12px;
        }
        QPushButton:hover { background-color: #1A365D; }
    )");

    removeLineBtn = new QPushButton("🗑️ Supprimer");
    removeLineBtn->setFixedHeight(32);
    removeLineBtn->setFixedWidth(100);
    removeLineBtn->setCursor(Qt::PointingHandCursor);
    removeLineBtn->setStyleSheet(R"(
        QPushButton {
            background-color: #E53E3E;
            color: white;
            font-size: 12px;
            font-weight: bold;
            border: none;
            border-radius: 6px;
            padding: 4px 12px;
        }
        QPushButton:hover { background-color: #C53030; }
    )");

    newLineLayout->addWidget(addLineBtn);
    newLineLayout->addWidget(removeLineBtn);

    articlesLayout->addWidget(newLineCard);

    // Totaux
    QHBoxLayout *totalsLayout = new QHBoxLayout;
    totalsLayout->addStretch();

    totalHTLabel = new QLabel("Total HT: 0.00 MAD");
    totalHTLabel->setStyleSheet("font-size: 13px; color: #010814;");

    totalTVALabel = new QLabel("TVA: 0.00 MAD");
    totalTVALabel->setStyleSheet("font-size: 13px; color: #010a19;");

    totalTTCLabel = new QLabel("Total TTC: 0.00 MAD");
    totalTTCLabel->setStyleSheet("font-size: 14px; font-weight: bold; color: #010911;");

    totalsLayout->addWidget(totalHTLabel);
    totalsLayout->addSpacing(14);
    totalsLayout->addWidget(totalTVALabel);
    totalsLayout->addSpacing(14);
    totalsLayout->addWidget(totalTTCLabel);

    articlesLayout->addLayout(totalsLayout);
    contentLayout->addWidget(articlesCard);

       // ========================================================
    // 5. FOOTER — BOUTONS (FIXE EN BAS, HORS SCROLL)
    // ========================================================
    
    // D'abord, finir le contenu scrollable
    scrollArea->setWidget(scrollContent);
    mainLayout->addWidget(scrollArea, 1);  // stretch = 1, prend tout l'espace

    // Footer fixe en bas
    QFrame *footerFrame = new QFrame;
    footerFrame->setStyleSheet("background-color: white; border-top: 1px solid #E2E8F0;");
    footerFrame->setFixedHeight(55);

    QHBoxLayout *footerLayout = new QHBoxLayout(footerFrame);
    footerLayout->setContentsMargins(20, 8, 20, 8);
    footerLayout->setSpacing(12);
    footerLayout->addStretch();

    cancelBtn = new QPushButton("❌ Annuler");
    cancelBtn->setFixedHeight(38);
    cancelBtn->setFixedWidth(120);
    cancelBtn->setCursor(Qt::PointingHandCursor);
    cancelBtn->setStyleSheet(R"(
        QPushButton {
            background: #F7FAFC;
            color: #011027;
            font-weight: 600;
            font-size: 13px;
            border: 1px solid #E2E8F0;
            border-radius: 8px;
        }
        QPushButton:hover { background: #EDF2F7; color: #4A5568; border-color: #CBD5E0; }
    )");

    saveBtn = new QPushButton("💾 Enregistrer");
    saveBtn->setFixedHeight(38);
    saveBtn->setFixedWidth(140);
    saveBtn->setCursor(Qt::PointingHandCursor);
    saveBtn->setStyleSheet(R"(
        QPushButton {
            background-color: #38A169;
            color: white;
            font-weight: bold;
            font-size: 13px;
            border: none;
            border-radius: 8px;
        }
        QPushButton:hover { background-color: #276749; }
    )");

    footerLayout->addWidget(cancelBtn);
    footerLayout->addWidget(saveBtn);

    mainLayout->addWidget(footerFrame);  // Ajouté APRÈS le scrollArea

    contentLayout->addLayout(footerLayout);

    // ===== ATTACHER LE CONTENU AU SCROLL =====
    scrollArea->setWidget(scrollContent);

    // ===== AJOUTER LE SCROLL AU LAYOUT PRINCIPAL =====
    mainLayout->addWidget(scrollArea);
    
    mainLayout->addWidget(footerFrame);  
    // ========================================================
    // CONNEXIONS
    // ========================================================
    connect(clientComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &InvoiceCreateDialog::onClientSelected);

    connect(designationEdit, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &InvoiceCreateDialog::onArticleSelected);

    connect(addLineBtn, &QPushButton::clicked, this, &InvoiceCreateDialog::onAddLine);
    connect(removeLineBtn, &QPushButton::clicked, this, &InvoiceCreateDialog::onRemoveLine);
    connect(saveBtn, &QPushButton::clicked, this, &InvoiceCreateDialog::onSave);
    connect(cancelBtn, &QPushButton::clicked, this, &InvoiceCreateDialog::onCancel);

    connect(logoBtn, &QPushButton::clicked, this, [this]() {
        QString path = QFileDialog::getOpenFileName(this, "Choisir le logo",
                       "", "Images (*.png *.jpg *.jpeg)");
        if (!path.isEmpty()) {
            logoPathEdit->setText(path);
            logoPreview->setPixmap(QPixmap(path));
        }
    });

    connect(signatureBtn, &QPushButton::clicked, this, [this]() {
        QString path = QFileDialog::getOpenFileName(this, "Choisir la signature",
                       "", "Images (*.png *.jpg *.jpeg)");
        if (!path.isEmpty()) {
            signaturePathEdit->setText(path);
            signaturePreview->setPixmap(QPixmap(path));
        }
    });

    loadArticles();
}

// ============================================================
// MÉTHODES MÉTIER (INCHANGÉES)
// ============================================================

void InvoiceCreateDialog::loadClients() {}
void InvoiceCreateDialog::loadArticles()
{
    designationEdit->clear();
    designationEdit->addItem("-- Saisie manuelle --", -1);

    QSqlQuery q("SELECT id, reference, designation, prix_ht, taux_tva "
                "FROM articles ORDER BY designation");
    while (q.next()) {
        QString display = q.value(2).toString() + " (" + q.value(1).toString() + ")";
        designationEdit->addItem(display, q.value(0).toInt());
        designationEdit->setItemData(designationEdit->count() - 1,
                                     q.value(3).toDouble(), Qt::UserRole + 1);
        designationEdit->setItemData(designationEdit->count() - 1,
                                     q.value(4).toDouble(), Qt::UserRole + 2);
    }
}
void InvoiceCreateDialog::loadInvoiceLines() {}
void InvoiceCreateDialog::updateLineData() {}

void InvoiceCreateDialog::refreshLineTable()
{
    linesTable->setRowCount(0);
    double totalHT = 0, totalTVA = 0;

    for (const InvoiceLineItem &item : m_lineItems) {
        int row = linesTable->rowCount();
        linesTable->insertRow(row);
        double lineHT = item.quantity * item.priceHT;

        linesTable->setItem(row, 0, new QTableWidgetItem(item.designation));
        linesTable->setItem(row, 1, new QTableWidgetItem(QString::number(item.quantity)));
        linesTable->setItem(row, 2, new QTableWidgetItem(QString::number(item.priceHT, 'f', 2)));
        linesTable->setItem(row, 3, new QTableWidgetItem(QString::number(item.taxRate, 'f', 1) + "%"));
        linesTable->setItem(row, 4, new QTableWidgetItem(QString::number(lineHT, 'f', 2)));

        totalHT += lineHT;
        totalTVA += lineHT * item.taxRate / 100.0;
    }

    totalHTLabel->setText(QString("Total HT: %1 MAD").arg(totalHT, 0, 'f', 2));
    totalTVALabel->setText(QString("TVA: %1 MAD").arg(totalTVA, 0, 'f', 2));
    totalTTCLabel->setText(QString("Total TTC: %1 MAD").arg(totalHT + totalTVA, 0, 'f', 2));
}

void InvoiceCreateDialog::onSave()
{
    if (clientNomEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "Erreur", "Entrez le nom du client");
        return;
    }
    if (numeroEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "Erreur", "Entrez le numéro de facture");
        return;
    }
    if (m_lineItems.isEmpty()) {
        QMessageBox::warning(this, "Erreur", "Ajoutez au moins un article");
        return;
    }

    double totalHT = 0, totalTVA = 0;
    for (const InvoiceLineItem &item : m_lineItems) {
        double ht = item.quantity * item.priceHT;
        totalHT += ht;
        totalTVA += ht * item.taxRate / 100.0;
    }
    double totalTTC = totalHT + totalTVA;

    m_logoPath = logoPathEdit->text();
    m_signaturePath = signaturePathEdit->text();

    QString clientNom, clientAdresse, clientTel, clientEmail;
    int clientId = clientComboBox->currentData().toInt();

    if (clientId > 0) {
        QSqlQuery cq;
        cq.prepare("SELECT id, nom, prenom, adresse, telephone, email FROM clients WHERE id = ?");
        cq.addBindValue(clientId);
        if (cq.exec() && cq.next()) {
            clientId = cq.value(0).toInt();
            clientNom = cq.value(1).toString() + " " + cq.value(2).toString();
            clientAdresse = cq.value(3).toString();
            clientTel = cq.value(4).toString();
            clientEmail = cq.value(5).toString();
        }
    } else {
        clientId = 1;
        clientNom = clientNomEdit->text().trimmed();
        clientAdresse = clientAdresseEdit->text().trimmed();
        clientTel = clientTelEdit->text().trimmed();
        clientEmail = clientEmailEdit->text().trimmed();
    }

    QSqlQuery q;
    q.prepare("INSERT INTO factures ("
        "numero, type, client_id, client_nom, client_adresse, client_tel, client_email, "
        "date_creation, date_echeance, date_validite, statut, total_ht, total_tva, total_ttc, "
        "facture_source_id, logo_path, signature_path"
        ") VALUES ("
        "?, ?, ?, ?, ?, ?, ?, "
        "?, ?, ?, ?, ?, ?, ?, "
        "?, ?, ?"
        ")");

    q.addBindValue(numeroEdit->text().trimmed());
    q.addBindValue(typeCombo->currentText());
    q.addBindValue(clientId);
    q.addBindValue(clientNom);
    q.addBindValue(clientAdresse);
    q.addBindValue(clientTel);
    q.addBindValue(clientEmail);
    q.addBindValue(dateCreationEdit->date().toString("yyyy-MM-dd"));
    q.addBindValue(dateEcheanceEdit->date().toString("yyyy-MM-dd"));
    q.addBindValue(dateEcheanceEdit->date().toString("yyyy-MM-dd"));
    q.addBindValue(statusCombo->currentText());
    q.addBindValue(totalHT);
    q.addBindValue(totalTVA);
    q.addBindValue(totalTTC);
    q.addBindValue(QVariant());
    q.addBindValue(logoPathEdit->text());
    q.addBindValue(signaturePathEdit->text());

    qDebug() << "=== DEBUG INSERT ===";
    qDebug() << "Nombre de ?:" << q.lastQuery().count('?');
    qDebug() << "Requête:" << q.lastQuery();

    if (!q.exec()) {
        qDebug() << "ERREUR SQL:" << q.lastError().text();
        QMessageBox::critical(this, "Erreur SQL",
            "Erreur: " + q.lastError().text() +
            "\n\nRequête: " + q.lastQuery());
        return;
    }

    m_invoiceId = q.lastInsertId().toInt();

    for (const InvoiceLineItem &item : m_lineItems) {
        QSqlQuery lq;
        lq.prepare("INSERT INTO lignes_facture "
                   "(facture_id, article_id, designation, quantite, prix_unitaire_ht, taux_tva) "
                   "VALUES (?, ?, ?, ?, ?, ?)");
        lq.addBindValue(m_invoiceId);
        lq.addBindValue(item.articleId > 0 ? item.articleId : QVariant());
        lq.addBindValue(item.designation);
        lq.addBindValue(item.quantity);
        lq.addBindValue(item.priceHT);
        lq.addBindValue(item.taxRate);
        lq.exec();
    }

    QMessageBox::information(this, "Succès", "Facture enregistrée avec succès !");
    accept();
}

void InvoiceCreateDialog::calculateTotals() { refreshLineTable(); }

void InvoiceCreateDialog::onAddLine()
{
    QString designation;
    int articleId = designationEdit->currentData().toInt();

    if (articleId > 0) {
        QString fullText = designationEdit->currentText();
        int idx = fullText.lastIndexOf(" (");
        if (idx > 0) {
            designation = fullText.left(idx).trimmed();
        } else {
            designation = fullText.trimmed();
        }
    } else {
        designation = designationEdit->currentText().trimmed();
    }

    if (designation.isEmpty() || designation == "-- Saisie manuelle --") {
        QMessageBox::warning(this, "Erreur", "Entrez une désignation");
        return;
    }
    if (priceHTSpinBox->value() <= 0) {
        QMessageBox::warning(this, "Erreur", "Entrez un prix valide");
        return;
    }

    InvoiceLineItem item;
    item.articleId = articleId > 0 ? articleId : 0;
    item.designation = designation;
    item.quantity = quantitySpinBox->value();
    item.priceHT = priceHTSpinBox->value();
    item.taxRate = taxRateSpinBox->value();
    m_lineItems.append(item);

    designationEdit->setCurrentIndex(0);
    quantitySpinBox->setValue(1);
    priceHTSpinBox->setValue(0);
    taxRateSpinBox->setValue(20);

    refreshLineTable();
}

void InvoiceCreateDialog::onRemoveLine()
{
    int row = linesTable->currentRow();
    if (row >= 0 && row < m_lineItems.size()) {
        m_lineItems.removeAt(row);
        refreshLineTable();
    }
}

void InvoiceCreateDialog::onEditLine() {}
void InvoiceCreateDialog::onArticleSelected(int index)
{
    Q_UNUSED(index)
    int articleId = designationEdit->currentData().toInt();

    if (articleId <= 0) {
        priceHTSpinBox->setValue(0);
        taxRateSpinBox->setValue(20);
        return;
    }

    QSqlQuery q;
    q.prepare("SELECT prix_ht, taux_tva FROM articles WHERE id = ?");
    q.addBindValue(articleId);
    if (q.exec() && q.next()) {
        priceHTSpinBox->setValue(q.value(0).toDouble());
        taxRateSpinBox->setValue(q.value(1).toDouble());
    }
}
void InvoiceCreateDialog::onLineDataChanged() {}

void InvoiceCreateDialog::onCancel() { reject(); }

void InvoiceCreateDialog::onArticleFromCatalog(int id, const QString &name, double price, double taxRate)
{
    InvoiceLineItem item;
    item.articleId = id;
    item.designation = name;
    item.quantity = 1;
    item.priceHT = price;
    item.taxRate = taxRate;

    m_lineItems.append(item);
    refreshLineTable();
}

void InvoiceCreateDialog::onClientSelected(int index)
{
    Q_UNUSED(index)
    int clientId = clientComboBox->currentData().toInt();

    if (clientId <= 0) {
        clientNomEdit->setReadOnly(false);
        clientAdresseEdit->setReadOnly(false);
        clientTelEdit->setReadOnly(false);
        clientEmailEdit->setReadOnly(false);

        clientNomEdit->clear();
        clientAdresseEdit->clear();
        clientTelEdit->clear();
        clientEmailEdit->clear();

        clientNomEdit->setStyleSheet(createStyledLineEdit("")->styleSheet());
        clientAdresseEdit->setStyleSheet(createStyledLineEdit("")->styleSheet());
        clientTelEdit->setStyleSheet(createStyledLineEdit("")->styleSheet());
        clientEmailEdit->setStyleSheet(createStyledLineEdit("")->styleSheet());

        return;
    }

    QSqlQuery q;
    q.prepare("SELECT nom, prenom, adresse, telephone, email "
              "FROM clients WHERE id = ?");
    q.addBindValue(clientId);
    if (q.exec() && q.next()) {
        clientNomEdit->setText(q.value(0).toString() + " " + q.value(1).toString());
        clientAdresseEdit->setText(q.value(2).toString());
        clientTelEdit->setText(q.value(3).toString());
        clientEmailEdit->setText(q.value(4).toString());

        clientNomEdit->setReadOnly(true);
        clientAdresseEdit->setReadOnly(true);
        clientTelEdit->setReadOnly(true);
        clientEmailEdit->setReadOnly(true);

        QString readOnlyStyle = R"(
            QLineEdit {
                border: 1px solid #E2E8F0;
                border-radius: 6px;
                padding: 6px 10px;
                font-size: 13px;
                background: #EDF2F7;
                color: #718096;
            }
        )";
        clientNomEdit->setStyleSheet(readOnlyStyle);
        clientAdresseEdit->setStyleSheet(readOnlyStyle);
        clientTelEdit->setStyleSheet(readOnlyStyle);
        clientEmailEdit->setStyleSheet(readOnlyStyle);
    }
}