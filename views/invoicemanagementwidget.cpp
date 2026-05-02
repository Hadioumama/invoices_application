#include "invoicemanagementwidget.h"
#include "dialogs/invoicedialog.h"
#include "database/database.h"
#include <QTableView>
#include <QHeaderView>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QSqlQuery>
#include <QSqlError>
#include <QLineEdit>
#include <QDateEdit>
#include <QComboBox>
#include <QLabel>
#include <QDebug>

InvoiceManagementWidget::InvoiceManagementWidget(QWidget *parent)
    : QWidget(parent)
{
    setupUI();
    loadInvoices();
}

InvoiceManagementWidget::~InvoiceManagementWidget()
{
}

void InvoiceManagementWidget::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // ===== Search section =====
    QHBoxLayout *searchLayout = new QHBoxLayout;
    searchLayout->addWidget(new QLabel("Rechercher par Client:"));
    clientIdEdit = new QLineEdit;
    clientIdEdit->setPlaceholderText("ID Client...");
    clientIdEdit->setMaximumWidth(100);
    searchLayout->addWidget(clientIdEdit);
    searchClientBtn = new QPushButton("Rechercher");
    searchLayout->addWidget(searchClientBtn);

    searchLayout->addWidget(new QLabel("Par Date:"));
    dateEdit = new QDateEdit;
    dateEdit->setDate(QDate::currentDate());
    searchLayout->addWidget(dateEdit);
    searchDateBtn = new QPushButton("Rechercher");
    searchLayout->addWidget(searchDateBtn);

    searchLayout->addWidget(new QLabel("Statut:"));
    statusCombo = new QComboBox;
    statusCombo->addItem("Tous");
    statusCombo->addItem("Brouillon");
    statusCombo->addItem("Validée");
    statusCombo->addItem("Payée");
    statusCombo->addItem("Annulée");
    searchLayout->addWidget(statusCombo);

    mainLayout->addLayout(searchLayout);

    // ===== Table view =====
    invoiceModel = new QSqlTableModel(this);
    invoiceModel->setTable("factures");
    invoiceModel->setEditStrategy(QSqlTableModel::OnManualSubmit);
    invoiceModel->select();

    invoiceView = new QTableView;
    invoiceView->setModel(invoiceModel);
    invoiceView->setSelectionBehavior(QAbstractItemView::SelectRows);
    invoiceView->setSelectionMode(QAbstractItemView::SingleSelection);
    invoiceView->horizontalHeader()->setStretchLastSection(true);
    mainLayout->addWidget(invoiceView);

    // ===== Action buttons =====
    QHBoxLayout *buttonLayout = new QHBoxLayout;
    createBtn = new QPushButton("Créer Facture");
    editBtn = new QPushButton("Modifier");
    deleteBtn = new QPushButton("Supprimer");
    changeStatusBtn = new QPushButton("Changer Statut");
    refreshBtn = new QPushButton("Actualiser");
    buttonLayout->addWidget(createBtn);
    buttonLayout->addWidget(editBtn);
    buttonLayout->addWidget(deleteBtn);
    buttonLayout->addWidget(changeStatusBtn);
    buttonLayout->addWidget(refreshBtn);
    mainLayout->addLayout(buttonLayout);

    // Connect signals
    connect(createBtn, &QPushButton::clicked, this, &InvoiceManagementWidget::onCreateInvoice);
    connect(editBtn, &QPushButton::clicked, this, &InvoiceManagementWidget::onEditInvoice);
    connect(deleteBtn, &QPushButton::clicked, this, &InvoiceManagementWidget::onDeleteInvoice);
    connect(changeStatusBtn, &QPushButton::clicked, this, &InvoiceManagementWidget::onChangeStatus);
    connect(refreshBtn, &QPushButton::clicked, this, &InvoiceManagementWidget::onRefresh);
    connect(searchClientBtn, &QPushButton::clicked, this, &InvoiceManagementWidget::onSearchByClient);
    connect(searchDateBtn, &QPushButton::clicked, this, &InvoiceManagementWidget::onSearchByDate);
}

void InvoiceManagementWidget::loadInvoices()
{
    invoiceModel->select();
}

void InvoiceManagementWidget::onCreateInvoice()
{
    InvoiceDialog dlg(this);
    dlg.setCreateMode();
    if (dlg.exec() == QDialog::Accepted) {
        loadInvoices();
    }
}

void InvoiceManagementWidget::onEditInvoice()
{
    int row = invoiceView->currentIndex().row();
    if (row < 0) {
        QMessageBox::warning(this, "Sélection", "Veuillez sélectionner une facture à modifier.");
        return;
    }

    int invoiceId = invoiceModel->data(invoiceModel->index(row, 0)).toInt();

    InvoiceDialog dlg(this);
    dlg.setEditMode(invoiceId);
    if (dlg.exec() == QDialog::Accepted) {
        loadInvoices();
    }
}

void InvoiceManagementWidget::onDeleteInvoice()
{
    int row = invoiceView->currentIndex().row();
    if (row < 0) {
        QMessageBox::warning(this, "Sélection", "Veuillez sélectionner une facture à supprimer.");
        return;
    }

    int invoiceId = invoiceModel->data(invoiceModel->index(row, 0)).toInt();

    QMessageBox::StandardButton reply = QMessageBox::question(this, "Confirmation",
        "Supprimer définitivement cette facture ?", QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        QSqlQuery query;
        
        // Delete associated items first
        query.prepare("DELETE FROM lignes_facture WHERE facture_id = ?");
        query.addBindValue(invoiceId);
        query.exec();

        // Then delete invoice
        query.prepare("DELETE FROM factures WHERE id = ?");
        query.addBindValue(invoiceId);

        if (query.exec()) {
            loadInvoices();
            QMessageBox::information(this, "Succès", "Facture supprimée");
        } else {
            QMessageBox::critical(this, "Erreur", "Échec suppression: " + query.lastError().text());
        }
    }
}

void InvoiceManagementWidget::onSearchByClient()
{
    bool ok;
    int clientId = clientIdEdit->text().toInt(&ok);
    if (!ok || clientId <= 0) {
        QMessageBox::warning(this, "Erreur", "ID Client invalide");
        return;
    }

    invoiceModel->setFilter(QString("client_id = %1").arg(clientId));
    invoiceModel->select();
}

void InvoiceManagementWidget::onSearchByDate()
{
    QString date = dateEdit->date().toString("yyyy-MM-dd");
    invoiceModel->setFilter(QString("DATE(date_creation) = '%1'").arg(date));
    invoiceModel->select();
}

void InvoiceManagementWidget::onRefresh()
{
    invoiceModel->setFilter("");
    invoiceModel->select();
    clientIdEdit->clear();
    statusCombo->setCurrentIndex(0);
}

void InvoiceManagementWidget::onChangeStatus()
{
    int row = invoiceView->currentIndex().row();
    if (row < 0) {
        QMessageBox::warning(this, "Sélection", "Veuillez sélectionner une facture.");
        return;
    }

    int invoiceId = invoiceModel->data(invoiceModel->index(row, 0)).toInt();

    QComboBox combo;
    combo.addItem("Brouillon");
    combo.addItem("Validée");
    combo.addItem("Payée");
    combo.addItem("Annulée");

 
    QString newStatus = combo.currentText();
    // À améliorer avec un dialog personnalisé

    QSqlQuery query;
    query.prepare("UPDATE factures SET statut = ? WHERE id = ?");
    query.addBindValue(statusCombo->currentText() != "Tous" ? statusCombo->currentText() : "Validée");
    query.addBindValue(invoiceId);

    if (query.exec()) {
        loadInvoices();
    }
}