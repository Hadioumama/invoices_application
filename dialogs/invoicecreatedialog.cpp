#include "invoicecreatedialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QTimer>
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
    setMinimumSize(900, 700);
    resize(960, 750);
    setStyleSheet("QDialog{background:#F5F7FA;}");

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);


    // Zone scrollable
    QScrollArea *scroll = new QScrollArea;
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setHorizontalScrollBarPolicy(
        Qt::ScrollBarAlwaysOff);

    QWidget *content = new QWidget;
    QVBoxLayout *cl = new QVBoxLayout(content);
    cl->setContentsMargins(16, 12, 16, 12);
    cl->setSpacing(8);
    cl->setAlignment(Qt::AlignTop);

    // ── 1. FACTURE ────────────────────────────────────
    cl->addWidget(createSectionTitle(
        "Informations Facture","📄"));
    QFrame *fc = createCard();
    QVBoxLayout *fl =
        qobject_cast<QVBoxLayout*>(fc->layout());

    QHBoxLayout *r1 = new QHBoxLayout;
    r1->setSpacing(10);
    QVBoxLayout *nc = new QVBoxLayout;
    nc->setSpacing(2);
    nc->addWidget(createFieldLabel("Numéro"));
    numeroEdit = createStyledLineEdit("FAC-2026-001");
    nc->addWidget(numeroEdit);
    r1->addLayout(nc,1);
    QVBoxLayout *tc = new QVBoxLayout;
    tc->setSpacing(2);
    tc->addWidget(createFieldLabel("Type"));
    typeCombo = new QComboBox;
    typeCombo->addItems({"Facture","Devis"});
    typeCombo->setFixedHeight(34);
    typeCombo->setStyleSheet(
        "QComboBox{border:1px solid #E2E8F0;"
        "border-radius:6px;padding:4px 10px;"
        "font-size:13px;background:#F7FAFC;"
        "color:#2D3748;}"
        "QComboBox:focus{border:1px solid #2B6CB0;}");
    tc->addWidget(typeCombo);
    r1->addLayout(tc,1);
    fl->addLayout(r1);

    QString ds =
        "QDateEdit{border:1px solid #E2E8F0;"
        "border-radius:6px;padding:4px 10px;"
        "font-size:13px;background:#F7FAFC;}"
        "QDateEdit:focus{border:1px solid #2B6CB0;}";
    QString cs =
        "QComboBox{border:1px solid #E2E8F0;"
        "border-radius:6px;padding:4px 10px;"
        "font-size:13px;background:#F7FAFC;"
        "color:#2D3748;}"
        "QComboBox:focus{border:1px solid #2B6CB0;}";

    QHBoxLayout *r2 = new QHBoxLayout;
    r2->setSpacing(10);
    QVBoxLayout *dc = new QVBoxLayout;
    dc->setSpacing(2);
    dc->addWidget(createFieldLabel("Date création"));
    dateCreationEdit = new QDateEdit(QDate::currentDate());
    dateCreationEdit->setCalendarPopup(true);
    dateCreationEdit->setFixedHeight(34);
    dateCreationEdit->setStyleSheet(ds);
    dc->addWidget(dateCreationEdit);
    r2->addLayout(dc,1);
    QVBoxLayout *dec = new QVBoxLayout;
    dec->setSpacing(2);
    dec->addWidget(createFieldLabel("Date échéance"));
    dateEcheanceEdit = new QDateEdit(
        QDate::currentDate().addDays(30));
    dateEcheanceEdit->setCalendarPopup(true);
    dateEcheanceEdit->setFixedHeight(34);
    dateEcheanceEdit->setStyleSheet(ds);
    dec->addWidget(dateEcheanceEdit);
    r2->addLayout(dec,1);
    QVBoxLayout *sc = new QVBoxLayout;
    sc->setSpacing(2);
    sc->addWidget(createFieldLabel("Statut"));
    statusCombo = new QComboBox;
    statusCombo->addItems({
        "Brouillon","Envoyée","Payée","Annulée"});
    statusCombo->setFixedHeight(34);
    statusCombo->setStyleSheet(cs);
    sc->addWidget(statusCombo);
    r2->addLayout(sc,1);
    fl->addLayout(r2);
    cl->addWidget(fc);

    // ── 2. PERSONNALISATION ───────────────────────────
    cl->addWidget(createSectionTitle(
        "Personnalisation","🎨"));
    QFrame *pc = createCard();
    QVBoxLayout *pl =
        qobject_cast<QVBoxLayout*>(pc->layout());
    QHBoxLayout *pr = new QHBoxLayout;
    pr->setSpacing(10);
    QString bfs =
        "QPushButton{background:#EDF2F7;color:#4A5568;"
        "border:1px solid #E2E8F0;border-radius:6px;}"
        "QPushButton:hover{background:#E2E8F0;}";
    QVBoxLayout *lgc = new QVBoxLayout;
    lgc->setSpacing(2);
    lgc->addWidget(createFieldLabel("Logo"));
    QHBoxLayout *lgr = new QHBoxLayout;
    lgr->setSpacing(4);
    logoPathEdit = createStyledLineEdit("Logo...");
    logoPathEdit->setReadOnly(true);
    logoBtn = new QPushButton("📁");
    logoBtn->setFixedSize(30,34);
    logoBtn->setStyleSheet(bfs);
    lgr->addWidget(logoPathEdit);
    lgr->addWidget(logoBtn);
    lgc->addLayout(lgr);
    pr->addLayout(lgc,1);
    QVBoxLayout *sgc = new QVBoxLayout;
    sgc->setSpacing(2);
    sgc->addWidget(createFieldLabel("Signature"));
    QHBoxLayout *sgr = new QHBoxLayout;
    sgr->setSpacing(4);
    signaturePathEdit = createStyledLineEdit("Signature...");
    signaturePathEdit->setReadOnly(true);
    signatureBtn = new QPushButton("📁");
    signatureBtn->setFixedSize(30,34);
    signatureBtn->setStyleSheet(bfs);
    sgr->addWidget(signaturePathEdit);
    sgr->addWidget(signatureBtn);
    sgc->addLayout(sgr);
    pr->addLayout(sgc,1);
    pl->addLayout(pr);
    cl->addWidget(pc);

    // ── 3. CLIENT ─────────────────────────────────────
    cl->addWidget(createSectionTitle(
        "Informations Client","👤"));
    QFrame *cc = createCard();
    QVBoxLayout *ccl =
        qobject_cast<QVBoxLayout*>(cc->layout());
    QHBoxLayout *cr1 = new QHBoxLayout;
    cr1->setSpacing(10);
    QVBoxLayout *cmc = new QVBoxLayout;
    cmc->setSpacing(2);
    cmc->addWidget(createFieldLabel("Client"));
    clientComboBox = new QComboBox;
    clientComboBox->setEditable(true);
    clientComboBox->addItem("-- Nouveau client --",-1);
    clientComboBox->setFixedHeight(34);
    clientComboBox->setStyleSheet(cs);
    QSqlQuery qc(
        "SELECT id,nom,prenom,adresse,telephone,email "
        "FROM clients WHERE role='client' ORDER BY nom");
    while(qc.next()){
        QString d = qc.value(1).toString()+" "+
                    qc.value(2).toString();
        int i = clientComboBox->count();
        clientComboBox->addItem(d,qc.value(0).toInt());
        clientComboBox->setItemData(i,qc.value(3),
            Qt::UserRole+1);
        clientComboBox->setItemData(i,qc.value(4),
            Qt::UserRole+2);
        clientComboBox->setItemData(i,qc.value(5),
            Qt::UserRole+3);
    }
    cmc->addWidget(clientComboBox);
    cr1->addLayout(cmc,1);
    QVBoxLayout *cnc = new QVBoxLayout;
    cnc->setSpacing(2);
    cnc->addWidget(createFieldLabel("Nom / Entreprise"));
    clientNomEdit = createStyledLineEdit("Nom...");
    cnc->addWidget(clientNomEdit);
    cr1->addLayout(cnc,1);
    ccl->addLayout(cr1);
    QVBoxLayout *cac = new QVBoxLayout;
    cac->setSpacing(2);
    cac->addWidget(createFieldLabel("Adresse"));
    clientAdresseEdit = createStyledLineEdit("Adresse...");
    cac->addWidget(clientAdresseEdit);
    ccl->addLayout(cac);
    QHBoxLayout *ctc = new QHBoxLayout;
    ctc->setSpacing(10);
    QVBoxLayout *tlc = new QVBoxLayout;
    tlc->setSpacing(2);
    tlc->addWidget(createFieldLabel("Téléphone"));
    clientTelEdit = createStyledLineEdit("+212...");
    tlc->addWidget(clientTelEdit);
    ctc->addLayout(tlc,1);
    QVBoxLayout *elc = new QVBoxLayout;
    elc->setSpacing(2);
    elc->addWidget(createFieldLabel("Email"));
    clientEmailEdit = createStyledLineEdit("email@...");
    elc->addWidget(clientEmailEdit);
    ctc->addLayout(elc,1);
    ccl->addLayout(ctc);
    cl->addWidget(cc);

    // ── 4. ARTICLES ───────────────────────────────────
    cl->addWidget(createSectionTitle(
        "Articles","🛒"));
    QFrame *ac = createCard();
    QVBoxLayout *acl =
        qobject_cast<QVBoxLayout*>(ac->layout());

    linesTable = new QTableWidget(0,5);
    linesTable->setHorizontalHeaderLabels({
        "Désignation","Qté","Prix HT","TVA%","Total HT"});
    linesTable->horizontalHeader()
              ->setStretchLastSection(true);
    linesTable->setEditTriggers(
        QAbstractItemView::NoEditTriggers);
    linesTable->setSelectionBehavior(
        QAbstractItemView::SelectRows);
    linesTable->setAlternatingRowColors(true);
    linesTable->setColumnWidth(0,280);
    linesTable->setColumnWidth(1,50);
    linesTable->setColumnWidth(2,80);
    linesTable->setColumnWidth(3,60);
    linesTable->setFixedHeight(130);
    linesTable->setStyleSheet(
        "QTableWidget{border:1px solid #3d7bcd;"
        "border-radius:6px;background:white;"
        "gridline-color:#EDF2F7;}"
        "QHeaderView::section{background:#2B6CB0;"
        "color:white;font-weight:bold;"
        "font-size:12px;padding:5px;border:none;}"
        "QTableWidget::item{padding:4px;"
        "font-size:12px;}"
        "QTableWidget::item:selected{"
        "background:#EBF8FF;color:#2B6CB0;}"
        "QTableWidget::item:alternate{"
        "background:#F7FAFC;}");
    acl->addWidget(linesTable);

    // Saisie ligne
    QFrame *nlf = new QFrame;
    nlf->setStyleSheet(
        "QFrame{background:#EBF8FF;"
        "border:1px solid #BEE3F8;"
        "border-radius:8px;}");
    QHBoxLayout *nll = new QHBoxLayout(nlf);
    nll->setContentsMargins(10,6,10,6);
    nll->setSpacing(8);

    QString ss =
        "border:1px solid #90CDF4;"
        "border-radius:6px;padding:2px;"
        "font-size:12px;background:white;";

    // Désignation
    QVBoxLayout *dlc = new QVBoxLayout;
    dlc->setSpacing(1);
    QLabel *dll = new QLabel("Désignation:");
    dll->setStyleSheet(
        "font-size:11px;font-weight:600;");
    dlc->addWidget(dll);
    designationEdit = new QComboBox;
    designationEdit->setEditable(true);
    designationEdit->addItem("-- Saisie manuelle --",-1);
    designationEdit->setMinimumWidth(160);
    designationEdit->setFixedHeight(28);
    designationEdit->setStyleSheet(
        "QComboBox{border:1px solid #90CDF4;"
        "border-radius:6px;padding:2px 8px;"
        "font-size:12px;background:white;}"
        "QComboBox:focus{border:1px solid #2B6CB0;}");
    dlc->addWidget(designationEdit);
    nll->addLayout(dlc,2);

    // Qté
    QVBoxLayout *qlc = new QVBoxLayout;
    qlc->setSpacing(1);
    qlc->addWidget(new QLabel("Qté:"));
    quantitySpinBox = new QSpinBox;
    quantitySpinBox->setRange(1,9999);
    quantitySpinBox->setValue(1);
    quantitySpinBox->setFixedSize(55,28);
    quantitySpinBox->setStyleSheet(
        "QSpinBox{"+ss+"}"
        "QSpinBox:focus{border:1px solid #2B6CB0;}");
    qlc->addWidget(quantitySpinBox);
    nll->addLayout(qlc);

    // Prix HT
    QVBoxLayout *plc = new QVBoxLayout;
    plc->setSpacing(1);
    plc->addWidget(new QLabel("Prix HT:"));
    priceHTSpinBox = new QDoubleSpinBox;
    priceHTSpinBox->setRange(0,999999);
    priceHTSpinBox->setDecimals(2);
    priceHTSpinBox->setSuffix(" MAD");
    priceHTSpinBox->setFixedSize(95,28);
    priceHTSpinBox->setStyleSheet(
        "QDoubleSpinBox{"+ss+"}"
        "QDoubleSpinBox:focus{"
        "border:1px solid #2B6CB0;}");
    plc->addWidget(priceHTSpinBox);
    nll->addLayout(plc);

    // TVA
    QVBoxLayout *tvlc = new QVBoxLayout;
    tvlc->setSpacing(1);
    tvlc->addWidget(new QLabel("TVA%:"));
    taxRateSpinBox = new QDoubleSpinBox;
    taxRateSpinBox->setRange(0,100);
    taxRateSpinBox->setValue(20);
    taxRateSpinBox->setDecimals(2);
    taxRateSpinBox->setSuffix("%");
    taxRateSpinBox->setFixedSize(70,28);
    taxRateSpinBox->setStyleSheet(
        "QDoubleSpinBox{"+ss+"}"
        "QDoubleSpinBox:focus{"
        "border:1px solid #2B6CB0;}");
    tvlc->addWidget(taxRateSpinBox);
    nll->addLayout(tvlc);

    // Boutons ajouter/supprimer
    addLineBtn = new QPushButton("➕ Ajouter");
    addLineBtn->setFixedSize(95,32);
    addLineBtn->setStyleSheet(
        "QPushButton{background:#2B6CB0;color:white;"
        "font-size:11px;font-weight:bold;border:none;"
        "border-radius:6px;}"
        "QPushButton:hover{background:#1A365D;}");
    removeLineBtn = new QPushButton("🗑️ Supp.");
    removeLineBtn->setFixedSize(80,32);
    removeLineBtn->setStyleSheet(
        "QPushButton{background:#E53E3E;color:white;"
        "font-size:11px;font-weight:bold;border:none;"
        "border-radius:6px;}"
        "QPushButton:hover{background:#C53030;}");
    nll->addWidget(addLineBtn);
    nll->addWidget(removeLineBtn);
    acl->addWidget(nlf);

    // Totaux
    QHBoxLayout *tl = new QHBoxLayout;
    tl->addStretch();
    totalHTLabel = new QLabel("HT: 0.00 MAD");
    totalTVALabel = new QLabel("TVA: 0.00 MAD");
    totalTTCLabel = new QLabel("TTC: 0.00 MAD");
    totalTTCLabel->setStyleSheet(
        "font-size:13px;font-weight:bold;"
        "color:#1B2A3B;");
    tl->addWidget(totalHTLabel);
    tl->addSpacing(12);
    tl->addWidget(totalTVALabel);
    tl->addSpacing(12);
    tl->addWidget(totalTTCLabel);
    acl->addLayout(tl);
    cl->addWidget(ac);

  // ── ATTACHER SCROLL ───────────────────────────────
scroll->setWidget(content);
m_scroll = scroll;

// ── FOOTER ────────────────────────────────────────
m_footer = new QWidget(this); // ← parent = this, PAS dans layout
m_footer->setFixedHeight(56);
m_footer->setStyleSheet(
    "background:#FFFFFF;"
    "border-top:2px solid #E2E8F0;");

QHBoxLayout *fbl = new QHBoxLayout(m_footer);
fbl->setContentsMargins(16, 8, 16, 8);
fbl->setSpacing(10);
fbl->addStretch();

cancelBtn = new QPushButton("❌ Annuler");
cancelBtn->setFixedSize(120, 38);
cancelBtn->setStyleSheet(
    "QPushButton{background:#F7FAFC;"
    "color:#4A5568;font-weight:600;"
    "font-size:13px;border:1px solid #CBD5E0;"
    "border-radius:8px;}"
    "QPushButton:hover{background:#EDF2F7;}");

saveBtn = new QPushButton("💾 Enregistrer");
saveBtn->setFixedSize(140, 38);
saveBtn->setStyleSheet(
    "QPushButton{background:#38A169;"
    "color:white;font-weight:bold;"
    "font-size:13px;border:none;"
    "border-radius:8px;}"
    "QPushButton:hover{background:#276749;}");

fbl->addWidget(cancelBtn);
fbl->addWidget(saveBtn);

// ← scroll prend tout sauf 56px du footer
mainLayout->addWidget(scroll);

// ── CONNEXIONS ────────────────────────────────────
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
connect(logoBtn, &QPushButton::clicked, [this](){
    QString p = QFileDialog::getOpenFileName(
        this, "Logo", "",
        "Images (*.png *.jpg *.jpeg)");
    if (!p.isEmpty()) logoPathEdit->setText(p);
});
connect(signatureBtn, &QPushButton::clicked, [this](){
    QString p = QFileDialog::getOpenFileName(
        this, "Signature", "",
        "Images (*.png *.jpg *.jpeg)");
    if (!p.isEmpty()) signaturePathEdit->setText(p);
});

loadArticles();
}
void InvoiceCreateDialog::resizeEvent(QResizeEvent *event)
{
    QDialog::resizeEvent(event);
    if (m_footer && m_scroll) {
        int fh = 56;
        int w  = width();
        int h  = height();
        // Footer collé en bas
        m_footer->setGeometry(0, h - fh, w, fh);
        // Scroll prend tout sauf le footer
        m_scroll->setGeometry(0, 0, w, h - fh);
    }
}

void InvoiceCreateDialog::showEvent(QShowEvent *event)
{
    QDialog::showEvent(event);
    // Forcer le positionnement dès l'affichage
    resizeEvent(nullptr);
    if (m_footer)
        m_footer->raise(); // ← s'assure qu'il est au-dessus
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