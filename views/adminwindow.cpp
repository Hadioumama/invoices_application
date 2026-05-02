#include "views/adminwindow.h"
#include "invoicemanagementwidget.h"
#include "dialogs/invoiceeditdialog.h"
#include "dialogs/client_edit_dialog.h"
#include <QTableView>
#include <QHeaderView>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QSqlQuery>
#include <QSqlQueryModel>
#include <QSqlError>
#include <QLineEdit>
#include <QLabel>
#include <QFileDialog>
#include "utils/invoicegenerator.h"
#include "dialogs/invoicecreatedialog.h"
#include "dialogs/invoiceactiondialog.h"
AdminWindow::AdminWindow(QWidget *parent) : QMainWindow(parent)
{
    setupUI();
    refreshModel();
}

AdminWindow::~AdminWindow() {}

void AdminWindow::setupUI()
{
    setWindowTitle("Panneau Administrateur");
    setGeometry(100, 100, 1200, 700);

    QWidget *central = new QWidget(this);
    setCentralWidget(central);
    QVBoxLayout *mainLayout = new QVBoxLayout(central);

    // ===== CRÉER LE TAB WIDGET =====
    QTabWidget *tabWidget = new QTabWidget(this);
    mainLayout->addWidget(tabWidget);

    // ===== TAB 1: GESTION CLIENTS =====
    QWidget *clientTab = new QWidget();
    QVBoxLayout *clientTabLayout = new QVBoxLayout(clientTab);
 QWidget *invoiceTab = new QWidget();
    QVBoxLayout *invoiceTabLayout = new QVBoxLayout(invoiceTab);

    // Search invoices
    QHBoxLayout *invoiceSearchLayout = new QHBoxLayout;
    invoiceSearchLayout->addWidget(new QLabel("Rechercher:"));
    invoiceSearchEdit = new QLineEdit;
    invoiceSearchEdit->setPlaceholderText("N° facture, client...");
    invoiceSearchBtn = new QPushButton("Rechercher");
    invoiceSearchLayout->addWidget(invoiceSearchEdit);
    invoiceSearchLayout->addWidget(invoiceSearchBtn);
    invoiceTabLayout->addLayout(invoiceSearchLayout);
    // Recherche clients (code existant)
    QHBoxLayout *searchLayout = new QHBoxLayout;
    searchLayout->addWidget(new QLabel("Rechercher par prénom :"));
    searchEdit = new QLineEdit;
    searchEdit->setPlaceholderText("Saisir le prénom...");
    searchButton = new QPushButton("Rechercher");
    QPushButton *resetButton = new QPushButton("Réinitialiser");
    searchLayout->addWidget(searchEdit);
    searchLayout->addWidget(searchButton);
    searchLayout->addWidget(resetButton);
    clientTabLayout->addLayout(searchLayout);

    // Table clients (code existant)
    clientModel = new QSqlTableModel(this);
    clientModel->setTable("clients");
    clientModel->setEditStrategy(QSqlTableModel::OnManualSubmit);
    clientModel->select();

    clientView = new QTableView;
    clientView->setModel(clientModel);
    clientView->setSelectionBehavior(QAbstractItemView::SelectRows);
    clientView->setSelectionMode(QAbstractItemView::SingleSelection);
    clientView->horizontalHeader()->setStretchLastSection(true);
    clientTabLayout->addWidget(clientView);

    // Boutons clients (code existant)
    QHBoxLayout *buttonLayout = new QHBoxLayout;
    addButton = new QPushButton("Ajouter");
    editButton = new QPushButton("Modifier");
    deleteButton = new QPushButton("Supprimer");
    refreshButton = new QPushButton("Actualiser");
    buttonLayout->addWidget(addButton);
    buttonLayout->addWidget(editButton);
    buttonLayout->addWidget(deleteButton);
    buttonLayout->addWidget(refreshButton);
    clientTabLayout->addLayout(buttonLayout);
       QHBoxLayout *invoiceButtonLayout = new QHBoxLayout;
    createInvoiceBtn = new QPushButton("+ Créer Facture");
    editInvoiceBtn = new QPushButton("✎ Modifier");
    deleteInvoiceBtn = new QPushButton("🗑️ Supprimer");
    actionsBtn = new QPushButton("⚙️ Actions (PDF/Email)");
    refreshInvoicesBtn = new QPushButton("🔄 Actualiser");
    invoiceButtonLayout->addWidget(createInvoiceBtn);
    invoiceButtonLayout->addWidget(editInvoiceBtn);
    invoiceButtonLayout->addWidget(deleteInvoiceBtn);
    invoiceButtonLayout->addWidget(actionsBtn);
    invoiceButtonLayout->addWidget(refreshInvoicesBtn);
    invoiceTabLayout->addLayout(invoiceButtonLayout);

    connect(createInvoiceBtn, &QPushButton::clicked, this, &AdminWindow::onCreateInvoice);
    connect(editInvoiceBtn, &QPushButton::clicked, this, &AdminWindow::onEditInvoice);
    connect(deleteInvoiceBtn, &QPushButton::clicked, this, &AdminWindow::onDeleteInvoice);
    connect(actionsBtn, &QPushButton::clicked, this, &AdminWindow::onInvoiceActions);
    connect(refreshInvoicesBtn, &QPushButton::clicked, this, &AdminWindow::onRefreshInvoices);

    tabWidget->addTab(invoiceTab, "📄 Gestion Factures");
invoiceModel = new QSqlQueryModel(this);
invoiceModel->setQuery(
    "SELECT f.id, f.numero, f.type, "
    "COALESCE(f.client_nom, c.nom || ' ' || c.prenom, 'N/A') as Client, "
    "f.date_creation, f.date_echeance, "
    "f.total_ht, f.total_tva, f.total_ttc, f.statut "
    "FROM factures f "
    "LEFT JOIN clients c ON f.client_id = c.id "
    "ORDER BY f.id DESC"
);
invoiceModel->setHeaderData(0, Qt::Horizontal, "ID");
invoiceModel->setHeaderData(1, Qt::Horizontal, "Numéro");
invoiceModel->setHeaderData(2, Qt::Horizontal, "Type");
invoiceModel->setHeaderData(3, Qt::Horizontal, "Client");
invoiceModel->setHeaderData(4, Qt::Horizontal, "Date création");
invoiceModel->setHeaderData(5, Qt::Horizontal, "Date échéance");
invoiceModel->setHeaderData(6, Qt::Horizontal, "Total HT");
invoiceModel->setHeaderData(7, Qt::Horizontal, "TVA");
invoiceModel->setHeaderData(8, Qt::Horizontal, "Total TTC");
invoiceModel->setHeaderData(9, Qt::Horizontal, "Statut");
invoiceView = new QTableView;
invoiceView->setModel(invoiceModel);
invoiceView->setSelectionBehavior(QAbstractItemView::SelectRows);
invoiceView->setSelectionMode(QAbstractItemView::SingleSelection);
invoiceView->horizontalHeader()->setStretchLastSection(true);
invoiceTabLayout->addWidget(invoiceView);

    // Connexions (code existant)
    connect(invoiceSearchBtn, &QPushButton::clicked, this, &AdminWindow::onSearchInvoice);
connect(invoiceSearchEdit, &QLineEdit::returnPressed, this, &AdminWindow::onSearchInvoice);
    connect(addButton, &QPushButton::clicked, this, &AdminWindow::onAddClient);
    connect(editButton, &QPushButton::clicked, this, &AdminWindow::onEditClient);
    connect(deleteButton, &QPushButton::clicked, this, &AdminWindow::onDeleteClient);
    connect(refreshButton, &QPushButton::clicked, this, &AdminWindow::refreshModel);
    connect(searchButton, &QPushButton::clicked, this, &AdminWindow::onSearch);
    connect(resetButton, &QPushButton::clicked, [this]() {
        searchEdit->clear();
        clientModel->setFilter("");
        refreshModel();
    });

    // Ajouter le tab des clients
    tabWidget->addTab(clientTab, "📋 Gestion Clients");

   
}
void AdminWindow::onSearchInvoice()
{
    QString search = invoiceSearchEdit->text().trimmed();
    if (search.isEmpty()) {
        invoiceModel->setFilter("");
    } else {
        invoiceModel->setFilter(
            QString("numero LIKE '%%1%' OR client_id IN "
                    "(SELECT id FROM clients WHERE nom LIKE '%%1%' OR prenom LIKE '%%1%')")
            .arg(search)
        );
    }
    invoiceModel->select();
}
void AdminWindow::onCreateInvoice()
{
    InvoiceCreateDialog dlg(-1, this);
    if (dlg.exec() == QDialog::Accepted) {
        onRefreshInvoices();
    }
}
void AdminWindow::onDeleteInvoice()
{
    int row = invoiceView->currentIndex().row();
    if (row < 0) {
        QMessageBox::warning(this, "Sélection", "Veuillez sélectionner une facture");
        return;
    }

    int invoiceId = invoiceModel->data(invoiceModel->index(row, 0)).toInt();

    QMessageBox::StandardButton reply = QMessageBox::question(this, "Confirmation",
        "Supprimer cette facture ?", QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        QSqlQuery query;
        query.prepare("DELETE FROM lignes_facture WHERE facture_id = ?");
        query.addBindValue(invoiceId);
        query.exec();

        query.prepare("DELETE FROM factures WHERE id = ?");
        query.addBindValue(invoiceId);

        if (query.exec()) {
            onRefreshInvoices();
            QMessageBox::information(this, "Succès", "Facture supprimée");
        }
    }
}
void AdminWindow::refreshModel()
{
    clientModel->select();
}
void AdminWindow::onSearchInvoice()
{
    QString search = invoiceSearchEdit->text().trimmed();
    if (search.isEmpty()) {
        invoiceModel->setQuery(
            "SELECT f.id, f.numero, f.type, "
            "COALESCE(f.client_nom, c.nom || ' ' || c.prenom, 'N/A') as Client, "
            "f.date_creation, f.date_echeance, "
            "f.total_ht, f.total_tva, f.total_ttc, f.statut "
            "FROM factures f "
            "LEFT JOIN clients c ON f.client_id = c.id "
            "ORDER BY f.id DESC"
        );
    } else {
        invoiceModel->setQuery(QString(
            "SELECT f.id, f.numero, f.type, "
            "COALESCE(f.client_nom, c.nom || ' ' || c.prenom, 'N/A') as Client, "
            "f.date_creation, f.date_echeance, "
            "f.total_ht, f.total_tva, f.total_ttc, f.statut "
            "FROM factures f "
            "LEFT JOIN clients c ON f.client_id = c.id "
            "WHERE f.numero LIKE '%%1%' OR f.client_nom LIKE '%%1%' "
            "OR c.nom LIKE '%%1%' "
            "ORDER BY f.id DESC"
        ).arg(search));
    }
}
void AdminWindow::onAddClient()
{
    ClientEditDialog dlg(this);
    if (dlg.exec() == QDialog::Accepted)
    {
        refreshModel();
    }
}

void AdminWindow::onEditClient()
{
    int row = clientView->currentIndex().row();
    if (row < 0)
    {
        QMessageBox::warning(this, "Sélection", "Veuillez sélectionner un client à modifier.");
        return;
    }
    int clientId = clientModel->data(clientModel->index(row, 0)).toInt();
    ClientEditDialog dlg(this);
    dlg.setClientId(clientId);
    if (dlg.exec() == QDialog::Accepted)
    {
        refreshModel();
    }
}
void AdminWindow::onInvoiceActions()
{
    int row = invoiceView->currentIndex().row();
    if (row < 0) {
        QMessageBox::warning(this, "Sélection", 
                             "Veuillez sélectionner une facture");
        return;
    }

    int invoiceId = invoiceModel->data(
        invoiceModel->index(row, 0)).toInt();
    
    InvoiceActionDialog dlg(invoiceId, this);
    dlg.exec();
}
void AdminWindow::onRefreshInvoices()
{
    invoiceModel->setQuery(invoiceModel->query().lastQuery());
}
void AdminWindow::onDeleteClient()
{
    int row = clientView->currentIndex().row();
    if (row < 0)
    {
        QMessageBox::warning(this, "Sélection", "Veuillez sélectionner un client à supprimer.");
        return;
    }
    int clientId = clientModel->data(clientModel->index(row, 0)).toInt();
    QMessageBox::StandardButton reply = QMessageBox::question(this, "Confirmation",
                                                              "Supprimer définitivement ce client ?", QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::Yes)
    {
        QSqlQuery query;
        query.prepare("DELETE FROM clients WHERE id = ?");
        query.addBindValue(clientId);
        if (query.exec())
        {
            refreshModel();
        }
        else
        {
            QMessageBox::critical(this, "Erreur", "Échec suppression : " + query.lastError().text());
        }
    }
}

void AdminWindow::onEditInvoice()
{
    int row = invoiceView->currentIndex().row();
    if (row < 0) {
        QMessageBox::warning(this, "Sélection", 
                             "Veuillez sélectionner une facture à modifier.");
        return;
    }

    int invoiceId = invoiceModel->data(invoiceModel->index(row, 0)).toInt();
    
    InvoiceEditDialog *dlg = new InvoiceEditDialog(invoiceId, this);
    if (dlg->exec() == QDialog::Accepted) {
        onRefreshInvoices();
    }
    delete dlg;
}
