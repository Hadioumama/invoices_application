#include "invoicedialog.h"
#include "invoiceitemdialog.h"
#include "database/database.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QDateEdit>
#include <QComboBox>
#include <QTableWidget>
#include <QPushButton>
#include <QMessageBox>
#include <QHeaderView>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

InvoiceDialog::InvoiceDialog(QWidget *parent)
    : QDialog(parent), m_isEditMode(false), m_invoiceId(-1)
{
    setupUI();
}

void InvoiceDialog::setupUI()
{
    setWindowTitle("Créer/Modifier une Facture");
    setGeometry(100, 100, 800, 600);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // ===== Details section =====
    QVBoxLayout *detailsLayout = new QVBoxLayout;

    // Numero
    QHBoxLayout *numeroLayout = new QHBoxLayout;
    numeroLayout->addWidget(new QLabel("Numéro:"));
    numeroEdit = new QLineEdit;
    numeroEdit->setReadOnly(true);
    numeroLayout->addWidget(numeroEdit);
    detailsLayout->addLayout(numeroLayout);

    // Type
    QHBoxLayout *typeLayout = new QHBoxLayout;
    typeLayout->addWidget(new QLabel("Type:"));
    typeCombo = new QComboBox;
    typeCombo->addItem("Facture");
    typeCombo->addItem("Devis");
    typeLayout->addWidget(typeCombo);
    detailsLayout->addLayout(typeLayout);

    // Client
    QHBoxLayout *clientLayout = new QHBoxLayout;
    clientLayout->addWidget(new QLabel("Client:"));
    clientCombo = new QComboBox;
    clientLayout->addWidget(clientCombo);
    detailsLayout->addLayout(clientLayout);
    connect(clientCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &InvoiceDialog::onClientSelected);

    // Dates
    QHBoxLayout *datesLayout = new QHBoxLayout;
    datesLayout->addWidget(new QLabel("Date Création:"));
    dateCreationEdit = new QDateEdit;
    dateCreationEdit->setDate(QDate::currentDate());
    datesLayout->addWidget(dateCreationEdit);
    datesLayout->addWidget(new QLabel("Échéance:"));
    dateEcheanceEdit = new QDateEdit;
    dateEcheanceEdit->setDate(QDate::currentDate().addDays(30));
    datesLayout->addWidget(dateEcheanceEdit);
    datesLayout->addWidget(new QLabel("Validité:"));
    dateValiditeEdit = new QDateEdit;
    dateValiditeEdit->setDate(QDate::currentDate().addDays(60));
    datesLayout->addWidget(dateValiditeEdit);
    detailsLayout->addLayout(datesLayout);

    // Status
    QHBoxLayout *statutLayout = new QHBoxLayout;
    statutLayout->addWidget(new QLabel("Statut:"));
    statutCombo = new QComboBox;
    statutCombo->addItem("Brouillon");
    statutCombo->addItem("Validée");
    statutCombo->addItem("Payée");
    statutCombo->addItem("Annulée");
    statutLayout->addWidget(statutCombo);
    mainLayout->addLayout(statutLayout);

    mainLayout->addLayout(detailsLayout);

    // ===== Items section =====
    mainLayout->addWidget(new QLabel("Articles:"));
    
    itemsTable = new QTableWidget;
    itemsTable->setColumnCount(5);
    itemsTable->setHorizontalHeaderLabels({"Article", "Quantité", "Prix HT", "Taux TVA", "Total HT"});
    itemsTable->horizontalHeader()->setStretchLastSection(true);
    itemsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    itemsTable->setSelectionMode(QAbstractItemView::SingleSelection);
    mainLayout->addWidget(itemsTable);

    // Item buttons
    QHBoxLayout *itemBtnLayout = new QHBoxLayout;
    addItemBtn = new QPushButton("Ajouter Article");
    removeItemBtn = new QPushButton("Retirer Article");
    itemBtnLayout->addWidget(addItemBtn);
    itemBtnLayout->addWidget(removeItemBtn);
    mainLayout->addLayout(itemBtnLayout);

    connect(addItemBtn, &QPushButton::clicked, this, &InvoiceDialog::onAddItem);
    connect(removeItemBtn, &QPushButton::clicked, this, &InvoiceDialog::onRemoveItem);

    // ===== Totals section =====
    QHBoxLayout *totalsLayout = new QHBoxLayout;
    totalsLayout->addWidget(new QLabel("Total HT:"));
    totalHTLabel = new QLabel("0.00 €");
    totalHTLabel->setStyleSheet("font-weight: bold;");
    totalsLayout->addWidget(totalHTLabel);
    
    totalsLayout->addWidget(new QLabel("Total TVA:"));
    totalTVALabel = new QLabel("0.00 €");
    totalTVALabel->setStyleSheet("font-weight: bold;");
    totalsLayout->addWidget(totalTVALabel);
    
    totalsLayout->addWidget(new QLabel("Total TTC:"));
    totalTTCLabel = new QLabel("0.00 €");
    totalTTCLabel->setStyleSheet("font-weight: bold; color: green;");
    totalsLayout->addWidget(totalTTCLabel);
    
    totalsLayout->addStretch();
    mainLayout->addLayout(totalsLayout);

    // ===== Action buttons =====
    QHBoxLayout *buttonLayout = new QHBoxLayout;
    saveBtn = new QPushButton("Enregistrer");
    cancelBtn = new QPushButton("Annuler");
    buttonLayout->addStretch();
    buttonLayout->addWidget(saveBtn);
    buttonLayout->addWidget(cancelBtn);
    mainLayout->addLayout(buttonLayout);

    connect(saveBtn, &QPushButton::clicked, this, &InvoiceDialog::onSave);
    connect(cancelBtn, &QPushButton::clicked, this, &InvoiceDialog::reject);

    loadClients();
}

void InvoiceDialog::loadClients()
{
    QSqlQuery query;
    query.exec("SELECT id, nom, prenom FROM clients WHERE role != 'admin' ORDER BY nom");

    clientCombo->clear();
    while (query.next()) {
        int id = query.value(0).toInt();
        QString nom = query.value(1).toString();
        QString prenom = query.value(2).toString();
        clientCombo->addItem(nom + " " + prenom, id);
    }
}

void InvoiceDialog::setCreateMode()
{
    m_isEditMode = false;
    m_invoice = Invoice();
    m_invoice.type = "Facture";
    m_invoice.statut = "Brouillon";
    m_invoice.dateCreation = QDate::currentDate();
    m_invoice.dateEcheance = QDate::currentDate().addDays(30);
    m_invoice.dateValidite = QDate::currentDate().addDays(60);

    // Generate invoice number
    QSqlQuery query;
    query.exec("SELECT COUNT(*) FROM factures");
    if (query.next()) {
        int count = query.value(0).toInt() + 1;
        m_invoice.numero = QString("FAC-%1-%2").arg(QDate::currentDate().year()).arg(count, 5, 10, QChar('0'));
    }

    numeroEdit->setText(m_invoice.numero);
    typeCombo->setCurrentText("Facture");
    statutCombo->setCurrentText("Brouillon");
}

void InvoiceDialog::setEditMode(int invoiceId)
{
    m_isEditMode = true;
    m_invoiceId = invoiceId;
    loadInvoiceData();
}

void InvoiceDialog::loadInvoiceData()
{
    QSqlQuery query;
    query.prepare("SELECT numero, type, client_id, date_creation, date_echeance, date_validite, "
                  "total_ht, total_tva, total_ttc, statut FROM factures WHERE id = ?");
    query.addBindValue(m_invoiceId);

    if (!query.exec() || !query.next()) {
        QMessageBox::critical(this, "Erreur", "Impossible de charger la facture");
        return;
    }

    m_invoice.id = m_invoiceId;
    m_invoice.numero = query.value(0).toString();
    m_invoice.type = query.value(1).toString();
    m_invoice.clientId = query.value(2).toInt();
    m_invoice.dateCreation = query.value(3).toDate();
    m_invoice.dateEcheance = query.value(4).toDate();
    m_invoice.dateValidite = query.value(5).toDate();
    m_invoice.totalHT = query.value(6).toDouble();
    m_invoice.totalTVA = query.value(7).toDouble();
    m_invoice.totalTTC = query.value(8).toDouble();
    m_invoice.statut = query.value(9).toString();

    numeroEdit->setText(m_invoice.numero);
    typeCombo->setCurrentText(m_invoice.type);
    int clientIndex = clientCombo->findData(m_invoice.clientId);
    if (clientIndex >= 0) clientCombo->setCurrentIndex(clientIndex);
    dateCreationEdit->setDate(m_invoice.dateCreation);
    dateEcheanceEdit->setDate(m_invoice.dateEcheance);
    dateValiditeEdit->setDate(m_invoice.dateValidite);
    statutCombo->setCurrentText(m_invoice.statut);

    loadInvoiceItems();
}

void InvoiceDialog::loadInvoiceItems()
{
    itemsTable->setRowCount(0);

    QSqlQuery query;
    query.prepare("SELECT a.designation, lf.quantite, lf.prix_unitaire_ht, lf.taux_tva, "
                  "(lf.quantite * lf.prix_unitaire_ht) as total FROM lignes_facture lf "
                  "JOIN articles a ON lf.article_id = a.id WHERE lf.facture_id = ?");
    query.addBindValue(m_invoiceId);

    if (!query.exec()) {
        qDebug() << "Erreur chargement articles:" << query.lastError().text();
        return;
    }

    int row = 0;
    while (query.next()) {
        itemsTable->insertRow(row);
        itemsTable->setItem(row, 0, new QTableWidgetItem(query.value(0).toString()));
        itemsTable->setItem(row, 1, new QTableWidgetItem(QString::number(query.value(1).toInt())));
        itemsTable->setItem(row, 2, new QTableWidgetItem(QString::number(query.value(2).toDouble(), 'f', 2)));
        itemsTable->setItem(row, 3, new QTableWidgetItem(QString::number(query.value(3).toDouble(), 'f', 2)));
        itemsTable->setItem(row, 4, new QTableWidgetItem(QString::number(query.value(4).toDouble(), 'f', 2)));
        row++;
    }

    updateTotals();
}

void InvoiceDialog::updateTotals()
{
    totalHTLabel->setText(QString::number(m_invoice.totalHT, 'f', 2) + " €");
    totalTVALabel->setText(QString::number(m_invoice.totalTVA, 'f', 2) + " €");
    totalTTCLabel->setText(QString::number(m_invoice.totalTTC, 'f', 2) + " €");
}

void InvoiceDialog::onAddItem()
{
    InvoiceItemDialog itemDlg(this);
    if (itemDlg.exec() == QDialog::Accepted) {
        int row = itemsTable->rowCount();
        itemsTable->insertRow(row);

        // Get article details
        QSqlQuery query;
        query.prepare("SELECT designation FROM articles WHERE id = ?");
        query.addBindValue(itemDlg.getArticleId());
        
        if (query.exec() && query.next()) {
            itemsTable->setItem(row, 0, new QTableWidgetItem(query.value(0).toString()));
            itemsTable->setItem(row, 1, new QTableWidgetItem(QString::number(itemDlg.getQuantity())));
            itemsTable->setItem(row, 2, new QTableWidgetItem(QString::number(itemDlg.getPrixUnitaire(), 'f', 2)));
            itemsTable->setItem(row, 3, new QTableWidgetItem(QString::number(itemDlg.getTauxTVA(), 'f', 2)));
            
            double total = itemDlg.getQuantity() * itemDlg.getPrixUnitaire();
            itemsTable->setItem(row, 4, new QTableWidgetItem(QString::number(total, 'f', 2)));
        }
        calculateTotals();
    }
}

void InvoiceDialog::onRemoveItem()
{
    int row = itemsTable->currentRow();
    if (row >= 0) {
        itemsTable->removeRow(row);
        calculateTotals();
    }
}

void InvoiceDialog::calculateTotals()
{
    double totalHT = 0.0;
    double totalTVA = 0.0;

    for (int row = 0; row < itemsTable->rowCount(); ++row) {
        double priceHT = itemsTable->item(row, 2)->text().toDouble();
        int quantity = itemsTable->item(row, 1)->text().toInt();
        double tauxTVA = itemsTable->item(row, 3)->text().toDouble();

        double rowTotalHT = priceHT * quantity;
        double rowTVA = rowTotalHT * (tauxTVA / 100.0);

        totalHT += rowTotalHT;
        totalTVA += rowTVA;
    }

    m_invoice.totalHT = totalHT;
    m_invoice.totalTVA = totalTVA;
    m_invoice.totalTTC = totalHT + totalTVA;

    updateTotals();
}

void InvoiceDialog::onClientSelected()
{
    m_invoice.clientId = clientCombo->currentData().toInt();
}

void InvoiceDialog::onSave()
{
    if (m_invoice.clientId <= 0) {
        QMessageBox::warning(this, "Erreur", "Veuillez sélectionner un client");
        return;
    }

    if (itemsTable->rowCount() == 0) {
        QMessageBox::warning(this, "Erreur", "Veuillez ajouter au moins un article");
        return;
    }

    m_invoice.type = typeCombo->currentText();
    m_invoice.dateCreation = dateCreationEdit->date();
    m_invoice.dateEcheance = dateEcheanceEdit->date();
    m_invoice.dateValidite = dateValiditeEdit->date();
    m_invoice.statut = statutCombo->currentText();

    if (m_isEditMode) {
        // Update existing invoice
        QSqlQuery query;
        query.prepare("UPDATE factures SET type=?, date_creation=?, date_echeance=?, "
                      "date_validite=?, total_ht=?, total_tva=?, total_ttc=?, statut=? WHERE id=?");
        query.addBindValue(m_invoice.type);
        query.addBindValue(m_invoice.dateCreation);
        query.addBindValue(m_invoice.dateEcheance);
        query.addBindValue(m_invoice.dateValidite);
        query.addBindValue(m_invoice.totalHT);
        query.addBindValue(m_invoice.totalTVA);
        query.addBindValue(m_invoice.totalTTC);
        query.addBindValue(m_invoice.statut);
        query.addBindValue(m_invoice.id);

        if (!query.exec()) {
            QMessageBox::critical(this, "Erreur", "Impossible de mettre à jour la facture: " + query.lastError().text());
            return;
        }

        // Delete old items
        query.prepare("DELETE FROM lignes_facture WHERE facture_id = ?");
        query.addBindValue(m_invoice.id);
        query.exec();

    } else {
        // Create new invoice
        QSqlQuery query;
        query.prepare("INSERT INTO factures (numero, type, client_id, date_creation, date_echeance, "
                      "date_validite, total_ht, total_tva, total_ttc, statut) "
                      "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");
        query.addBindValue(m_invoice.numero);
        query.addBindValue(m_invoice.type);
        query.addBindValue(m_invoice.clientId);
        query.addBindValue(m_invoice.dateCreation);
        query.addBindValue(m_invoice.dateEcheance);
        query.addBindValue(m_invoice.dateValidite);
        query.addBindValue(m_invoice.totalHT);
        query.addBindValue(m_invoice.totalTVA);
        query.addBindValue(m_invoice.totalTTC);
        query.addBindValue(m_invoice.statut);

        if (!query.exec()) {
            QMessageBox::critical(this, "Erreur", "Impossible de créer la facture: " + query.lastError().text());
            return;
        }

        m_invoice.id = query.lastInsertId().toInt();
    }

    // Insert invoice items
    for (int row = 0; row < itemsTable->rowCount(); ++row) {
        // Get article ID from displayed text
        QString articleName = itemsTable->item(row, 0)->text();
        QSqlQuery artQuery;
        artQuery.prepare("SELECT id FROM articles WHERE designation = ?");
        artQuery.addBindValue(articleName);

        int articleId = -1;
        if (artQuery.exec() && artQuery.next()) {
            articleId = artQuery.value(0).toInt();
        }

        double priceHT = itemsTable->item(row, 2)->text().toDouble();
        int quantity = itemsTable->item(row, 1)->text().toInt();
        double tauxTVA = itemsTable->item(row, 3)->text().toDouble();

        QSqlQuery itemQuery;
        itemQuery.prepare("INSERT INTO lignes_facture (facture_id, article_id, quantite, prix_unitaire_ht, taux_tva) "
                         "VALUES (?, ?, ?, ?, ?)");
        itemQuery.addBindValue(m_invoice.id);
        itemQuery.addBindValue(articleId);
        itemQuery.addBindValue(quantity);
        itemQuery.addBindValue(priceHT);
        itemQuery.addBindValue(tauxTVA);

        if (!itemQuery.exec()) {
            qDebug() << "Erreur insertion article:" << itemQuery.lastError().text();
        }
    }

    QMessageBox::information(this, "Succès", "Facture enregistrée avec succès");
    accept();
}

void InvoiceDialog::onCancel()
{
    reject();
}