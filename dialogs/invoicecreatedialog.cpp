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
    setWindowTitle(m_isEditMode ? 
        "Modifier la Facture" : "Créer une Facture");
    setMinimumSize(900, 800);
    resize(960, );
    setStyleSheet("QDialog { background-color: #F5F7FA; }");

    // Layout principal
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // ── SCROLL AREA ──────────────────────────────────────
    QScrollArea *scrollArea = new QScrollArea;
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setHorizontalScrollBarPolicy(
        Qt::ScrollBarAlwaysOff);
    scrollArea->setStyleSheet(
        "QScrollArea{background:#F5F7FA;border:none;}"
        "QScrollBar:vertical{background:transparent;"
        "width:5px;}"
        "QScrollBar::handle:vertical{background:#6198d3;"
        "border-radius:2px;min-height:10px;}"
        "QScrollBar::add-line:vertical,"
        "QScrollBar::sub-line:vertical{height:0px;}");

    // Contenu scrollable
    QWidget *scrollContent = new QWidget;
    scrollContent->setStyleSheet(
        "background:#F5F7FA;");
    QVBoxLayout *contentLayout =
        new QVBoxLayout(scrollContent);
    contentLayout->setSpacing(6);
    contentLayout->setContentsMargins(20, 16, 20, 8);
    contentLayout->setAlignment(Qt::AlignTop);

    // ════════════════════════════════════════════════════
    // 1. INFORMATIONS FACTURE
    // ════════════════════════════════════════════════════
    contentLayout->addWidget(
        createSectionTitle("Informations Facture","📄"));

    QFrame *factureCard = createCard();
    QVBoxLayout *factureLayout =
        qobject_cast<QVBoxLayout*>(factureCard->layout());

    QHBoxLayout *row1 = new QHBoxLayout;
    row1->setSpacing(10);
    QVBoxLayout *numCol = new QVBoxLayout;
    numCol->setSpacing(2);
    numCol->addWidget(createFieldLabel("Numéro"));
    numeroEdit = createStyledLineEdit("FAC-2026-0001");
    numCol->addWidget(numeroEdit);
    row1->addLayout(numCol, 1);
    QVBoxLayout *typeCol = new QVBoxLayout;
    typeCol->setSpacing(2);
    typeCol->addWidget(createFieldLabel("Type"));
    typeCombo = new QComboBox;
    typeCombo->addItems({"Facture","Devis"});
    typeCombo->setFixedHeight(34);
    typeCombo->setStyleSheet(
        "QComboBox{border:1px solid #E2E8F0;"
        "border-radius:6px;padding:4px 10px;"
        "font-size:13px;background:#F7FAFC;color:#2D3748;}"
        "QComboBox:focus{border:1px solid #2B6CB0;}"
        "QComboBox::drop-down{border:none;width:24px;}"
        "QComboBox::down-arrow{image:none;"
        "border-left:4px solid transparent;"
        "border-right:4px solid transparent;"
        "border-top:5px solid #4068a3;"
        "width:0px;height:0px;}");
    typeCol->addWidget(typeCombo);
    row1->addLayout(typeCol, 1);
    factureLayout->addLayout(row1);

    QHBoxLayout *row2 = new QHBoxLayout;
    row2->setSpacing(10);
    QString dateStyle =
        "QDateEdit{border:1px solid #E2E8F0;"
        "border-radius:6px;padding:4px 10px;"
        "font-size:13px;background:#F7FAFC;}"
        "QDateEdit:focus{border:1px solid #2B6CB0;}";
    QString comboStyle =
        "QComboBox{border:1px solid #E2E8F0;"
        "border-radius:6px;padding:4px 10px;"
        "font-size:13px;background:#F7FAFC;color:#2D3748;}"
        "QComboBox:focus{border:1px solid #2B6CB0;}"
        "QComboBox::drop-down{border:none;width:24px;}"
        "QComboBox::down-arrow{image:none;"
        "border-left:4px solid transparent;"
        "border-right:4px solid transparent;"
        "border-top:5px solid #2B6CB0;"
        "width:0px;height:0px;}";

    QVBoxLayout *dateCreaCol = new QVBoxLayout;
    dateCreaCol->setSpacing(2);
    dateCreaCol->addWidget(createFieldLabel("Date création"));
    dateCreationEdit = new QDateEdit(QDate::currentDate());
    dateCreationEdit->setCalendarPopup(true);
    dateCreationEdit->setFixedHeight(34);
    dateCreationEdit->setStyleSheet(dateStyle);
    dateCreaCol->addWidget(dateCreationEdit);
    row2->addLayout(dateCreaCol, 1);

    QVBoxLayout *dateEchCol = new QVBoxLayout;
    dateEchCol->setSpacing(2);
    dateEchCol->addWidget(
        createFieldLabel("Date échéance"));
    dateEcheanceEdit = new QDateEdit(
        QDate::currentDate().addDays(30));
    dateEcheanceEdit->setCalendarPopup(true);
    dateEcheanceEdit->setFixedHeight(34);
    dateEcheanceEdit->setStyleSheet(dateStyle);
    dateEchCol->addWidget(dateEcheanceEdit);
    row2->addLayout(dateEchCol, 1);

    QVBoxLayout *statusCol = new QVBoxLayout;
    statusCol->setSpacing(2);
    statusCol->addWidget(createFieldLabel("Statut"));
    statusCombo = new QComboBox;
    statusCombo->addItems({
        "Brouillon","Envoyée","Payée","Annulée"});
    statusCombo->setFixedHeight(34);
    statusCombo->setStyleSheet(comboStyle);
    statusCol->addWidget(statusCombo);
    row2->addLayout(statusCol, 1);
    factureLayout->addLayout(row2);
    contentLayout->addWidget(factureCard);

    // ════════════════════════════════════════════════════
    // 2. PERSONNALISATION
    // ════════════════════════════════════════════════════
    contentLayout->addSpacing(4);
    contentLayout->addWidget(createSectionTitle(
        "Personnalisation (Entreprise)","🎨"));

    QFrame *persoCard = createCard();
    QVBoxLayout *persoLayout =
        qobject_cast<QVBoxLayout*>(persoCard->layout());

    QHBoxLayout *persoRow = new QHBoxLayout;
    persoRow->setSpacing(10);

    QString btnFileStyle =
        "QPushButton{background:#EDF2F7;color:#4A5568;"
        "font-size:12px;border:1px solid #E2E8F0;"
        "border-radius:6px;}"
        "QPushButton:hover{background:#E2E8F0;}";

    QVBoxLayout *logoCol = new QVBoxLayout;
    logoCol->setSpacing(2);
    logoCol->addWidget(createFieldLabel("Logo entreprise"));
    QHBoxLayout *logoRow = new QHBoxLayout;
    logoRow->setSpacing(6);
    logoPathEdit = createStyledLineEdit("Chemin du logo...");
    logoPathEdit->setReadOnly(true);
    logoBtn = new QPushButton("📁");
    logoBtn->setFixedSize(30, 34);
    logoBtn->setCursor(Qt::PointingHandCursor);
    logoBtn->setStyleSheet(btnFileStyle);
    logoRow->addWidget(logoPathEdit);
    logoRow->addWidget(logoBtn);
    logoCol->addLayout(logoRow);
    persoRow->addLayout(logoCol, 1);

    QVBoxLayout *signCol = new QVBoxLayout;
    signCol->setSpacing(2);
    signCol->addWidget(createFieldLabel("Signature"));
    QHBoxLayout *signRow = new QHBoxLayout;
    signRow->setSpacing(6);
    signaturePathEdit = createStyledLineEdit(
        "Chemin de la signature...");
    signaturePathEdit->setReadOnly(true);
    signatureBtn = new QPushButton("📁");
    signatureBtn->setFixedSize(30, 34);
    signatureBtn->setCursor(Qt::PointingHandCursor);
    signatureBtn->setStyleSheet(btnFileStyle);
    signRow->addWidget(signaturePathEdit);
    signRow->addWidget(signatureBtn);
    signCol->addLayout(signRow);
    persoRow->addLayout(signCol, 1);

    persoLayout->addLayout(persoRow);
    contentLayout->addWidget(persoCard);

    // ════════════════════════════════════════════════════
    // 3. INFORMATIONS CLIENT
    // ════════════════════════════════════════════════════
    contentLayout->addSpacing(4);
    contentLayout->addWidget(
        createSectionTitle("Informations Client","👤"));

    QFrame *clientCard = createCard();
    QVBoxLayout *clientLayout =
        qobject_cast<QVBoxLayout*>(clientCard->layout());

    QHBoxLayout *clientRow1 = new QHBoxLayout;
    clientRow1->setSpacing(10);

    QVBoxLayout *comboCol = new QVBoxLayout;
    comboCol->setSpacing(2);
    comboCol->addWidget(createFieldLabel("Client"));
    clientComboBox = new QComboBox;
    clientComboBox->setEditable(true);
    clientComboBox->addItem("-- Nouveau client --", -1);
    clientComboBox->setFixedHeight(34);
    clientComboBox->setStyleSheet(comboStyle);

    QSqlQuery qc(
        "SELECT id,nom,prenom,adresse,telephone,email "
        "FROM clients WHERE role='client' ORDER BY nom");
    while (qc.next()) {
        QString d = qc.value(1).toString() + " " +
                    qc.value(2).toString();
        int idx = clientComboBox->count();
        clientComboBox->addItem(d, qc.value(0).toInt());
        clientComboBox->setItemData(
            idx, qc.value(3), Qt::UserRole+1);
        clientComboBox->setItemData(
            idx, qc.value(4), Qt::UserRole+2);
        clientComboBox->setItemData(
            idx, qc.value(5), Qt::UserRole+3);
    }
    comboCol->addWidget(clientComboBox);
    clientRow1->addLayout(comboCol, 1);

    QVBoxLayout *nomCol = new QVBoxLayout;
    nomCol->setSpacing(2);
    nomCol->addWidget(createFieldLabel("Nom / Entreprise"));
    clientNomEdit = createStyledLineEdit("Nom complet...");
    nomCol->addWidget(clientNomEdit);
    clientRow1->addLayout(nomCol, 1);
    clientLayout->addLayout(clientRow1);

    QVBoxLayout *adrCol = new QVBoxLayout;
    adrCol->setSpacing(2);
    adrCol->addWidget(createFieldLabel("Adresse"));
    clientAdresseEdit = createStyledLineEdit(
        "Adresse complète...");
    adrCol->addWidget(clientAdresseEdit);
    clientLayout->addLayout(adrCol);

    QHBoxLayout *contactRow = new QHBoxLayout;
    contactRow->setSpacing(10);
    QVBoxLayout *telCol = new QVBoxLayout;
    telCol->setSpacing(2);
    telCol->addWidget(createFieldLabel("Téléphone"));
    clientTelEdit = createStyledLineEdit("+212 6XX XXX XXX");
    telCol->addWidget(clientTelEdit);
    contactRow->addLayout(telCol, 1);
    QVBoxLayout *emailCol = new QVBoxLayout;
    emailCol->setSpacing(2);
    emailCol->addWidget(createFieldLabel("Email"));
    clientEmailEdit = createStyledLineEdit(
        "email@exemple.com");
    emailCol->addWidget(clientEmailEdit);
    contactRow->addLayout(emailCol, 1);
    clientLayout->addLayout(contactRow);
    contentLayout->addWidget(clientCard);

    // ════════════════════════════════════════════════════
    // 4. ARTICLES
    // ════════════════════════════════════════════════════
    contentLayout->addSpacing(4);
    contentLayout->addWidget(createSectionTitle(
        "Articles de la facture","🛒"));

    QFrame *articlesCard = createCard();
    QVBoxLayout *articlesLayout =
        qobject_cast<QVBoxLayout*>(articlesCard->layout());

    linesTable = new QTableWidget(0, 5);
    linesTable->setHorizontalHeaderLabels({
        "Désignation","Qté","Prix HT","TVA %","Total HT"});
    linesTable->horizontalHeader()
              ->setStretchLastSection(true);
    linesTable->setEditTriggers(
        QAbstractItemView::NoEditTriggers);
    linesTable->setSelectionBehavior(
        QAbstractItemView::SelectRows);
    linesTable->setColumnWidth(0, 300);
    linesTable->setColumnWidth(1, 55);
    linesTable->setColumnWidth(2, 85);
    linesTable->setColumnWidth(3, 65);
    linesTable->setAlternatingRowColors(true);
    linesTable->setStyleSheet(
        "QTableWidget{border:1px solid #3d7bcd;"
        "border-radius:6px;background:white;"
        "gridline-color:#EDF2F7;}"
        "QHeaderView::section{background:#2B6CB0;"
        "color:white;font-weight:bold;font-size:12px;"
        "padding:5px;border:none;}"
        "QTableWidget::item{padding:4px;"
        "border-bottom:1px solid #EDF2F7;font-size:12px;}"
        "QTableWidget::item:selected{"
        "background:#EBF8FF;color:#2B6CB0;}"
        "QTableWidget::item:alternate{"
        "background:#F7FAFC;}");
    linesTable->setMinimumHeight(80);
    linesTable->setMaximumHeight(150);
    articlesLayout->addWidget(linesTable);

    // Saisie nouvelle ligne
    QFrame *newLineFrame = new QFrame;
    newLineFrame->setStyleSheet(
        "QFrame{background:#EBF8FF;"
        "border:1px solid #BEE3F8;border-radius:8px;}");
    QHBoxLayout *newLineLayout =
        new QHBoxLayout(newLineFrame);
    newLineLayout->setContentsMargins(10, 6, 10, 6);
    newLineLayout->setSpacing(8);

    QString spinStyle =
        "border:1px solid #90CDF4;border-radius:6px;"
        "padding:2px;font-size:12px;background:white;";

    QVBoxLayout *desCol = new QVBoxLayout;
    desCol->setSpacing(1);
    QLabel *desLbl = new QLabel("Désignation:");
    desLbl->setStyleSheet(
        "font-size:11px;font-weight:600;color:#030a17;");
    desCol->addWidget(desLbl);
    designationEdit = new QComboBox;
    designationEdit->setEditable(true);
    designationEdit->addItem("-- Saisie manuelle --", -1);
    designationEdit->setMinimumWidth(180);
    designationEdit->setFixedHeight(28);
    designationEdit->setStyleSheet(
        "QComboBox{border:1px solid #90CDF4;"
        "border-radius:6px;padding:2px 8px;"
        "font-size:12px;background:white;color:#2D3748;}"
        "QComboBox:focus{border:1px solid #2B6CB0;}"
        "QComboBox::drop-down{border:none;width:20px;}"
        "QComboBox::down-arrow{image:none;"
        "border-left:3px solid transparent;"
        "border-right:3px solid transparent;"
        "border-top:5px solid #2B6CB0;"
        "width:0px;height:0px;}");
    desCol->addWidget(designationEdit);
    newLineLayout->addLayout(desCol, 2);

    QVBoxLayout *qteCol = new QVBoxLayout;
    qteCol->setSpacing(1);
    QLabel *qteLbl = new QLabel("Qté:");
    qteLbl->setStyleSheet(
        "font-size:11px;font-weight:600;color:#010a1a;");
    qteCol->addWidget(qteLbl);
    quantitySpinBox = new QSpinBox;
    quantitySpinBox->setMinimum(1);
    quantitySpinBox->setMaximum(9999);
    quantitySpinBox->setValue(1);
    quantitySpinBox->setFixedWidth(55);
    quantitySpinBox->setFixedHeight(28);
    quantitySpinBox->setStyleSheet(
        "QSpinBox{" + spinStyle + "}"
        "QSpinBox:focus{border:1px solid #2B6CB0;}");
    qteCol->addWidget(quantitySpinBox);
    newLineLayout->addLayout(qteCol);

    QVBoxLayout *prixCol = new QVBoxLayout;
    prixCol->setSpacing(1);
    QLabel *prixLbl = new QLabel("Prix HT:");
    prixLbl->setStyleSheet(
        "font-size:11px;font-weight:600;color:#020c1e;");
    prixCol->addWidget(prixLbl);
    priceHTSpinBox = new QDoubleSpinBox;
    priceHTSpinBox->setMaximum(999999);
    priceHTSpinBox->setDecimals(2);
    priceHTSpinBox->setFixedWidth(95);
    priceHTSpinBox->setFixedHeight(28);
    priceHTSpinBox->setSuffix(" MAD");
    priceHTSpinBox->setStyleSheet(
        "QDoubleSpinBox{" + spinStyle + "}"
        "QDoubleSpinBox:focus{border:1px solid #2B6CB0;}");
    prixCol->addWidget(priceHTSpinBox);
    newLineLayout->addLayout(prixCol);

    QVBoxLayout *tvaCol = new QVBoxLayout;
    tvaCol->setSpacing(1);
    QLabel *tvaLbl = new QLabel("TVA %:");
    tvaLbl->setStyleSheet(
        "font-size:11px;font-weight:600;color:#010c1f;");
    tvaCol->addWidget(tvaLbl);
    taxRateSpinBox = new QDoubleSpinBox;
    taxRateSpinBox->setValue(20.0);
    taxRateSpinBox->setMaximum(100);
    taxRateSpinBox->setDecimals(2);
    taxRateSpinBox->setFixedWidth(70);
    taxRateSpinBox->setFixedHeight(28);
    taxRateSpinBox->setSuffix(" %");
    taxRateSpinBox->setStyleSheet(
        "QDoubleSpinBox{" + spinStyle + "}"
        "QDoubleSpinBox:focus{border:1px solid #2B6CB0;}");
    tvaCol->addWidget(taxRateSpinBox);
    newLineLayout->addLayout(tvaCol);

    addLineBtn = new QPushButton("➕ Ajouter");
    addLineBtn->setFixedSize(100, 32);
    addLineBtn->setCursor(Qt::PointingHandCursor);
    addLineBtn->setStyleSheet(
        "QPushButton{background:#2B6CB0;color:white;"
        "font-size:12px;font-weight:bold;border:none;"
        "border-radius:6px;padding:4px 12px;}"
        "QPushButton:hover{background:#1A365D;}");

    removeLineBtn = new QPushButton("🗑️ Supprimer");
    removeLineBtn->setFixedSize(100, 32);
    removeLineBtn->setCursor(Qt::PointingHandCursor);
    removeLineBtn->setStyleSheet(
        "QPushButton{background:#E53E3E;color:white;"
        "font-size:12px;font-weight:bold;border:none;"
        "border-radius:6px;padding:4px 12px;}"
        "QPushButton:hover{background:#C53030;}");

    newLineLayout->addWidget(addLineBtn);
    newLineLayout->addWidget(removeLineBtn);
    articlesLayout->addWidget(newLineFrame);

    // Totaux
    QHBoxLayout *totalsLayout = new QHBoxLayout;
    totalsLayout->addStretch();
    totalHTLabel = new QLabel("Total HT: 0.00 MAD");
    totalHTLabel->setStyleSheet("font-size:13px;color:#333;");
    totalTVALabel = new QLabel("TVA: 0.00 MAD");
    totalTVALabel->setStyleSheet("font-size:13px;color:#333;");
    totalTTCLabel = new QLabel("Total TTC: 0.00 MAD");
    totalTTCLabel->setStyleSheet(
        "font-size:14px;font-weight:bold;color:#1B2A3B;");
    totalsLayout->addWidget(totalHTLabel);
    totalsLayout->addSpacing(14);
    totalsLayout->addWidget(totalTVALabel);
    totalsLayout->addSpacing(14);
    totalsLayout->addWidget(totalTTCLabel);
    articlesLayout->addLayout(totalsLayout);
    contentLayout->addWidget(articlesCard);

    // ════════════════════════════════════════════════════
    // ATTACHER CONTENU AU SCROLL — UNE SEULE FOIS
    // ════════════════════════════════════════════════════
    scrollArea->setWidget(scrollContent);

    // ════════════════════════════════════════════════════
    // FOOTER FIXE EN BAS — HORS DU SCROLL
    // ════════════════════════════════════════════════════
    QFrame *footerFrame = new QFrame;
    footerFrame->setFixedHeight(58);
    footerFrame->setStyleSheet(
        "QFrame{background:white;"
        "border-top:1px solid #E2E8F0;}");

    QHBoxLayout *footerLayout =
        new QHBoxLayout(footerFrame);
    footerLayout->setContentsMargins(20, 8, 20, 8);
    footerLayout->setSpacing(12);
    footerLayout->addStretch();

    cancelBtn = new QPushButton("❌ Annuler");
    cancelBtn->setFixedSize(120, 38);
    cancelBtn->setCursor(Qt::PointingHandCursor);
    cancelBtn->setStyleSheet(
        "QPushButton{background:#F7FAFC;color:#4A5568;"
        "font-weight:600;font-size:13px;"
        "border:1px solid #CBD5E0;border-radius:8px;}"
        "QPushButton:hover{background:#EDF2F7;}");

    saveBtn = new QPushButton("💾 Enregistrer");
    saveBtn->setFixedSize(140, 38);
    saveBtn->setCursor(Qt::PointingHandCursor);
    saveBtn->setStyleSheet(
        "QPushButton{background:#38A169;color:white;"
        "font-weight:bold;font-size:13px;"
        "border:none;border-radius:8px;}"
        "QPushButton:hover{background:#276749;}");

    footerLayout->addWidget(cancelBtn);
    footerLayout->addWidget(saveBtn);

    // ════════════════════════════════════════════════════
    // ASSEMBLER LE LAYOUT PRINCIPAL
    // ════════════════════════════════════════════════════
    mainLayout->addWidget(scrollArea, 1); // stretch=1
    mainLayout->addWidget(footerFrame, 0); // stretch=0

    // ════════════════════════════════════════════════════
    // CONNEXIONS
    // ════════════════════════════════════════════════════
    connect(clientComboBox,
            QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &InvoiceCreateDialog::onClientSelected);
    connect(designationEdit,
            QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &InvoiceCreateDialog::onArticleSelected);
    connect(addLineBtn, &QPushButton::clicked,
            this, &InvoiceCreateDialog::onAddLine);
    connect(removeLineBtn, &QPushButton::clicked,
            this, &InvoiceCreateDialog::onRemoveLine);
    connect(saveBtn, &QPushButton::clicked,
            this, &InvoiceCreateDialog::onSave);
    connect(cancelBtn, &QPushButton::clicked,
            this, &InvoiceCreateDialog::onCancel);
    connect(logoBtn, &QPushButton::clicked, [this]() {
        QString p = QFileDialog::getOpenFileName(
            this, "Choisir le logo", "",
            "Images (*.png *.jpg *.jpeg)");
        if (!p.isEmpty()) logoPathEdit->setText(p);
    });
    connect(signatureBtn, &QPushButton::clicked, [this]() {
        QString p = QFileDialog::getOpenFileName(
            this, "Choisir la signature", "",
            "Images (*.png *.jpg *.jpeg)");
        if (!p.isEmpty()) signaturePathEdit->setText(p);
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