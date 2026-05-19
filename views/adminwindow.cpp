#include "views/adminwindow.h"
#include "invoicemanagementwidget.h"
#include "dialogs/invoiceeditdialog.h"
#include "dialogs/client_edit_dialog.h"
#include "views/articleswidget.h"
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
#include <QTabWidget>
#include "utils/invoicegenerator.h"
#include "views/dashboardwidget.h"
#include "dialogs/invoicecreatedialog.h"
#include "dialogs/invoiceactiondialog.h"
#include "dialogs/paymentdialog.h"

AdminWindow::AdminWindow(int adminId, QWidget *parent)
    : QWidget(parent),  // ✅ QWidget, pas QMainWindow
      m_adminId(adminId),
      m_invoiceDialog(nullptr)
{
    
    setupUI();
}

AdminWindow::~AdminWindow()
{
}
void AdminWindow::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(10, 2, 10, 2);
    mainLayout->setSpacing(4);

    // ===== TITRE =====
    QLabel *titleLabel = new QLabel(QString("Espace Administrateur ").arg(m_adminId));
    titleLabel->setStyleSheet("font-size: 18px; font-weight: bold; color: #2B6CB0; margin: 0px; padding: 0px;");
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setFixedHeight(24);
    mainLayout->addWidget(titleLabel);

    // ===== BOUTON DÉCONNEXION =====
    QPushButton *logoutBtn = new QPushButton("🔒 Déconnexion");
    logoutBtn->setStyleSheet(
        "background:#E53E3E; color:white; font-weight:bold; padding:4px 8px; border-radius:6px; font-size:11px;");
    logoutBtn->setFixedHeight(28);
    
    QHBoxLayout *topLayout = new QHBoxLayout;
    topLayout->setContentsMargins(0, 0, 0, 0);
    topLayout->setSpacing(0);
    topLayout->addStretch();
    topLayout->addWidget(logoutBtn);
    mainLayout->addLayout(topLayout);

    // ===== TAB WIDGET =====
    QTabWidget *tabWidget = new QTabWidget(this);
    tabWidget->setMinimumSize(800, 500);
    tabWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    tabWidget->setDocumentMode(false);
    tabWidget->setStyleSheet("QTabWidget::pane { border: none; padding: 0px; margin: 0px; }");
    
    mainLayout->addWidget(tabWidget);

    // ===== TAB DASHBOARD =====
    DashboardWidget *dashboard = new DashboardWidget(this);
    tabWidget->addTab(dashboard, "📊 Dashboard");

    // ===== TAB ARTICLES =====
    ArticlesWidget *articlesWidget = new ArticlesWidget(this);
    tabWidget->addTab(articlesWidget, "📦 Articles");
    // ===== TAB GESTION FACTURES — DESIGN IDENTIQUE À GESTION CLIENTS =====
    QWidget *invoiceTab = new QWidget();
    QVBoxLayout *invoiceTabLayout = new QVBoxLayout(invoiceTab);
    invoiceTabLayout->setContentsMargins(12, 8, 12, 8);   // ← Marge haut 0
    invoiceTabLayout->setSpacing(8);                     // ← Aucun espacement

    // -- Titre compact — collé en haut --
    QLabel *invoiceTitle = new QLabel("📄 Gestion des Factures");
    invoiceTitle->setStyleSheet(
        "font-size:14px;font-weight:bold;color:#1B2A3B;"
        "margin:0px;padding:0px;");
    invoiceTitle->setFixedHeight(22);                    // ← CHANGEMENT : 18 → 22
invoiceTitle->setContentsMargins(0, 0, 0, 4);        // ← CHANGEMENT : marge bas 4px
invoiceTabLayout->addWidget(invoiceTitle);            // ← Supprime , 0, Qt::AlignTop

    // -- Recherche factures —
    QHBoxLayout *invoiceSearchLayout = new QHBoxLayout;
    invoiceSearchLayout->setSpacing(6);
    invoiceSearchLayout->setContentsMargins(0, 0, 0, 0);

    invoiceSearchEdit = new QLineEdit;
    invoiceSearchEdit->setPlaceholderText("🔍 Rechercher par N° facture, client...");
    invoiceSearchEdit->setStyleSheet(
        "padding:5px;border:1px solid #CBD5E0;"
        "border-radius:4px;font-size:11px;");
    invoiceSearchEdit->setFixedHeight(28);

    invoiceSearchBtn = new QPushButton("Rechercher");
    invoiceSearchBtn->setStyleSheet(
        "background:#3182CE;color:white;font-weight:bold;"
        "padding:4px 12px;border-radius:4px;border:none;"
        "font-size:11px;");
    invoiceSearchBtn->setFixedHeight(28);
    invoiceSearchBtn->setCursor(Qt::PointingHandCursor);

    QPushButton *clearSearchBtn = new QPushButton("✕");
    clearSearchBtn->setStyleSheet(
        "background:#718096;color:white;font-weight:bold;"
        "padding:4px 12px;border-radius:4px;border:none;"
        "font-size:11px;");
    clearSearchBtn->setFixedHeight(28);
    clearSearchBtn->setCursor(Qt::PointingHandCursor);

    invoiceSearchLayout->addWidget(invoiceSearchEdit, 1);
    invoiceSearchLayout->addWidget(invoiceSearchBtn);
    invoiceSearchLayout->addWidget(clearSearchBtn);
    invoiceTabLayout->addLayout(invoiceSearchLayout);

    // -- Boutons actions — MÊME STYLE EXACT QUE CLIENTS --
    QHBoxLayout *invoiceButtonLayout = new QHBoxLayout;
    invoiceButtonLayout->setSpacing(6);
    invoiceButtonLayout->setContentsMargins(0, 0, 0, 0);

    createInvoiceBtn = new QPushButton("➕ Créer");
    editInvoiceBtn = new QPushButton("✏️ Modifier");
    deleteInvoiceBtn = new QPushButton("🗑️ Supprimer");
    actionsBtn = new QPushButton("⚙️ Actions");
    paymentBtn = new QPushButton("💳 Paiements");
    refreshInvoicesBtn = new QPushButton("🔄");

    // Styles IDENTIQUES à Clients (même QString partagé)
    QString btnBaseStyle =
        "font-weight:bold;border-radius:4px;border:none;"
        "padding:4px 12px;font-size:11px;color:white;";
    createInvoiceBtn->setStyleSheet(btnBaseStyle + "background:#27AE60;");
    editInvoiceBtn->setStyleSheet(btnBaseStyle + "background:#3182CE;");
    deleteInvoiceBtn->setStyleSheet(btnBaseStyle + "background:#E53E3E;");
    actionsBtn->setStyleSheet(btnBaseStyle + "background:#DD6B20;");
    paymentBtn->setStyleSheet(btnBaseStyle + "background:#805AD5;");
    refreshInvoicesBtn->setStyleSheet(btnBaseStyle + "background:#718096;");

    // Hauteur IDENTIQUE à Clients
    createInvoiceBtn->setFixedHeight(30);
    editInvoiceBtn->setFixedHeight(30);
    deleteInvoiceBtn->setFixedHeight(30);
    actionsBtn->setFixedHeight(30);
    paymentBtn->setFixedHeight(30);
    refreshInvoicesBtn->setFixedHeight(30);

    // Largeur IDENTIQUE à Clients
    createInvoiceBtn->setMinimumWidth(90);
    editInvoiceBtn->setMinimumWidth(90);
    deleteInvoiceBtn->setMinimumWidth(90);
    actionsBtn->setMinimumWidth(90);
    paymentBtn->setMinimumWidth(90);
    refreshInvoicesBtn->setMinimumWidth(90);

    // Curseurs
    createInvoiceBtn->setCursor(Qt::PointingHandCursor);
    editInvoiceBtn->setCursor(Qt::PointingHandCursor);
    deleteInvoiceBtn->setCursor(Qt::PointingHandCursor);
    actionsBtn->setCursor(Qt::PointingHandCursor);
    paymentBtn->setCursor(Qt::PointingHandCursor);
    refreshInvoicesBtn->setCursor(Qt::PointingHandCursor);

    invoiceButtonLayout->addWidget(createInvoiceBtn);
    invoiceButtonLayout->addWidget(editInvoiceBtn);
    invoiceButtonLayout->addWidget(deleteInvoiceBtn);
    invoiceButtonLayout->addWidget(actionsBtn);
    invoiceButtonLayout->addWidget(paymentBtn);
    invoiceButtonLayout->addStretch();
    invoiceButtonLayout->addWidget(refreshInvoicesBtn);
    invoiceTabLayout->addLayout(invoiceButtonLayout);

    // -- Tableau factures — STYLE IDENTIQUE À CLIENTS --
    invoiceModel = new QSqlQueryModel(this);
    invoiceModel->setQuery(
        "SELECT f.id, f.numero, f.type, "
        "COALESCE(f.client_nom, c.nom || ' ' || "
        "c.prenom, 'N/A') as Client, "
        "f.date_creation, f.date_echeance, "
        "f.total_ht, f.total_tva, "
        "f.total_ttc, f.statut "
        "FROM factures f "
        "LEFT JOIN clients c ON f.client_id = c.id "
        "ORDER BY f.id DESC");
    invoiceModel->setHeaderData(0,Qt::Horizontal,"ID");
    invoiceModel->setHeaderData(1,Qt::Horizontal,"Numéro");
    invoiceModel->setHeaderData(2,Qt::Horizontal,"Type");
    invoiceModel->setHeaderData(3,Qt::Horizontal,"Client");
    invoiceModel->setHeaderData(4,Qt::Horizontal,"Date créa.");
    invoiceModel->setHeaderData(5,Qt::Horizontal,"Échéance");
    invoiceModel->setHeaderData(6,Qt::Horizontal,"HT");
    invoiceModel->setHeaderData(7,Qt::Horizontal,"TVA");
    invoiceModel->setHeaderData(8,Qt::Horizontal,"TTC");
    invoiceModel->setHeaderData(9,Qt::Horizontal,"Statut");

    invoiceView = new QTableView;
    invoiceView->setModel(invoiceModel);
    invoiceView->setSelectionBehavior(QAbstractItemView::SelectRows);
    invoiceView->setSelectionMode(QAbstractItemView::SingleSelection);
    invoiceView->horizontalHeader()->setStretchLastSection(true);
    invoiceView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    invoiceView->verticalHeader()->setVisible(false);
    invoiceView->setAlternatingRowColors(true);
    invoiceView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    
    // Hauteur IDENTIQUE à Clients
    invoiceView->setFixedHeight(200);
    invoiceView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    // Style IDENTIQUE à Clients
    invoiceView->setStyleSheet(
        "QTableView {"
        "  border: 1px solid #E2E8F0;"
        "  gridline-color: #EDF2F7;"
        "  selection-background-color: #BEE3F8;"
        "  selection-color: #2D3748;"
        "}"
        "QHeaderView::section {"
        "  background: #2B6CB0;"
        "  color: white;"
        "  font-weight: bold;"
        "  padding: 5px;"
        "  border: none;"
        "  font-size: 11px;"
        "}"
    );
    invoiceView->setColumnHidden(0, true);

    invoiceTabLayout->addWidget(invoiceView);

    // ← AJOUT CLÉ : empêcher l'expansion verticale inutile
    invoiceTabLayout->addStretch(1);

    tabWidget->addTab(invoiceTab, "📄 Gestion Factures");

    // Connexion bouton clear recherche
    connect(clearSearchBtn, &QPushButton::clicked,
            [this](){
        invoiceSearchEdit->clear();
        onRefreshInvoices();
    });
        // ===== TAB GESTION CLIENTS — DESIGN IDENTIQUE À GESTION FACTURES =====
    QWidget *clientTab = new QWidget();
    QVBoxLayout *clientTabLayout = new QVBoxLayout(clientTab);
    clientTabLayout->setContentsMargins(12, 8, 12, 8);   // ← IDENTIQUE à Factures
    clientTabLayout->setSpacing(8);                       // ← IDENTIQUE à Factures

    // -- Titre compact —
    QLabel *clientTitle = new QLabel("📋 Gestion Clients");
    clientTitle->setStyleSheet(
        "font-size:14px;font-weight:bold;color:#1B2A3B;"
        "margin:0px;padding:0px;");
    clientTitle->setFixedHeight(22);                       // ← IDENTIQUE à Factures
    clientTitle->setContentsMargins(0, 0, 0, 4);           // ← IDENTIQUE à Factures
    clientTabLayout->addWidget(clientTitle);             // ← IDENTIQUE (pas de AlignTop)

    // -- Recherche clients —
    QHBoxLayout *searchLayout = new QHBoxLayout;
    searchLayout->setSpacing(6);
    searchLayout->setContentsMargins(0, 0, 0, 0);

    searchEdit = new QLineEdit;
    searchEdit->setPlaceholderText("🔍 Rechercher par prénom...");
    searchEdit->setStyleSheet(
        "padding:5px;border:1px solid #CBD5E0;"
        "border-radius:4px;font-size:11px;");
    searchEdit->setFixedHeight(28);

    searchButton = new QPushButton("Rechercher");
    searchButton->setStyleSheet(
        "background:#3182CE;color:white;font-weight:bold;"
        "padding:4px 12px;border-radius:4px;border:none;"
        "font-size:11px;");
    searchButton->setFixedHeight(28);
    searchButton->setCursor(Qt::PointingHandCursor);

    QPushButton *resetButton = new QPushButton("✕ Réinitialiser");
    resetButton->setStyleSheet(
        "background:#718096;color:white;font-weight:bold;"
        "padding:4px 12px;border-radius:4px;border:none;"
        "font-size:11px;");
    resetButton->setFixedHeight(28);
    resetButton->setCursor(Qt::PointingHandCursor);

    searchLayout->addWidget(searchEdit, 1);
    searchLayout->addWidget(searchButton);
    searchLayout->addWidget(resetButton);
    clientTabLayout->addLayout(searchLayout);

    // -- Boutons clients — AU-DESSUS DU TABLEAU —
    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->setSpacing(6);
    buttonLayout->setContentsMargins(0, 0, 0, 0);

    addButton = new QPushButton("➕ Ajouter");
    editButton = new QPushButton("✏️ Modifier");
    deleteButton = new QPushButton("🗑️ Supprimer");
    refreshButton = new QPushButton("🔄 Actualiser");

    // Styles IDENTIQUES à Factures
    QString clientStyleBtn =
        "font-weight:bold;border-radius:4px;border:none;"
        "padding:4px 12px;font-size:11px;color:white;";
    addButton->setStyleSheet(clientStyleBtn + "background:#27AE60;");
    editButton->setStyleSheet(clientStyleBtn + "background:#3182CE;");
    deleteButton->setStyleSheet(clientStyleBtn + "background:#E53E3E;");
    refreshButton->setStyleSheet(clientStyleBtn + "background:#718096;");

    // Hauteur IDENTIQUE à Factures
    addButton->setFixedHeight(30);
    editButton->setFixedHeight(30);
    deleteButton->setFixedHeight(30);
    refreshButton->setFixedHeight(30);

    // Largeur IDENTIQUE à Factures
    addButton->setMinimumWidth(90);
    editButton->setMinimumWidth(90);
    deleteButton->setMinimumWidth(90);
    refreshButton->setMinimumWidth(90);

    addButton->setCursor(Qt::PointingHandCursor);
    editButton->setCursor(Qt::PointingHandCursor);
    deleteButton->setCursor(Qt::PointingHandCursor);
    refreshButton->setCursor(Qt::PointingHandCursor);

    buttonLayout->addWidget(addButton);
    buttonLayout->addWidget(editButton);
    buttonLayout->addWidget(deleteButton);
    buttonLayout->addWidget(refreshButton);
    buttonLayout->addStretch();
    clientTabLayout->addLayout(buttonLayout);

    // -- Table clients — EN DESSOUS DES BOUTONS —
    clientModel = new QSqlTableModel(this);
    clientModel->setTable("clients");
    clientModel->setEditStrategy(QSqlTableModel::OnManualSubmit);
    clientModel->select();

    clientView = new QTableView;
    clientView->setModel(clientModel);
    clientView->setSelectionBehavior(QAbstractItemView::SelectRows);
    clientView->setSelectionMode(QAbstractItemView::SingleSelection);
    clientView->horizontalHeader()->setStretchLastSection(true);
   clientView->setColumnHidden(4,  true); // mot_de_passe
clientView->setColumnHidden(10, true); // role
clientView->setColumnHidden(12, true); // signature_path
clientView->setColumnHidden(13, true); // logo_path
    clientView->verticalHeader()->setVisible(false);
    clientView->setAlternatingRowColors(true);
    clientView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    
    // Hauteur IDENTIQUE à Factures
    clientView->setFixedHeight(200);
    clientView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    // Style IDENTIQUE à Factures
    clientView->setStyleSheet(
        "QTableView {"
        "  border: 1px solid #E2E8F0;"
        "  gridline-color: #EDF2F7;"
        "  selection-background-color: #BEE3F8;"
        "  selection-color: #2D3748;"
        "}"
        "QHeaderView::section {"
        "  background: #2B6CB0;"
        "  color: white;"
        "  font-weight: bold;"
        "  padding: 5px;"
        "  border: none;"
        "  font-size: 11px;"
        "}"
    );

    clientTabLayout->addWidget(clientView);

    // ← AJOUT : empêcher l'expansion verticale (IDENTIQUE à Factures)
    clientTabLayout->addStretch(1);

    tabWidget->addTab(clientTab, "📋 Gestion Clients");
    // ===== CONNEXIONS =====
    connect(createInvoiceBtn, &QPushButton::clicked, this, &AdminWindow::onCreateInvoice);
    connect(editInvoiceBtn, &QPushButton::clicked, this, &AdminWindow::onEditInvoice);
    connect(deleteInvoiceBtn, &QPushButton::clicked, this, &AdminWindow::onDeleteInvoice);
    connect(actionsBtn, &QPushButton::clicked, this, &AdminWindow::onInvoiceActions);
    connect(paymentBtn, &QPushButton::clicked, this, &AdminWindow::onPaymentClicked);
    connect(refreshInvoicesBtn, &QPushButton::clicked, this, &AdminWindow::onRefreshInvoices);
    connect(invoiceSearchBtn, &QPushButton::clicked, this, &AdminWindow::onSearchInvoice);
    connect(invoiceSearchEdit, &QLineEdit::returnPressed, this, &AdminWindow::onSearchInvoice);

    connect(addButton, &QPushButton::clicked, this, &AdminWindow::onAddClient);
    connect(editButton, &QPushButton::clicked, this, &AdminWindow::onEditClient);
    connect(deleteButton, &QPushButton::clicked, this, &AdminWindow::onDeleteClient);
    connect(refreshButton, &QPushButton::clicked, this, &AdminWindow::refreshModel);
    connect(searchButton, &QPushButton::clicked, this, &AdminWindow::onSearch);
   

    connect(logoutBtn, &QPushButton::clicked, this, &AdminWindow::onLogout);

    // Articles → InvoiceCreateDialog
    if (!m_invoiceDialog) {
        m_invoiceDialog = new InvoiceCreateDialog(-1, this);
    }
    connect(articlesWidget, &ArticlesWidget::articleSelected, 
            m_invoiceDialog, &InvoiceCreateDialog::onArticleFromCatalog);
}

// ============================================
// MÉTHODES FACTURES (inchangées)
// ============================================

void AdminWindow::onSearchInvoice()
{
    QString search = invoiceSearchEdit->text().trimmed();
    if (search.isEmpty()) {
        onRefreshInvoices();
        return;
    }
    
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

void AdminWindow::onCreateInvoice()
{
    InvoiceCreateDialog dlg(-1, this);
dlg.showMaximized(); // ← ouvre en plein écran
if (dlg.exec() == QDialog::Accepted) {
    onRefreshInvoices();
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

void AdminWindow::onInvoiceActions()
{
    int row = invoiceView->currentIndex().row();
    if (row < 0) {
        QMessageBox::warning(this, "Sélection", 
                             "Veuillez sélectionner une facture");
        return;
    }

    int invoiceId = invoiceModel->data(invoiceModel->index(row, 0)).toInt();
    
    InvoiceActionDialog dlg(invoiceId, this);
    dlg.exec();
    onRefreshInvoices();
}

void AdminWindow::onPaymentClicked()
{
    int row = invoiceView->currentIndex().row();
    if (row < 0) {
        QMessageBox::warning(this, "Sélection", "Veuillez sélectionner une facture");
        return;
    }
    
    int invoiceId = invoiceModel->data(invoiceModel->index(row, 0)).toInt();
    
    PaymentDialog *dlg = new PaymentDialog(invoiceId, this);
    dlg->exec();
    delete dlg;
    onRefreshInvoices();
}

void AdminWindow::onRefreshInvoices()
{
    delete invoiceModel;
    invoiceModel = new QSqlQueryModel(this);
    
    invoiceModel->setQuery(QString(
        "SELECT f.id, f.numero, f.type, "
        "COALESCE(f.client_nom, c.nom || ' ' || c.prenom, 'N/A') as Client, "
        "f.date_creation, f.date_echeance, "
        "f.total_ht, f.total_tva, f.total_ttc, f.statut "
        "FROM factures f "
        "LEFT JOIN clients c ON f.client_id = c.id "
        "ORDER BY f.id DESC"
    ));
    
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
    
    invoiceView->setModel(invoiceModel);
    invoiceView->setSelectionBehavior(QAbstractItemView::SelectRows);
    invoiceView->setSelectionMode(QAbstractItemView::SingleSelection);
    invoiceView->horizontalHeader()->setStretchLastSection(true);
}

// ============================================
// MÉTHODES CLIENTS (inchangées)
// ============================================

void AdminWindow::refreshModel()
{
    clientModel->setFilter("");
    clientModel->select();
}

void AdminWindow::onSearch()
{
    QString prenom = searchEdit->text().trimmed();
    if (prenom.isEmpty()) {
        clientModel->setFilter("role = 'client'");
    } else {
        clientModel->setFilter(
            QString("role = 'client' AND prenom LIKE '%%1%'")
            .arg(prenom));
    }
    clientModel->select();
}

void AdminWindow::onAddClient()
{
    ClientEditDialog dlg(this);
    if (dlg.exec() == QDialog::Accepted) {
        refreshModel();
        QMessageBox::information(this, "Succès", 
                                 "Client ajouté avec succès !");
    }
}

void AdminWindow::onEditClient()
{
    int row = clientView->currentIndex().row();
    if (row < 0) {
        QMessageBox::warning(this, "Sélection", "Veuillez sélectionner un client à modifier.");
        return;
    }
    int clientId = clientModel->data(clientModel->index(row, 0)).toInt();
    ClientEditDialog dlg(this);
    dlg.setClientId(clientId);
    if (dlg.exec() == QDialog::Accepted) {
        refreshModel();
    }
}

void AdminWindow::onDeleteClient()
{
    int row = clientView->currentIndex().row();
    if (row < 0) {
        QMessageBox::warning(this, "Sélection", "Veuillez sélectionner un client à supprimer.");
        return;
    }
    int clientId = clientModel->data(clientModel->index(row, 0)).toInt();
    QMessageBox::StandardButton reply = QMessageBox::question(this, "Confirmation",
                                                              "Supprimer définitivement ce client ?", QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::Yes) {
        QSqlQuery query;
        query.prepare("DELETE FROM clients WHERE id = ?");
        query.addBindValue(clientId);
        if (query.exec()) {
            refreshModel();
        } else {
            QMessageBox::critical(this, "Erreur", "Échec suppression : " + query.lastError().text());
        }
    }
}

void AdminWindow::onLogout()
{
    auto reply = QMessageBox::question(this, "Déconnexion",
        "Voulez-vous vous déconnecter ?",
        QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        emit logoutRequested();
    }
}