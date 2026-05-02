#include "invoicecreatedialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
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
#include <QFileInfo>
#include <QPixmap>
#include <QDebug>

InvoiceCreateDialog::InvoiceCreateDialog(int invoiceId, QWidget *parent)
    : QDialog(parent), m_invoiceId(invoiceId), m_isEditMode(invoiceId > 0)
{
    setupUI();
    if (m_isEditMode)
        loadInvoiceLines();
}

void InvoiceCreateDialog::setupUI()
{
    setWindowTitle(m_isEditMode ? "Modifier la Facture" : "Créer une Facture");
    setMinimumSize(900, 700);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // ===== GROUPE INFOS FACTURE =====
    QGroupBox *factureGroup = new QGroupBox("Informations Facture");
    QFormLayout *factureForm = new QFormLayout(factureGroup);

    numeroEdit = new QLineEdit;
    typeCombo = new QComboBox;
    typeCombo->addItems({"Facture", "Devis"});
    dateCreationEdit = new QDateEdit(QDate::currentDate());
    dateCreationEdit->setCalendarPopup(true);
    dateEcheanceEdit = new QDateEdit(QDate::currentDate().addDays(30));
    dateEcheanceEdit->setCalendarPopup(true);
    statusCombo = new QComboBox;
    statusCombo->addItems({"Brouillon", "Envoyée", "Payée", "Annulée"});

    factureForm->addRow("Numéro:", numeroEdit);
    factureForm->addRow("Type:", typeCombo);
    factureForm->addRow("Date création:", dateCreationEdit);
    factureForm->addRow("Date échéance:", dateEcheanceEdit);
    factureForm->addRow("Statut:", statusCombo);
    mainLayout->addWidget(factureGroup);
// ===== GROUPE PERSONNALISATION =====
QGroupBox *persoGroup = new QGroupBox("Personnalisation (Entreprise)");
QFormLayout *persoForm = new QFormLayout(persoGroup);
// Logo
QHBoxLayout *logoLayout = new QHBoxLayout;
logoPathEdit = new QLineEdit;
logoPathEdit->setPlaceholderText("Chemin du logo...");
logoPathEdit->setReadOnly(true);
logoBtn = new QPushButton("📁 Choisir Logo");
logoBtn->setFixedWidth(130);
logoPreview = new QLabel;
logoPreview->setFixedSize(80, 50);
logoPreview->setStyleSheet("border:1px solid #ccc;");
logoPreview->setScaledContents(true);
logoLayout->addWidget(logoPathEdit);
logoLayout->addWidget(logoBtn);
logoLayout->addWidget(logoPreview);
persoForm->addRow("Logo entreprise:", logoLayout);

// Signature
QHBoxLayout *signLayout = new QHBoxLayout;
signaturePathEdit = new QLineEdit;
signaturePathEdit->setPlaceholderText("Chemin de la signature...");
signaturePathEdit->setReadOnly(true);
signatureBtn = new QPushButton("📁 Choisir Signature");
signatureBtn->setFixedWidth(130);
signaturePreview = new QLabel;
signaturePreview->setFixedSize(80, 50);
signaturePreview->setStyleSheet("border:1px solid #ccc;");
signaturePreview->setScaledContents(true);
signLayout->addWidget(signaturePathEdit);
signLayout->addWidget(signatureBtn);
signLayout->addWidget(signaturePreview);
persoForm->addRow("Signature:", signLayout);

mainLayout->addWidget(persoGroup);
    // ===== GROUPE INFOS CLIENT =====
   QGroupBox *clientGroup = new QGroupBox("Informations Client");
QFormLayout *clientForm = new QFormLayout(clientGroup);

// ComboBox avec clients existants + option saisie libre
clientComboBox = new QComboBox;
clientComboBox->setEditable(true);
clientComboBox->addItem("-- Nouveau client --", -1);

// Charger les clients existants
QSqlQuery qc("SELECT id, nom, prenom, adresse, telephone, email "
             "FROM clients WHERE role = 'client' ORDER BY nom");
while (qc.next()) {
    QString display = qc.value(1).toString() + " " + 
                      qc.value(2).toString();
    clientComboBox->addItem(display, qc.value(0).toInt());
}

clientNomEdit = new QLineEdit;
clientNomEdit->setPlaceholderText("Nom complet ou entreprise...");
clientAdresseEdit = new QLineEdit;
clientAdresseEdit->setPlaceholderText("Adresse complète...");
clientTelEdit = new QLineEdit;
clientTelEdit->setPlaceholderText("+212 6XX XXX XXX");
clientEmailEdit = new QLineEdit;
clientEmailEdit->setPlaceholderText("email@exemple.com");

clientForm->addRow("Choisir client:", clientComboBox);
clientForm->addRow("Nom / Entreprise:*", clientNomEdit);
clientForm->addRow("Adresse:", clientAdresseEdit);
clientForm->addRow("Téléphone:", clientTelEdit);
clientForm->addRow("Email:", clientEmailEdit);
mainLayout->addWidget(clientGroup);

// Quand on sélectionne un client existant, remplir les champs
connect(clientComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
    [this](int index) {
        int clientId = clientComboBox->currentData().toInt();
        if (clientId <= 0) {
            clientNomEdit->clear();
            clientAdresseEdit->clear();
            clientTelEdit->clear();
            clientEmailEdit->clear();
            return;
        }
        QSqlQuery q;
        q.prepare("SELECT nom, prenom, adresse, telephone, email "
                  "FROM clients WHERE id = ?");
        q.addBindValue(clientId);
        if (q.exec() && q.next()) {
            clientNomEdit->setText(
                q.value(0).toString() + " " + q.value(1).toString());
            clientAdresseEdit->setText(q.value(2).toString());
            clientTelEdit->setText(q.value(3).toString());
            clientEmailEdit->setText(q.value(4).toString());
        }
    });

    // ===== TABLE LIGNES =====
    QGroupBox *lignesGroup = new QGroupBox("Articles / Services");
    QVBoxLayout *lignesLayout = new QVBoxLayout(lignesGroup);

    linesTable = new QTableWidget(0, 5);
    linesTable->setHorizontalHeaderLabels({"Désignation", "Qté", "Prix HT", "TVA %", "Total HT"});
    linesTable->horizontalHeader()->setStretchLastSection(true);
    linesTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    linesTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    linesTable->setColumnWidth(0, 300);
    linesTable->setColumnWidth(1, 60);
    linesTable->setColumnWidth(2, 100);
    linesTable->setColumnWidth(3, 70);
    lignesLayout->addWidget(linesTable);

    // Saisie ligne
    QHBoxLayout *lineEditLayout = new QHBoxLayout;
    lineEditLayout->addWidget(new QLabel("Désignation:"));
    designationEdit = new QLineEdit;
    designationEdit->setPlaceholderText("Nom article/service...");
    designationEdit->setMinimumWidth(200);
    lineEditLayout->addWidget(designationEdit);

    lineEditLayout->addWidget(new QLabel("Qté:"));
    quantitySpinBox = new QSpinBox;
    quantitySpinBox->setMinimum(1);
    quantitySpinBox->setMaximum(9999);
    quantitySpinBox->setValue(1);
    quantitySpinBox->setFixedWidth(70);
    lineEditLayout->addWidget(quantitySpinBox);

    lineEditLayout->addWidget(new QLabel("Prix HT:"));
    priceHTSpinBox = new QDoubleSpinBox;
    priceHTSpinBox->setMaximum(999999);
    priceHTSpinBox->setDecimals(2);
    priceHTSpinBox->setFixedWidth(100);
    lineEditLayout->addWidget(priceHTSpinBox);

    lineEditLayout->addWidget(new QLabel("TVA%:"));
    taxRateSpinBox = new QDoubleSpinBox;
    taxRateSpinBox->setValue(20.0);
    taxRateSpinBox->setMaximum(100);
    taxRateSpinBox->setFixedWidth(70);
    lineEditLayout->addWidget(taxRateSpinBox);

    lignesLayout->addLayout(lineEditLayout);

    // Boutons lignes
    QHBoxLayout *lineBtnLayout = new QHBoxLayout;
    addLineBtn = new QPushButton("+ Ajouter");
    removeLineBtn = new QPushButton("- Supprimer");
    addLineBtn->setFixedWidth(120);
    removeLineBtn->setFixedWidth(120);
    lineBtnLayout->addWidget(addLineBtn);
    lineBtnLayout->addWidget(removeLineBtn);
    lineBtnLayout->addStretch();
    lignesLayout->addLayout(lineBtnLayout);

    mainLayout->addWidget(lignesGroup);

    // ===== TOTAUX =====
    QHBoxLayout *totalsLayout = new QHBoxLayout;
    totalsLayout->addStretch();
    totalHTLabel = new QLabel("Total HT: 0.00");
    totalTVALabel = new QLabel("TVA: 0.00");
    totalTTCLabel = new QLabel("Total TTC: 0.00");
    totalHTLabel->setStyleSheet("font-weight: bold;");
    totalTVALabel->setStyleSheet("font-weight: bold;");
    totalTTCLabel->setStyleSheet("font-weight: bold; font-size: 14px; color: #2c3e50;");
    totalsLayout->addWidget(totalHTLabel);
    totalsLayout->addSpacing(20);
    totalsLayout->addWidget(totalTVALabel);
    totalsLayout->addSpacing(20);
    totalsLayout->addWidget(totalTTCLabel);
    mainLayout->addLayout(totalsLayout);

    // ===== BOUTONS SAVE/CANCEL =====
    QHBoxLayout *btnLayout = new QHBoxLayout;
    saveBtn = new QPushButton("💾 Enregistrer");
    cancelBtn = new QPushButton("Annuler");
    saveBtn->setFixedHeight(35);
    cancelBtn->setFixedHeight(35);
    saveBtn->setStyleSheet("background-color: #2ecc71; color: white; font-weight: bold;");
    btnLayout->addStretch();
    btnLayout->addWidget(cancelBtn);
    btnLayout->addWidget(saveBtn);
    mainLayout->addLayout(btnLayout);

    // Connexions
    connect(addLineBtn, &QPushButton::clicked, this, &InvoiceCreateDialog::onAddLine);
    connect(removeLineBtn, &QPushButton::clicked, this, &InvoiceCreateDialog::onRemoveLine);
    connect(saveBtn, &QPushButton::clicked, this, &InvoiceCreateDialog::onSave);
    connect(cancelBtn, &QPushButton::clicked, this, &InvoiceCreateDialog::onCancel);
    // Connexions boutons
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
}


void InvoiceCreateDialog::loadClients() {}
void InvoiceCreateDialog::loadArticles() {}
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

void InvoiceCreateDialog::calculateTotals() { refreshLineTable(); }

void InvoiceCreateDialog::onAddLine()
{
    if (designationEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "Erreur", "Entrez une désignation");
        return;
    }
    if (priceHTSpinBox->value() <= 0) {
        QMessageBox::warning(this, "Erreur", "Entrez un prix valide");
        return;
    }

    InvoiceLineItem item;
    item.articleId = 0;
    item.designation = designationEdit->text().trimmed();
    item.quantity = quantitySpinBox->value();
    item.priceHT = priceHTSpinBox->value();
    item.taxRate = taxRateSpinBox->value();
    m_lineItems.append(item);

    designationEdit->clear();
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
void InvoiceCreateDialog::onArticleSelected(int index) { Q_UNUSED(index) }
void InvoiceCreateDialog::onLineDataChanged() {}

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

    QSqlQuery q;
    q.prepare("INSERT INTO factures "
              "(numero, type, client_id, client_nom, client_adresse, client_tel, client_email, "
              "date_creation, date_echeance, statut, total_ht, total_tva, total_ttc) "
              "VALUES (?,?,?,?,?,?,?,?,?,?,?,?,?)");
    q.addBindValue(numeroEdit->text().trimmed());
    q.addBindValue(typeCombo->currentText());
    q.addBindValue(1); // client_id admin par défaut
    q.addBindValue(clientNomEdit->text().trimmed());
    q.addBindValue(clientAdresseEdit->text().trimmed());
    q.addBindValue(clientTelEdit->text().trimmed());
    q.addBindValue(clientEmailEdit->text().trimmed());
    q.addBindValue(dateCreationEdit->date().toString("yyyy-MM-dd"));
    q.addBindValue(dateEcheanceEdit->date().toString("yyyy-MM-dd"));
    q.addBindValue(statusCombo->currentText());
    q.addBindValue(totalHT);
    q.addBindValue(totalTVA);
    q.addBindValue(totalHT + totalTVA);

    if (!q.exec()) {
        QMessageBox::critical(this, "Erreur", q.lastError().text());
        return;
    }

    m_invoiceId = q.lastInsertId().toInt();

    for (const InvoiceLineItem &item : m_lineItems) {
        QSqlQuery lq;
        lq.prepare("INSERT INTO lignes_facture "
                   "(facture_id, article_id, designation, quantite, prix_unitaire_ht, taux_tva) "
                   "VALUES (?,?,?,?,?,?)");
        lq.addBindValue(m_invoiceId);
        lq.addBindValue(item.articleId);
        lq.addBindValue(item.designation);
        lq.addBindValue(item.quantity);
        lq.addBindValue(item.priceHT);
        lq.addBindValue(item.taxRate);
        lq.exec();
    }
   m_logoPath = logoPathEdit->text();
m_signaturePath = signaturePathEdit->text();
    QMessageBox::information(this, "Succès", "Facture enregistrée avec succès !");
    accept();
}

void InvoiceCreateDialog::onCancel() { reject(); }