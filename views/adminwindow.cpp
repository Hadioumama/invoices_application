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
    mainLayout->setContentsMargins(10, 10, 10, 10);
    mainLayout->setSpacing(10);

    // ===== TITRE =====
    QLabel *titleLabel = new QLabel(QString("Espace Administrateur ").arg(m_adminId));
    titleLabel->setStyleSheet("font-size: 20px; font-weight: bold; color: #2B6CB0;");
    titleLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(titleLabel);

    // ===== BOUTON DÉCONNEXION =====
    QPushButton *logoutBtn = new QPushButton("🔒 Déconnexion");
    logoutBtn->setStyleSheet(
        "background:#E53E3E; color:white; font-weight:bold; padding:8px; border-radius:6px;");
    logoutBtn->setFixedHeight(36);  // ← Hauteur fixe
    
    QHBoxLayout *topLayout = new QHBoxLayout;
    topLayout->addStretch();
    topLayout->addWidget(logoutBtn);
    mainLayout->addLayout(topLayout);

    // ===== TAB WIDGET — CORRECTION DÉFINITIVE =====
    QTabWidget *tabWidget = new QTabWidget(this);
    
    // ← CORRECTION CLÉ : Minimum size pour le tabWidget pour qu'il ne soit pas écrasé
    tabWidget->setMinimumSize(800, 500);
    
    // ← CORRECTION CLÉ : Size policy pour qu'il prenne l'espace disponible
    // mais respecte le minimum size hint des enfants
    tabWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    
    // ← CORRECTION CLÉ : Mode de redimensionnement des onglets
    tabWidget->setDocumentMode(false);
    
    mainLayout->addWidget(tabWidget, 1);  // ← Stretch factor 1

    // ===== TAB DASHBOARD =====
    DashboardWidget *dashboard = new DashboardWidget(this);
    tabWidget->addTab(dashboard, "📊 Dashboard");

    // ===== TAB ARTICLES =====
    ArticlesWidget *articlesWidget = new ArticlesWidget(this);
    tabWidget->addTab(articlesWidget, "📦 Articles");

    // ===== TAB GESTION FACTURES =====
QWidget *invoiceTab = new QWidget();
invoiceTab->setStyleSheet("background:#F7FAFC;");
QVBoxLayout *invoiceTabLayout =
    new QVBoxLayout(invoiceTab);
invoiceTabLayout->setContentsMargins(12, 12, 12, 12);
invoiceTabLayout->setSpacing(10);

// Bandeau titre
QFrame *invoiceHeader = new QFrame;
invoiceHeader->setFixedHeight(50);
invoiceHeader->setStyleSheet(
    "QFrame{"
    "background:#1B2A3B;"
    "border-radius:8px;}");
QHBoxLayout *headerLayout =
    new QHBoxLayout(invoiceHeader);
headerLayout->setContentsMargins(16, 0, 16, 0);
QLabel *invoiceTitle = new QLabel("📄 Gestion des Factures");
invoiceTitle->setStyleSheet(
    "color:white;font-size:15px;"
    "font-weight:bold;background:transparent;");
headerLayout->addWidget(invoiceTitle);
headerLayout->addStretch();
invoiceTabLayout->addWidget(invoiceHeader);

// Barre de recherche stylisée
QFrame *searchFrame = new QFrame;
searchFrame->setStyleSheet(
    "QFrame{background:white;"
    "border:1px solid #E2E8F0;"
    "border-radius:8px;}");
searchFrame->setFixedHeight(50);
QHBoxLayout *searchFrameLayout =
    new QHBoxLayout(searchFrame);
searchFrameLayout->setContentsMargins(12, 8, 12, 8);
searchFrameLayout->setSpacing(8);

QLabel *searchIcon = new QLabel("🔍");
searchIcon->setStyleSheet("background:transparent;");
invoiceSearchEdit = new QLineEdit;
invoiceSearchEdit->setPlaceholderText(
    "Rechercher par N° facture, client...");
invoiceSearchEdit->setStyleSheet(
    "border:none;background:transparent;"
    "font-size:13px;color:#2D3748;");
invoiceSearchEdit->setFixedHeight(34);
invoiceSearchBtn = new QPushButton("Rechercher");
invoiceSearchBtn->setFixedHeight(32);
invoiceSearchBtn->setStyleSheet(
    "background:#2B6CB0;color:white;"
    "font-weight:bold;border-radius:6px;"
    "padding:0 16px;");
QPushButton *clearSearchBtn =
    new QPushButton("✕");
clearSearchBtn->setFixedSize(32, 32);
clearSearchBtn->setStyleSheet(
    "background:#EDF2F7;color:#718096;"
    "border-radius:6px;font-weight:bold;");

searchFrameLayout->addWidget(searchIcon);
searchFrameLayout->addWidget(invoiceSearchEdit, 1);
searchFrameLayout->addWidget(invoiceSearchBtn);
searchFrameLayout->addWidget(clearSearchBtn);
invoiceTabLayout->addWidget(searchFrame);

// Barre boutons actions
QFrame *actionsFrame = new QFrame;
actionsFrame->setStyleSheet(
    "QFrame{background:white;"
    "border:1px solid #E2E8F0;"
    "border-radius:8px;}");
actionsFrame->setFixedHeight(52);
QHBoxLayout *actionsLayout =
    new QHBoxLayout(actionsFrame);
actionsLayout->setContentsMargins(12, 8, 12, 8);
actionsLayout->setSpacing(8);

createInvoiceBtn =
    new QPushButton("➕ Créer");
editInvoiceBtn =
    new QPushButton("✏️ Modifier");
deleteInvoiceBtn =
    new QPushButton("🗑️ Supprimer");
actionsBtn =
    new QPushButton("⚙️ Actions");
paymentBtn =
    new QPushButton("💳 Paiements");
refreshInvoicesBtn =
    new QPushButton("🔄");

// Styles par couleur
QString btnBase =
    "font-weight:bold;border-radius:6px;"
    "border:none;font-size:12px;";
createInvoiceBtn->setStyleSheet(
    btnBase + "background:#27AE60;color:white;");
editInvoiceBtn->setStyleSheet(
    btnBase + "background:#3182CE;color:white;");
deleteInvoiceBtn->setStyleSheet(
    btnBase + "background:#E53E3E;color:white;");
actionsBtn->setStyleSheet(
    btnBase + "background:#DD6B20;color:white;");
paymentBtn->setStyleSheet(
    btnBase + "background:#805AD5;color:white;");
refreshInvoicesBtn->setStyleSheet(
    btnBase + "background:#718096;color:white;");

// Hauteurs
createInvoiceBtn->setFixedHeight(34);
editInvoiceBtn->setFixedHeight(34);
deleteInvoiceBtn->setFixedHeight(34);
actionsBtn->setFixedHeight(34);
paymentBtn->setFixedHeight(34);
refreshInvoicesBtn->setFixedSize(34, 34);

// Curseurs
createInvoiceBtn->setCursor(Qt::PointingHandCursor);
editInvoiceBtn->setCursor(Qt::PointingHandCursor);
deleteInvoiceBtn->setCursor(Qt::PointingHandCursor);
actionsBtn->setCursor(Qt::PointingHandCursor);
paymentBtn->setCursor(Qt::PointingHandCursor);
refreshInvoicesBtn->setCursor(Qt::PointingHandCursor);

actionsLayout->addWidget(createInvoiceBtn);
actionsLayout->addWidget(editInvoiceBtn);
actionsLayout->addWidget(deleteInvoiceBtn);
actionsLayout->addWidget(actionsBtn);
actionsLayout->addWidget(paymentBtn);
actionsLayout->addStretch();
actionsLayout->addWidget(refreshInvoicesBtn);
invoiceTabLayout->addWidget(actionsFrame);

// Tableau factures
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
invoiceView->setSelectionBehavior(
    QAbstractItemView::SelectRows);
invoiceView->setSelectionMode(
    QAbstractItemView::SingleSelection);
invoiceView->horizontalHeader()
            ->setStretchLastSection(true);
invoiceView->horizontalHeader()
            ->setSectionResizeMode(
                QHeaderView::Stretch);
invoiceView->verticalHeader()->setVisible(false);
invoiceView->setAlternatingRowColors(true);
invoiceView->setEditTriggers(
    QAbstractItemView::NoEditTriggers);
invoiceView->setSizePolicy(
    QSizePolicy::Expanding,
    QSizePolicy::Expanding);
invoiceView->setStyleSheet(
    "QTableView{"
    "border:1px solid #E2E8F0;"
    "border-radius:8px;"
    "gridline-color:#EDF2F7;"
    "background:white;"
    "selection-background-color:#BEE3F8;"
    "selection-color:#2D3748;}"
    "QTableView::item{"
    "padding:8px 6px;"
    "border-bottom:1px solid #EDF2F7;}"
    "QTableView::item:selected{"
    "background:#BEE3F8;color:#1A202C;}"
    "QTableView::item:alternate{"
    "background:#F7FAFC;}"
    "QHeaderView::section{"
    "background:#1B2A3B;color:white;"
    "font-weight:bold;padding:9px 6px;"
    "border:none;"
    "border-right:1px solid #2D3748;"
    "font-size:11px;}");
invoiceView->setColumnHidden(0, true);

invoiceTabLayout->addWidget(invoiceView, 1);
tabWidget->addTab(invoiceTab, "📄 Gestion Factures");

// Connexion bouton clear recherche
connect(clearSearchBtn, &QPushButton::clicked,
        [this](){
    invoiceSearchEdit->clear();
    onRefreshInvoices();
});
    // ===== TAB GESTION CLIENTS — COMPACT VERS LE HAUT =====
    QWidget *clientTab = new QWidget();
    QVBoxLayout *clientTabLayout = new QVBoxLayout(clientTab);
    clientTabLayout->setContentsMargins(8, 0, 8, 0);   // ← Marges réduites (4px haut/bas)
    clientTabLayout->setSpacing(0);                     // ← Espacement réduit (4px)

    // -- Titre compact --
    QLabel *clientTitle = new QLabel("📋 Gestion Clients");
    clientTitle->setStyleSheet(
        "font-size:14px;font-weight:bold;color:#1B2A3B;"  // ← 14px au lieu de 16px
        "margin:0px;padding:0px;");                       // ← Pas de marge
    clientTitle->setFixedHeight(14);                      // ← Hauteur fixe compacte
    clientTabLayout->addWidget(clientTitle);

    // -- Recherche clients — ULTRA COMPACT --
    QHBoxLayout *searchLayout = new QHBoxLayout;
    searchLayout->setSpacing(6);                          // ← Espacement réduit
    searchLayout->setContentsMargins(0, 0, 0, 0);          // ← Pas de marge interne
    
    searchEdit = new QLineEdit;
    searchEdit->setPlaceholderText("🔍 Rechercher par prénom...");
    searchEdit->setStyleSheet(
        "padding:5px;border:1px solid #CBD5E0;"           // ← 5px au lieu de 7px
        "border-radius:4px;font-size:11px;");
    searchEdit->setFixedHeight(28);                         // ← 28px au lieu de 32px
    
    searchButton = new QPushButton("Rechercher");
    searchButton->setStyleSheet(
        "background:#3182CE;color:white;font-weight:bold;"
        "padding:4px 12px;border-radius:4px;border:none;"  // ← Padding réduit
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

    // -- Table clients — HAUTEUR RÉDUITE --
    clientModel = new QSqlTableModel(this);
    clientModel->setTable("clients");
    clientModel->setEditStrategy(QSqlTableModel::OnManualSubmit);
    clientModel->select();

    clientView = new QTableView;
    clientView->setModel(clientModel);
    clientView->setSelectionBehavior(QAbstractItemView::SelectRows);
    clientView->setSelectionMode(QAbstractItemView::SingleSelection);
    clientView->horizontalHeader()->setStretchLastSection(true);
    clientView->setColumnHidden(4, true);
    clientView->setColumnHidden(5, true);
    clientView->verticalHeader()->setVisible(false);
    clientView->setAlternatingRowColors(true);
    clientView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    
    // ← HAUTEUR FIXE RÉDUITE pour faire remonter les boutons
    clientView->setFixedHeight(200);                        // ← 200px au lieu de 280px
    clientView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    
    // Style du tableau
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
        "  padding: 5px;"                                    // ← 5px au lieu de 7px
        "  border: none;"
        "  font-size: 11px;"                                 // ← Police réduite
        "}"
    );

    clientTabLayout->addWidget(clientView);

    // -- Boutons clients — COMPACTS ET COLORÉS --
    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->setSpacing(6);                             // ← Espacement réduit
    buttonLayout->setContentsMargins(0, 0, 0, 0);          // ← Pas de marge

    addButton = new QPushButton("➕ Ajouter");
    editButton = new QPushButton("✏️ Modifier");
    deleteButton = new QPushButton("🗑️ Supprimer");
    refreshButton = new QPushButton("🔄 Actualiser");
    
    // Styles colorés
    addButton->setStyleSheet(
        "background:#27AE60;color:white;font-weight:bold;"
        "padding:4px 12px;border-radius:4px;border:none;"
        "font-size:11px;");
    editButton->setStyleSheet(
        "background:#3182CE;color:white;font-weight:bold;"
        "padding:4px 12px;border-radius:4px;border:none;"
        "font-size:11px;");
    deleteButton->setStyleSheet(
        "background:#E53E3E;color:white;font-weight:bold;"
        "padding:4px 12px;border-radius:4px;border:none;"
        "font-size:11px;");
    refreshButton->setStyleSheet(
        "background:#718096;color:white;font-weight:bold;"
        "padding:4px 12px;border-radius:4px;border:none;"
        "font-size:11px;");

    // Hauteur réduite
    addButton->setFixedHeight(30);                          // ← 30px au lieu de 34px
    editButton->setFixedHeight(30);
    deleteButton->setFixedHeight(30);
    refreshButton->setFixedHeight(30);
    
    addButton->setMinimumWidth(90);                         // ← 90px au lieu de 100px
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