#include "invoiceeditdialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QScrollArea>
#include <QMessageBox>
#include <QSqlQuery>
#include <QHeaderView>
#include <QSqlError>
#include <QDate>
#include <QDebug>

InvoiceEditDialog::InvoiceEditDialog(int invoiceId, QWidget *parent)
    : QDialog(parent), m_invoiceId(invoiceId)
{
    setupUI();
    loadData();
    loadLines(); 
}

void InvoiceEditDialog::setupUI()
{
    setWindowTitle("Modifier la Facture");
    setMinimumSize(750, 700);
    resize(800, 750);

    QScrollArea *scroll = new QScrollArea(this);
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);

    QWidget *container = new QWidget;
    QVBoxLayout *mainLayout = new QVBoxLayout(container);
    mainLayout->setSpacing(10);
    mainLayout->setContentsMargins(16, 16, 16, 16);

    // Style global
    setStyleSheet(
        "QGroupBox {"
        "  font-weight: bold;"
        "  font-size: 12px;"
        "  border: 1px solid #CBD5E0;"
        "  border-radius: 6px;"
        "  margin-top: 8px;"
        "  padding-top: 8px;"
        "}"
        "QGroupBox::title {"
        "  subcontrol-origin: margin;"
        "  left: 10px;"
        "  color: #2B6CB0;"
        "}"
        "QLineEdit, QDateEdit, QComboBox {"
        "  border: 1px solid #CBD5E0;"
        "  border-radius: 4px;"
        "  padding: 5px 8px;"
        "  background: white;"
        "  min-height: 28px;"
        "}"
        "QLineEdit:focus, QDateEdit:focus {"
        "  border: 1px solid #3182CE;"
        "}"
    );

    // ── GROUPE FACTURE ──────────────────────────────────────
    QGroupBox *factureGroup = new QGroupBox("📄 Informations Facture");
    QFormLayout *factureForm = new QFormLayout(factureGroup);
    factureForm->setSpacing(8);
    factureForm->setLabelAlignment(Qt::AlignRight);

    numeroEdit = new QLineEdit;
    typeCombo = new QComboBox;
    typeCombo->addItems({"Facture", "Devis"});
    statusCombo = new QComboBox;
    statusCombo->addItems({"Brouillon", "Envoyée", "Payée", "Annulée"});
    dateCreationEdit = new QDateEdit;
    dateCreationEdit->setCalendarPopup(true);
    dateCreationEdit->setDisplayFormat("dd/MM/yyyy");
    dateEcheanceEdit = new QDateEdit;
    dateEcheanceEdit->setCalendarPopup(true);
    dateEcheanceEdit->setDisplayFormat("dd/MM/yyyy");

    factureForm->addRow("Numéro:", numeroEdit);
    factureForm->addRow("Type:", typeCombo);
    factureForm->addRow("Statut:", statusCombo);
    factureForm->addRow("Date création:", dateCreationEdit);
    factureForm->addRow("Date échéance:", dateEcheanceEdit);
    mainLayout->addWidget(factureGroup);

    // ── GROUPE CLIENT ───────────────────────────────────────
    QGroupBox *clientGroup = new QGroupBox("👤 Informations Client");
    QFormLayout *clientForm = new QFormLayout(clientGroup);
    clientForm->setSpacing(8);
    clientForm->setLabelAlignment(Qt::AlignRight);

    clientNomEdit = new QLineEdit;
    clientAdresseEdit = new QLineEdit;
    clientTelEdit = new QLineEdit;
    clientEmailEdit = new QLineEdit;

    clientForm->addRow("Nom / Entreprise:", clientNomEdit);
    clientForm->addRow("Adresse:", clientAdresseEdit);
    clientForm->addRow("Téléphone:", clientTelEdit);
    clientForm->addRow("Email:", clientEmailEdit);
    mainLayout->addWidget(clientGroup);

    // ── GROUPE ARTICLES ─────────────────────────────────────
    QGroupBox *lignesGroup = new QGroupBox("🛒 Articles de la facture");
    QVBoxLayout *lignesLayout = new QVBoxLayout(lignesGroup);
    lignesLayout->setSpacing(8);

    // Tableau
    linesTable = new QTableWidget(0, 5);
    linesTable->setHorizontalHeaderLabels(
        {"Désignation", "Qté", "Prix HT", "TVA%", "Total HT"});
    linesTable->horizontalHeader()->setSectionResizeMode(
        0, QHeaderView::Stretch);
    linesTable->horizontalHeader()->setSectionResizeMode(
        1, QHeaderView::Fixed);
    linesTable->horizontalHeader()->setSectionResizeMode(
        2, QHeaderView::Fixed);
    linesTable->horizontalHeader()->setSectionResizeMode(
        3, QHeaderView::Fixed);
    linesTable->horizontalHeader()->setSectionResizeMode(
        4, QHeaderView::Fixed);
    linesTable->setColumnWidth(1, 55);
    linesTable->setColumnWidth(2, 95);
    linesTable->setColumnWidth(3, 65);
    linesTable->setColumnWidth(4, 95);
    linesTable->verticalHeader()->setVisible(false);
    linesTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    linesTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    linesTable->setSelectionMode(QAbstractItemView::SingleSelection);
    linesTable->setAlternatingRowColors(true);
    linesTable->setMinimumHeight(160);
    linesTable->setMaximumHeight(220);
    linesTable->setStyleSheet(
        "QTableWidget {"
        "  border: 1px solid #CBD5E0;"
        "  gridline-color: #EDF2F7;"
        "  background: white;"
        "}"
        "QTableWidget::item { padding: 5px 6px; }"
        "QHeaderView::section {"
        "  background: #2B6CB0;"
        "  color: white;"
        "  font-weight: bold;"
        "  font-size: 11px;"
        "  padding: 7px 4px;"
        "  border: none;"
        "}"
        "QTableWidget::item:selected {"
        "  background: #BEE3F8;"
        "  color: #1A202C;"
        "}"
        "QTableWidget::item:alternate {"
        "  background: #F7FAFC;"
        "}"
    );
    lignesLayout->addWidget(linesTable);

    // ── SAISIE NOUVELLE LIGNE ───────────────────────────────
    QFrame *addFrame = new QFrame;
    addFrame->setStyleSheet(
        "QFrame {"
        "  background: #EBF8FF;"
        "  border: 1px solid #BEE3F8;"
        "  border-radius: 6px;"
        "  padding: 4px;"
        "}"
    );
    QGridLayout *addGrid = new QGridLayout(addFrame);
    addGrid->setSpacing(6);
    addGrid->setContentsMargins(10, 8, 10, 8);

    QLabel *addTitle = new QLabel("➕ Nouvelle ligne:");
    addTitle->setStyleSheet(
        "font-weight:bold;color:#2B6CB0;font-size:11px;"
        "background:transparent;border:none;");
    addGrid->addWidget(addTitle, 0, 0, 1, 4);

    addGrid->addWidget(new QLabel("Désignation:"), 1, 0);
    desigEdit = new QLineEdit;
    desigEdit->setPlaceholderText("Nom de l'article ou service...");
    addGrid->addWidget(desigEdit, 1, 1, 1, 3);

    addGrid->addWidget(new QLabel("Quantité:"), 2, 0);
    qtyEdit = new QSpinBox;
    qtyEdit->setMinimum(1);
    qtyEdit->setMaximum(9999);
    qtyEdit->setValue(1);
    qtyEdit->setFixedWidth(80);
    addGrid->addWidget(qtyEdit, 2, 1);

    addGrid->addWidget(new QLabel("Prix HT:"), 2, 2);
    prixEdit = new QDoubleSpinBox;
    prixEdit->setMaximum(999999);
    prixEdit->setDecimals(2);
    prixEdit->setFixedWidth(110);
    prixEdit->setSuffix(" MAD");
    addGrid->addWidget(prixEdit, 2, 3);

    addGrid->addWidget(new QLabel("TVA %:"), 3, 0);
    tvaEdit = new QDoubleSpinBox;
    tvaEdit->setValue(20.0);
    tvaEdit->setMaximum(100);
    tvaEdit->setFixedWidth(80);
    tvaEdit->setSuffix("%");
    addGrid->addWidget(tvaEdit, 3, 1);

    lignesLayout->addWidget(addFrame);

    // Boutons + Total
    QHBoxLayout *btnLine = new QHBoxLayout;
    addLineBtn = new QPushButton("➕ Ajouter");
    removeLineBtn = new QPushButton("🗑️ Supprimer");
    addLineBtn->setFixedHeight(32);
    removeLineBtn->setFixedHeight(32);
    addLineBtn->setStyleSheet(
        "background:#2B6CB0;color:white;font-weight:bold;"
        "padding:0 16px;border-radius:4px;border:none;");
    removeLineBtn->setStyleSheet(
        "background:#E53E3E;color:white;font-weight:bold;"
        "padding:0 16px;border-radius:4px;border:none;");
    totalLabel = new QLabel("Total TTC: 0.00 MAD");
    totalLabel->setStyleSheet(
        "font-weight:bold;font-size:13px;color:#2B6CB0;");

    btnLine->addWidget(addLineBtn);
    btnLine->addWidget(removeLineBtn);
    btnLine->addStretch();
    btnLine->addWidget(totalLabel);
    lignesLayout->addLayout(btnLine);

    mainLayout->addWidget(lignesGroup);

    // ── BOUTONS SAVE/CANCEL ─────────────────────────────────
    QHBoxLayout *saveBtns = new QHBoxLayout;
    cancelBtn = new QPushButton("Annuler");
    saveBtn = new QPushButton("💾 Enregistrer");
    cancelBtn->setFixedHeight(36);
    saveBtn->setFixedHeight(36);
    cancelBtn->setStyleSheet(
        "padding:0 20px;border-radius:4px;"
        "border:1px solid #CBD5E0;background:white;");
    saveBtn->setStyleSheet(
        "background:#27AE60;color:white;font-weight:bold;"
        "padding:0 20px;border-radius:4px;border:none;");
    saveBtns->addStretch();
    saveBtns->addWidget(cancelBtn);
    saveBtns->addWidget(saveBtn);
    mainLayout->addLayout(saveBtns);

    // Scroll
    scroll->setWidget(container);
    QVBoxLayout *dialogLayout = new QVBoxLayout(this);
    dialogLayout->setContentsMargins(0, 0, 0, 0);
    dialogLayout->addWidget(scroll);

    connect(addLineBtn, &QPushButton::clicked,
            this, &InvoiceEditDialog::onAddLine);
    connect(removeLineBtn, &QPushButton::clicked,
            this, &InvoiceEditDialog::onRemoveLine);
    connect(saveBtn, &QPushButton::clicked,
            this, &InvoiceEditDialog::onSave);
    connect(cancelBtn, &QPushButton::clicked,
            this, &InvoiceEditDialog::onCancel);
            connect(linesTable, &QTableWidget::itemClicked,
        this, [this](QTableWidgetItem *item) {
    int row = item->row();
    if (row >= 0 && row < m_lines.size()) {
        const EditLineItem &line = m_lines[row];
        desigEdit->setText(line.designation);
        qtyEdit->setValue(line.quantity);
        prixEdit->setValue(line.priceHT);
        tvaEdit->setValue(line.taxRate);
        addLineBtn->setText("✏️ Modifier ligne");
    }
});
}
void InvoiceEditDialog::loadData()
{
    QSqlQuery q;
    q.prepare("SELECT numero, type, statut, date_creation, date_echeance, "
              "client_nom, client_adresse, client_tel, client_email "
              "FROM factures WHERE id = ?");
    q.addBindValue(m_invoiceId);

    if (!q.exec() || !q.next()) {
        QMessageBox::critical(this, "Erreur", "Facture introuvable");
        return;
    }

    numeroEdit->setText(q.value(0).toString());
    
    int typeIdx = typeCombo->findText(q.value(1).toString());
    if (typeIdx >= 0) typeCombo->setCurrentIndex(typeIdx);
    
    int statIdx = statusCombo->findText(q.value(2).toString());
    if (statIdx >= 0) statusCombo->setCurrentIndex(statIdx);
    
    dateCreationEdit->setDate(q.value(3).toDate());
    dateEcheanceEdit->setDate(q.value(4).toDate());
    clientNomEdit->setText(q.value(5).toString());
    clientAdresseEdit->setText(q.value(6).toString());
    clientTelEdit->setText(q.value(7).toString());
    clientEmailEdit->setText(q.value(8).toString());
}
void InvoiceEditDialog::loadLines()
{
    m_lines.clear();
    QSqlQuery q;
    q.prepare("SELECT id, designation, quantite, "
              "prix_unitaire_ht, taux_tva "
              "FROM lignes_facture WHERE facture_id = ?");
    q.addBindValue(m_invoiceId);
    
    if (!q.exec()) {
        qDebug() << "Erreur loadLines:" << q.lastError().text();
        return;
    }
    
    while (q.next()) {
        EditLineItem item;
        item.id          = q.value(0).toInt();
        item.designation = q.value(1).toString();
        item.quantity    = q.value(2).toInt();
        item.priceHT     = q.value(3).toDouble();
        item.taxRate     = q.value(4).toDouble();
        m_lines.append(item);
    }
    qDebug() << "Lignes chargées:" << m_lines.size();
    refreshLinesTable();
}
void InvoiceEditDialog::refreshLinesTable()
{
    linesTable->setRowCount(0);
    double totalTTC = 0;

    for (int i = 0; i < m_lines.size(); i++) {
        const EditLineItem &item = m_lines[i];
        linesTable->insertRow(i);

        double lineHT  = item.quantity * item.priceHT;
        double lineTTC = lineHT * (1.0 + item.taxRate / 100.0);
        totalTTC += lineTTC;

        // Désignation
        QTableWidgetItem *d = new QTableWidgetItem(item.designation);
        d->setFlags(d->flags() & ~Qt::ItemIsEditable);
        linesTable->setItem(i, 0, d);

        // Quantité
        QTableWidgetItem *q = new QTableWidgetItem(
            QString::number(item.quantity));
        q->setTextAlignment(Qt::AlignCenter);
        q->setFlags(q->flags() & ~Qt::ItemIsEditable);
        linesTable->setItem(i, 1, q);

        // Prix HT
        QTableWidgetItem *p = new QTableWidgetItem(
            QString::number(item.priceHT, 'f', 2));
        p->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        p->setFlags(p->flags() & ~Qt::ItemIsEditable);
        linesTable->setItem(i, 2, p);

        // TVA
        QTableWidgetItem *t = new QTableWidgetItem(
            QString::number(item.taxRate, 'f', 1) + "%");
        t->setTextAlignment(Qt::AlignCenter);
        t->setFlags(t->flags() & ~Qt::ItemIsEditable);
        linesTable->setItem(i, 3, t);

        // Total HT
        QTableWidgetItem *tot = new QTableWidgetItem(
            QString::number(lineHT, 'f', 2));
        tot->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        tot->setFlags(tot->flags() & ~Qt::ItemIsEditable);
        linesTable->setItem(i, 4, tot);

        linesTable->setRowHeight(i, 30);
    }

    totalLabel->setText(
        QString("Total TTC: %1 MAD").arg(totalTTC, 0, 'f', 2));
}
void InvoiceEditDialog::onAddLine()
{
    if (desigEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "Erreur", "Entrez une désignation");
        return;
    }
    if (prixEdit->value() <= 0) {
        QMessageBox::warning(this, "Erreur", "Entrez un prix valide");
        return;
    }

    int selectedRow = linesTable->currentRow();

    // Si une ligne est sélectionnée → modifier
    if (selectedRow >= 0 && selectedRow < m_lines.size() &&
        desigEdit->text().trimmed() == 
        m_lines[selectedRow].designation) {
        
        m_lines[selectedRow].designation = desigEdit->text().trimmed();
        m_lines[selectedRow].quantity    = qtyEdit->value();
        m_lines[selectedRow].priceHT     = prixEdit->value();
        m_lines[selectedRow].taxRate     = tvaEdit->value();
        
        addLineBtn->setText("➕ Ajouter");
        linesTable->clearSelection();
    } else {
        // Nouvelle ligne
        EditLineItem item;
        item.id          = 0;
        item.designation = desigEdit->text().trimmed();
        item.quantity    = qtyEdit->value();
        item.priceHT     = prixEdit->value();
        item.taxRate     = tvaEdit->value();
        m_lines.append(item);
    }

    desigEdit->clear();
    qtyEdit->setValue(1);
    prixEdit->setValue(0);
    tvaEdit->setValue(20.0);
    desigEdit->setFocus();

    refreshLinesTable();
}
void InvoiceEditDialog::onRemoveLine()
{
    int row = linesTable->currentRow();
    if (row >= 0 && row < m_lines.size())
        m_lines.removeAt(row);
    refreshLinesTable();
}

void InvoiceEditDialog::saveLines()
{
    // Supprimer anciennes lignes
    QSqlQuery del;
    del.prepare("DELETE FROM lignes_facture WHERE facture_id = ?");
    del.addBindValue(m_invoiceId);
    del.exec();

    // Recalculer totaux
    double totalHT=0, totalTVA=0;
    for (const EditLineItem &item : m_lines) {
        double ht = item.quantity * item.priceHT;
        totalHT  += ht;
        totalTVA += ht * item.taxRate / 100.0;

        QSqlQuery ins;
        ins.prepare("INSERT INTO lignes_facture "
                    "(facture_id, article_id, designation, "
                    "quantite, prix_unitaire_ht, taux_tva) "
                    "VALUES (?,?,?,?,?,?)");
        ins.addBindValue(m_invoiceId);
        ins.addBindValue(0);
        ins.addBindValue(item.designation);
        ins.addBindValue(item.quantity);
        ins.addBindValue(item.priceHT);
        ins.addBindValue(item.taxRate);
        ins.exec();
    }

    // Mettre à jour totaux dans factures
    QSqlQuery upd;
    upd.prepare("UPDATE factures SET total_ht=?, "
                "total_tva=?, total_ttc=? WHERE id=?");
    upd.addBindValue(totalHT);
    upd.addBindValue(totalTVA);
    upd.addBindValue(totalHT + totalTVA);
    upd.addBindValue(m_invoiceId);
    upd.exec();
}
void InvoiceEditDialog::onSave()
{
    if (clientNomEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(this,"Erreur",
                             "Le nom du client est obligatoire");
        return;
    }

    QSqlQuery q;
    q.prepare("UPDATE factures SET "
              "numero=?, type=?, statut=?, date_creation=?, "
              "date_echeance=?, client_nom=?, client_adresse=?, "
              "client_tel=?, client_email=? WHERE id=?");
    q.addBindValue(numeroEdit->text().trimmed());
    q.addBindValue(typeCombo->currentText());
    q.addBindValue(statusCombo->currentText());
    q.addBindValue(dateCreationEdit->date().toString("yyyy-MM-dd"));
    q.addBindValue(dateEcheanceEdit->date().toString("yyyy-MM-dd"));
    q.addBindValue(clientNomEdit->text().trimmed());
    q.addBindValue(clientAdresseEdit->text().trimmed());
    q.addBindValue(clientTelEdit->text().trimmed());
    q.addBindValue(clientEmailEdit->text().trimmed());
    q.addBindValue(m_invoiceId);

    if (!q.exec()) {
        QMessageBox::critical(this,"Erreur",q.lastError().text());
        return;
    }

    saveLines();  // ← Sauvegarder les lignes

    QMessageBox::information(this,"Succès",
                             "Facture modifiée avec succès !");
    accept();
}
void InvoiceEditDialog::onCancel() { reject(); }