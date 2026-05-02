#include "invoiceeditdialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
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
    setMinimumSize(600, 500);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // Groupe facture
    QGroupBox *factureGroup = new QGroupBox("Informations Facture");
    QFormLayout *factureForm = new QFormLayout(factureGroup);

    numeroEdit = new QLineEdit;
    typeCombo = new QComboBox;
    typeCombo->addItems({"Facture", "Devis"});
    statusCombo = new QComboBox;
    statusCombo->addItems({"Brouillon", "Envoyée", "Payée", "Annulée"});
    dateCreationEdit = new QDateEdit;
    dateCreationEdit->setCalendarPopup(true);
    dateEcheanceEdit = new QDateEdit;
    dateEcheanceEdit->setCalendarPopup(true);

    factureForm->addRow("Numéro:", numeroEdit);
    factureForm->addRow("Type:", typeCombo);
    factureForm->addRow("Statut:", statusCombo);
    factureForm->addRow("Date création:", dateCreationEdit);
    factureForm->addRow("Date échéance:", dateEcheanceEdit);
    mainLayout->addWidget(factureGroup);

    // Groupe client
    QGroupBox *clientGroup = new QGroupBox("Informations Client");
    QFormLayout *clientForm = new QFormLayout(clientGroup);

    clientNomEdit = new QLineEdit;
    clientAdresseEdit = new QLineEdit;
    clientTelEdit = new QLineEdit;
    clientEmailEdit = new QLineEdit;

    clientForm->addRow("Nom / Entreprise:", clientNomEdit);
    clientForm->addRow("Adresse:", clientAdresseEdit);
    clientForm->addRow("Téléphone:", clientTelEdit);
    clientForm->addRow("Email:", clientEmailEdit);
    mainLayout->addWidget(clientGroup);
    // Groupe lignes
QGroupBox *lignesGroup = new QGroupBox("Articles de la facture");
QVBoxLayout *lignesLayout = new QVBoxLayout(lignesGroup);

linesTable = new QTableWidget(0, 5);
linesTable->setHorizontalHeaderLabels(
    {"Désignation","Qté","Prix HT","TVA%","Total HT"});
linesTable->horizontalHeader()->setStretchLastSection(true);
linesTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
linesTable->setSelectionBehavior(QAbstractItemView::SelectRows);
linesTable->setColumnWidth(0, 200);
lignesLayout->addWidget(linesTable);

// Saisie ligne
QHBoxLayout *lineRow = new QHBoxLayout;
desigEdit = new QLineEdit;
desigEdit->setPlaceholderText("Désignation...");
qtyEdit = new QSpinBox;
qtyEdit->setMinimum(1); qtyEdit->setMaximum(9999);
prixEdit = new QDoubleSpinBox;
prixEdit->setMaximum(999999); prixEdit->setDecimals(2);
tvaEdit = new QDoubleSpinBox;
tvaEdit->setValue(20); tvaEdit->setMaximum(100);
lineRow->addWidget(new QLabel("Désig:"));
lineRow->addWidget(desigEdit);
lineRow->addWidget(new QLabel("Qté:"));
lineRow->addWidget(qtyEdit);
lineRow->addWidget(new QLabel("Prix:"));
lineRow->addWidget(prixEdit);
lineRow->addWidget(new QLabel("TVA%:"));
lineRow->addWidget(tvaEdit);
lignesLayout->addLayout(lineRow);

QHBoxLayout *lineBtns = new QHBoxLayout;
addLineBtn = new QPushButton("+ Ajouter");
removeLineBtn = new QPushButton("- Supprimer");
totalLabel = new QLabel("Total TTC: 0.00");
totalLabel->setStyleSheet("font-weight:bold;font-size:13px;color:#1565C0;");
lineBtns->addWidget(addLineBtn);
lineBtns->addWidget(removeLineBtn);
lineBtns->addStretch();
lineBtns->addWidget(totalLabel);
lignesLayout->addLayout(lineBtns);

mainLayout->addWidget(lignesGroup);

connect(addLineBtn,    &QPushButton::clicked,
        this, &InvoiceEditDialog::onAddLine);
connect(removeLineBtn, &QPushButton::clicked,
        this, &InvoiceEditDialog::onRemoveLine);

    // Boutons
    QHBoxLayout *btnLayout = new QHBoxLayout;
    saveBtn = new QPushButton("💾 Enregistrer");
    cancelBtn = new QPushButton("Annuler");
    saveBtn->setStyleSheet("background-color: #2ecc71; color: white; font-weight: bold;");
    saveBtn->setFixedHeight(35);
    btnLayout->addStretch();
    btnLayout->addWidget(cancelBtn);
    btnLayout->addWidget(saveBtn);
    mainLayout->addLayout(btnLayout);

    connect(saveBtn, &QPushButton::clicked, this, &InvoiceEditDialog::onSave);
    connect(cancelBtn, &QPushButton::clicked, this, &InvoiceEditDialog::onCancel);
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
    double ttc = 0;
    for (const EditLineItem &item : m_lines) {
        int row = linesTable->rowCount();
        linesTable->insertRow(row);
        double lineHT = item.quantity * item.priceHT;
        ttc += lineHT * (1 + item.taxRate/100.0);
        linesTable->setItem(row,0,
            new QTableWidgetItem(item.designation));
        linesTable->setItem(row,1,
            new QTableWidgetItem(QString::number(item.quantity)));
        linesTable->setItem(row,2,
            new QTableWidgetItem(QString::number(item.priceHT,'f',2)));
        linesTable->setItem(row,3,
            new QTableWidgetItem(QString::number(item.taxRate,'f',1)+"%"));
        linesTable->setItem(row,4,
            new QTableWidgetItem(QString::number(lineHT,'f',2)));
    }
    totalLabel->setText(QString("Total TTC: %1 MAD")
                        .arg(ttc, 0,'f',2));
}

void InvoiceEditDialog::onAddLine()
{
    if (desigEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(this,"Erreur","Entrez une désignation");
        return;
    }
    EditLineItem item;
    item.designation = desigEdit->text().trimmed();
    item.quantity    = qtyEdit->value();
    item.priceHT     = prixEdit->value();
    item.taxRate     = tvaEdit->value();
    m_lines.append(item);
    desigEdit->clear();
    qtyEdit->setValue(1);
    prixEdit->setValue(0);
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