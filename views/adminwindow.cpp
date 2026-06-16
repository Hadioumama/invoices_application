#include "views/adminwindow.h"
#include "views/dashboardwidget.h"
#include "views/articleswidget.h"
#include "dialogs/invoiceeditdialog.h"
#include "dialogs/client_edit_dialog.h"
#include "dialogs/invoicecreatedialog.h"
#include "dialogs/invoiceactiondialog.h"
#include "dialogs/paymentdialog.h"
#include "views/entreprise_config_widget.h"
#include "utils/entreprise_config_manager.h"
#include "utils/invoicegenerator.h"          // ⬅️ AJOUTÉ
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QTableView>
#include <QHeaderView>
#include <QPushButton>
#include <QMessageBox>
#include <QDesktopServices>
#include <QSqlQuery>
#include <QSqlQueryModel>
#include <QSqlError>
#include <QLineEdit>
#include <QLabel>
#include <QFrame>
#include <QGraphicsDropShadowEffect>
#include <QFileDialog>                       // ⬅️ AJOUTÉ
#include <QStackedWidget>                    // ⬅️ AJOUTÉ (si manquant)

namespace A {
    constexpr auto BG        = "#F1F5F9";
    constexpr auto CARD      = "#FFFFFF";
    constexpr auto BORDER    = "#E2E8F0";
    constexpr auto TXT_HEAD  = "rgb(4, 19, 50)";
    constexpr auto TXT_SUB   = "#64748B";
    constexpr auto BLUE      = "#2563EB";
    constexpr auto GREEN     = "#16A34A";
    constexpr auto RED       = "#DC2626";
    constexpr auto GRAY      = "#64748B";
    constexpr auto AMBER     = "#D97706";
    constexpr auto VIOLET    = "#7C3AED";
    constexpr auto ORANGE    = "#C2410C";
    constexpr auto TH_BG     = "#1E3A5F";
    constexpr auto TH_TXT    = "#FFFFFF";
}

static QGraphicsDropShadowEffect* mkShadow()
{
    auto *e = new QGraphicsDropShadowEffect;
    e->setBlurRadius(16);
    e->setOffset(0, 3);
    e->setColor(QColor(0, 0, 0, 22));
    return e;
}

// ── ORDRE D'INITIALISATION CORRECT (match l'ordre dans le header) ───────────
AdminWindow::AdminWindow(int adminId, QWidget *parent)
    : QWidget(parent),
      m_adminId(adminId),           // 1
      m_dashboard(nullptr),         // 2
      m_pageStack(nullptr),         // 3
      m_articles(nullptr),            // 4
      m_invoicePage(nullptr),       // 5
      m_clientPage(nullptr),          // 6
      m_entrepriseConfig(nullptr),    // 7 ← AVANT m_invoiceDialog
      m_invoiceDialog(nullptr),       // 8 ← APRÈS m_entrepriseConfig
      invoiceView(nullptr),
      invoiceModel(nullptr),
      invoiceSearchEdit(nullptr),
      invoiceSearchBtn(nullptr),
      createInvoiceBtn(nullptr),
      editInvoiceBtn(nullptr),
      deleteInvoiceBtn(nullptr),
      actionsBtn(nullptr),
  
      refreshInvoicesBtn(nullptr),
      clientView(nullptr),
      clientModel(nullptr),
      searchEdit(nullptr),
      searchButton(nullptr),
      addButton(nullptr),
      editButton(nullptr),
      deleteButton(nullptr),
      refreshButton(nullptr)
{
    setupUI();
}

AdminWindow::~AdminWindow() {}

void AdminWindow::setupUI()
{
    setStyleSheet(QString("background:%1;").arg(A::BG));

    QHBoxLayout *root = new QHBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

    m_dashboard = new DashboardWidget(this);

    m_pageStack = new QStackedWidget;
    m_pageStack->setStyleSheet(QString("background:%1;").arg(A::BG));

    // Pages
    m_pageStack->addWidget(m_dashboard->contentArea()); // index 0

    m_articles = new ArticlesWidget(this);
    m_pageStack->addWidget(m_articles);                 // index 1

    m_invoicePage = buildInvoicePage();
    m_pageStack->addWidget(m_invoicePage);              // index 2

    m_clientPage  = buildClientPage();
    m_pageStack->addWidget(m_clientPage);               // index 3

    m_entrepriseConfig = new EntrepriseConfigWidget(this);
    m_pageStack->addWidget(m_entrepriseConfig);         // index 4

    m_pageStack->setCurrentIndex(0);

    QWidget *sidebar = m_dashboard->sidebarOnly();
sidebar->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);  // ← AJOUTER
root->addWidget(sidebar);
root->addWidget(m_pageStack, 1);

    // Signals
    connect(m_dashboard, &DashboardWidget::navigateTo,
            this, &AdminWindow::onNavigateTo);
    connect(m_dashboard, &DashboardWidget::logoutRequested,
            this, &AdminWindow::onLogout);

    connect(m_entrepriseConfig, &EntrepriseConfigWidget::configSaved,
            this, &AdminWindow::onConfigSaved);
    connect(m_entrepriseConfig, &EntrepriseConfigWidget::backToDashboard,
            [this]() { m_pageStack->setCurrentIndex(0); });

    if (!m_invoiceDialog)
        m_invoiceDialog = new InvoiceCreateDialog(-1, this);
    connect(m_articles, &ArticlesWidget::articleSelected,
            m_invoiceDialog, &InvoiceCreateDialog::onArticleFromCatalog);
}

void AdminWindow::onNavigateTo(const QString &page)
{
    if      (page == "dashboard")         m_pageStack->setCurrentIndex(0);
    else if (page == "articles")          m_pageStack->setCurrentIndex(1);
    else if (page == "factures")          m_pageStack->setCurrentIndex(2);
    else if (page == "clients")           m_pageStack->setCurrentIndex(3);
    else if (page == "entreprise_config") m_pageStack->setCurrentIndex(4);
}

void AdminWindow::onConfigSaved(const EntrepriseConfig &cfg)
{
    Q_UNUSED(cfg)
    m_pageStack->setCurrentIndex(0);
}

QWidget* AdminWindow::buildInvoicePage()
{
    QWidget *page = new QWidget;
    page->setStyleSheet(QString("background:%1;").arg(A::BG));

    QVBoxLayout *vl = new QVBoxLayout(page);
    vl->setContentsMargins(28, 22, 28, 20);
    vl->setSpacing(16);

    QLabel *ttl = new QLabel("🧾  Gestion des Factures");
    ttl->setStyleSheet(
        QString("font-family:'Segoe UI Semibold','SF Pro Display',sans-serif;"
                "font-size:20px;font-weight:700;color:%1;").arg(A::TXT_HEAD));

    QLabel *sub = new QLabel("Créez, modifiez et suivez vos factures");
    sub->setStyleSheet(QString("font-size:12px;color:%1;").arg(A::TXT_SUB));

    vl->addWidget(ttl);
    vl->addWidget(sub);

    QFrame *card = new QFrame;
    card->setStyleSheet(
        QString("QFrame{background:%1;border-radius:14px;border:1px solid %2;}")
            .arg(A::CARD, A::BORDER));
    card->setGraphicsEffect(mkShadow());

    QVBoxLayout *cl = new QVBoxLayout(card);
    cl->setContentsMargins(20, 16, 20, 16);
    cl->setSpacing(12);

    QHBoxLayout *toolbar = new QHBoxLayout;
    toolbar->setSpacing(8);

    invoiceSearchEdit = new QLineEdit;
    invoiceSearchEdit->setPlaceholderText("🔍  Rechercher par N° facture, client…");
    invoiceSearchEdit->setStyleSheet(
        "padding:6px 10px;border:1px solid #CBD5E0;border-radius:7px;"
        "font-size:12px;background:white;");
    invoiceSearchEdit->setFixedHeight(34);

    invoiceSearchBtn = new QPushButton("Rechercher");
    QPushButton *clearBtn = new QPushButton("✕");

    auto makeBtn = [](const QString &label, const QString &color,
                      int minW = 80) -> QPushButton* {
        QPushButton *b = new QPushButton(label);
        b->setFixedHeight(34);
        b->setMinimumWidth(minW);
        b->setCursor(Qt::PointingHandCursor);
        b->setStyleSheet(
            QString("QPushButton{background:%1;color:white;font-weight:600;"
                    "border:none;border-radius:7px;padding:0 12px;font-size:12px;}"
                    "QPushButton:hover{opacity:0.9;}").arg(color));
        return b;
    };

    invoiceSearchBtn  = makeBtn("Rechercher",    A::BLUE,   100);
    clearBtn          = makeBtn("✕ Effacer",     A::GRAY,    80);

    toolbar->addWidget(invoiceSearchEdit, 1);
    toolbar->addWidget(invoiceSearchBtn);
    toolbar->addWidget(clearBtn);

    QHBoxLayout *actions = new QHBoxLayout;
    actions->setSpacing(8);

    createInvoiceBtn  = makeBtn("➕ Créer",     A::GREEN,  100);
    editInvoiceBtn    = makeBtn("✏️ Modifier",   A::BLUE,   100);
    deleteInvoiceBtn  = makeBtn("🗑️ Supprimer", A::RED,    100);
    actionsBtn        = makeBtn("⚙️ Actions",   A::AMBER,  100);
    refreshInvoicesBtn= makeBtn("🔄",           A::GRAY,    42);

    // ⬇️ BOUTON EXPORT PDF AJOUTÉ
    QPushButton *exportPdfBtn = makeBtn("📄 PDF", A::ORANGE, 90);

    actions->addWidget(createInvoiceBtn);
    actions->addWidget(editInvoiceBtn);
    actions->addWidget(deleteInvoiceBtn);
    actions->addWidget(actionsBtn);
 
    actions->addWidget(exportPdfBtn);        // ⬅️ AJOUTÉ
    actions->addStretch();
    actions->addWidget(refreshInvoicesBtn);

    cl->addLayout(toolbar);
    cl->addLayout(actions);

    invoiceModel = new QSqlQueryModel(this);
    invoiceModel->setQuery(
        "SELECT f.id, f.numero, f.type, "
        "COALESCE(f.client_nom, c.nom || ' ' || c.prenom, 'N/A') as Client, "
        "f.date_creation, f.date_echeance, "
        "f.total_ht, f.total_tva, f.total_ttc, f.statut "
        "FROM factures f LEFT JOIN clients c ON f.client_id = c.id "
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
    invoiceView->setColumnHidden(0, true);
    invoiceView->setStyleSheet(
        QString("QTableView{"
                "  border:none;gridline-color:#F1F5F9;"
                "  selection-background-color:#DBEAFE;"
                "  selection-color:#1E3A5F;"
                "  font-size:12px;"
                "}"
                "QHeaderView::section{"
                "  background:%1;color:%2;"
                "  font-weight:700;padding:8px 6px;"
                "  border:none;font-size:11px;"
                "}"
                "QTableView::item{padding:6px;border-bottom:1px solid #F1F5F9;}"
               ).arg(A::TH_BG, A::TH_TXT));

    cl->addWidget(invoiceView, 1);
    vl->addWidget(card, 1);

    connect(invoiceSearchBtn, &QPushButton::clicked,
            this, &AdminWindow::onSearchInvoice);
    connect(invoiceSearchEdit, &QLineEdit::returnPressed,
            this, &AdminWindow::onSearchInvoice);
    connect(clearBtn, &QPushButton::clicked, this, [this]{
        invoiceSearchEdit->clear(); onRefreshInvoices();
    });
    connect(createInvoiceBtn,  &QPushButton::clicked, this, &AdminWindow::onCreateInvoice);
    connect(editInvoiceBtn,    &QPushButton::clicked, this, &AdminWindow::onEditInvoice);
    connect(deleteInvoiceBtn,  &QPushButton::clicked, this, &AdminWindow::onDeleteInvoice);
    connect(actionsBtn,        &QPushButton::clicked, this, &AdminWindow::onInvoiceActions);
    connect(refreshInvoicesBtn,&QPushButton::clicked, this, &AdminWindow::onRefreshInvoices);

    // ⬇️ CONNEXION DU BOUTON EXPORT PDF
    connect(exportPdfBtn, &QPushButton::clicked, this, &AdminWindow::onExportInvoicePDF);

    return page;
}

QWidget* AdminWindow::buildClientPage()
{
    QWidget *page = new QWidget;
    page->setStyleSheet(QString("background:%1;").arg(A::BG));

    QVBoxLayout *vl = new QVBoxLayout(page);
    vl->setContentsMargins(28, 22, 28, 20);
    vl->setSpacing(16);

    QLabel *ttl = new QLabel("👥  Gestion des Clients");
    ttl->setStyleSheet(
        QString("font-family:'Segoe UI Semibold','SF Pro Display',sans-serif;"
                "font-size:20px;font-weight:700;color:%1;").arg(A::TXT_HEAD));
    QLabel *sub = new QLabel("Gérez votre portefeuille clients");
    sub->setStyleSheet(QString("font-size:12px;color:%1;").arg(A::TXT_SUB));

    vl->addWidget(ttl);
    vl->addWidget(sub);

    QFrame *card = new QFrame;
    card->setStyleSheet(
        QString("QFrame{background:%1;border-radius:14px;border:1px solid %2;}")
            .arg(A::CARD, A::BORDER));
    card->setGraphicsEffect(mkShadow());

    QVBoxLayout *cl = new QVBoxLayout(card);
    cl->setContentsMargins(20, 16, 20, 16);
    cl->setSpacing(12);

    auto makeBtn = [](const QString &label, const QString &color,
                      int minW = 80) -> QPushButton* {
        QPushButton *b = new QPushButton(label);
        b->setFixedHeight(34);
        b->setMinimumWidth(minW);
        b->setCursor(Qt::PointingHandCursor);
        b->setStyleSheet(
            QString("QPushButton{background:%1;color:white;font-weight:600;"
                    "border:none;border-radius:7px;padding:0 12px;font-size:12px;}"
                    "QPushButton:hover{opacity:0.9;}").arg(color));
        return b;
    };

    QHBoxLayout *searchRow = new QHBoxLayout;
    searchRow->setSpacing(8);
    searchEdit = new QLineEdit;
    searchEdit->setPlaceholderText("🔍  Rechercher par prénom…");
    searchEdit->setStyleSheet(
        "padding:6px 10px;border:1px solid #CBD5E0;border-radius:7px;"
        "font-size:12px;background:white;");
    searchEdit->setFixedHeight(34);
    searchButton = makeBtn("Rechercher", A::BLUE, 100);
    QPushButton *resetBtn = makeBtn("✕ Réinitialiser", A::GRAY, 110);
    searchRow->addWidget(searchEdit, 1);
    searchRow->addWidget(searchButton);
    searchRow->addWidget(resetBtn);

    QHBoxLayout *actRow = new QHBoxLayout;
    actRow->setSpacing(8);
    addButton     = makeBtn("➕ Ajouter",   A::GREEN,  100);
    editButton    = makeBtn("✏️ Modifier",   A::BLUE,   100);
    deleteButton  = makeBtn("🗑️ Supprimer", A::RED,    100);
   
    actRow->addWidget(addButton);
    actRow->addWidget(editButton);
    actRow->addWidget(deleteButton);
    actRow->addStretch();

    cl->addLayout(searchRow);
    cl->addLayout(actRow);

    clientModel = new QSqlTableModel(this);
    clientModel->setTable("clients");
    clientModel->setEditStrategy(QSqlTableModel::OnManualSubmit);
    clientModel->select();

    clientView = new QTableView;
    clientView->setModel(clientModel);
    clientView->setSelectionBehavior(QAbstractItemView::SelectRows);
    clientView->setSelectionMode(QAbstractItemView::SingleSelection);
    clientView->horizontalHeader()->setStretchLastSection(true);
    clientView->setColumnHidden(4,  true);
    clientView->setColumnHidden(10, true);
    clientView->setColumnHidden(12, true);
    clientView->setColumnHidden(13, true);
    clientView->verticalHeader()->setVisible(false);
    clientView->setAlternatingRowColors(true);
    clientView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    clientView->setStyleSheet(
        QString("QTableView{"
                "  border:none;gridline-color:#F1F5F9;"
                "  selection-background-color:#DBEAFE;"
                "  selection-color:#1E3A5F;"
                "  font-size:12px;"
                "}"
                "QHeaderView::section{"
                "  background:%1;color:%2;"
                "  font-weight:700;padding:8px 6px;"
                "  border:none;font-size:11px;"
                "}"
                "QTableView::item{padding:6px;border-bottom:1px solid #F1F5F9;}"
               ).arg(A::TH_BG, A::TH_TXT));

    cl->addWidget(clientView, 1);
    vl->addWidget(card, 1);

    connect(searchButton, &QPushButton::clicked, this, &AdminWindow::onSearch);
    connect(searchEdit,   &QLineEdit::returnPressed, this, &AdminWindow::onSearch);
    connect(resetBtn, &QPushButton::clicked, this, [this]{
        searchEdit->clear(); refreshModel();
    });
    connect(addButton,    &QPushButton::clicked, this, &AdminWindow::onAddClient);
    connect(editButton,   &QPushButton::clicked, this, &AdminWindow::onEditClient);
    connect(deleteButton, &QPushButton::clicked, this, &AdminWindow::onDeleteClient);
    connect(refreshButton,&QPushButton::clicked, this, &AdminWindow::refreshModel);

    return page;
}

void AdminWindow::onSearchInvoice()
{
    QString s = invoiceSearchEdit->text().trimmed();
    if (s.isEmpty()) { onRefreshInvoices(); return; }
    invoiceModel->setQuery(QString(
        "SELECT f.id, f.numero, f.type, "
        "COALESCE(f.client_nom, c.nom || ' ' || c.prenom, 'N/A') as Client, "
        "f.date_creation, f.date_echeance, "
        "f.total_ht, f.total_tva, f.total_ttc, f.statut "
        "FROM factures f LEFT JOIN clients c ON f.client_id = c.id "
        "WHERE f.numero LIKE '%%1%' OR f.client_nom LIKE '%%1%' "
        "OR c.nom LIKE '%%1%' ORDER BY f.id DESC").arg(s));
}

void AdminWindow::onCreateInvoice()
{
    InvoiceCreateDialog dlg(-1, this);
    dlg.showMaximized();
    if (dlg.exec() == QDialog::Accepted) onRefreshInvoices();
}

void AdminWindow::onEditInvoice()
{
    int row = invoiceView->currentIndex().row();
    if (row < 0) { QMessageBox::warning(this,"Sélection","Sélectionnez une facture."); return; }
    int id = invoiceModel->data(invoiceModel->index(row, 0)).toInt();
    InvoiceEditDialog *dlg = new InvoiceEditDialog(id, this);
    if (dlg->exec() == QDialog::Accepted) onRefreshInvoices();
    delete dlg;
}

void AdminWindow::onDeleteInvoice()
{
    int row = invoiceView->currentIndex().row();
    if (row < 0) { QMessageBox::warning(this,"Sélection","Sélectionnez une facture."); return; }
    int id = invoiceModel->data(invoiceModel->index(row, 0)).toInt();
    if (QMessageBox::question(this,"Confirmation","Supprimer cette facture ?",
            QMessageBox::Yes|QMessageBox::No) == QMessageBox::Yes) {
        QSqlQuery q;
        q.prepare("DELETE FROM lignes_facture WHERE facture_id = ?");
        q.addBindValue(id); q.exec();
        q.prepare("DELETE FROM factures WHERE id = ?");
        q.addBindValue(id);
        if (q.exec()) { onRefreshInvoices(); QMessageBox::information(this,"Succès","Facture supprimée."); }
    }
}

void AdminWindow::onInvoiceActions()
{
    int row = invoiceView->currentIndex().row();
    if (row < 0) { QMessageBox::warning(this,"Sélection","Sélectionnez une facture."); return; }
    int id = invoiceModel->data(invoiceModel->index(row, 0)).toInt();
    InvoiceActionDialog dlg(id, this);
    dlg.exec();
    onRefreshInvoices();
}

void AdminWindow::onPaymentClicked()
{
    int row = invoiceView->currentIndex().row();
    if (row < 0) { QMessageBox::warning(this,"Sélection","Sélectionnez une facture."); return; }
    int id = invoiceModel->data(invoiceModel->index(row, 0)).toInt();
    PaymentDialog *dlg = new PaymentDialog(id, this);
    dlg->exec(); delete dlg;
    onRefreshInvoices();
}

void AdminWindow::onRefreshInvoices()
{
    delete invoiceModel;
    invoiceModel = new QSqlQueryModel(this);
    invoiceModel->setQuery(
        "SELECT f.id, f.numero, f.type, "
        "COALESCE(f.client_nom, c.nom || ' ' || c.prenom, 'N/A') as Client, "
        "f.date_creation, f.date_echeance, "
        "f.total_ht, f.total_tva, f.total_ttc, f.statut "
        "FROM factures f LEFT JOIN clients c ON f.client_id = c.id "
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
    invoiceView->setModel(invoiceModel);
    invoiceView->setSelectionBehavior(QAbstractItemView::SelectRows);
    invoiceView->horizontalHeader()->setStretchLastSection(true);
}

void AdminWindow::refreshModel()
{
    clientModel->setFilter(""); clientModel->select();
}

void AdminWindow::onSearch()
{
    QString p = searchEdit->text().trimmed();
    clientModel->setFilter(p.isEmpty()
        ? "role = 'client'"
        : QString("role = 'client' AND prenom LIKE '%%1%'").arg(p));
    clientModel->select();
}

void AdminWindow::onAddClient()
{
    ClientEditDialog dlg(this);
    if (dlg.exec() == QDialog::Accepted) {
        refreshModel();
        QMessageBox::information(this,"Succès","Client ajouté !");
    }
}

void AdminWindow::onEditClient()
{
    int row = clientView->currentIndex().row();
    if (row < 0) { QMessageBox::warning(this,"Sélection","Sélectionnez un client."); return; }
    int id = clientModel->data(clientModel->index(row, 0)).toInt();
    ClientEditDialog dlg(this);
    dlg.setClientId(id);
    if (dlg.exec() == QDialog::Accepted) refreshModel();
}

void AdminWindow::onDeleteClient()
{
    int row = clientView->currentIndex().row();
    if (row < 0) { QMessageBox::warning(this,"Sélection","Sélectionnez un client."); return; }
    int id = clientModel->data(clientModel->index(row, 0)).toInt();
    if (QMessageBox::question(this,"Confirmation","Supprimer ce client ?",
            QMessageBox::Yes|QMessageBox::No) == QMessageBox::Yes) {
        QSqlQuery q;
        q.prepare("DELETE FROM clients WHERE id = ?");
        q.addBindValue(id);
        if (!q.exec())
            QMessageBox::critical(this,"Erreur","Échec : "+q.lastError().text());
        else refreshModel();
    }
}

void AdminWindow::onLogout()
{
    if (QMessageBox::question(this,"Déconnexion","Se déconnecter ?",
            QMessageBox::Yes|QMessageBox::No) == QMessageBox::Yes)
        emit logoutRequested();
}
void AdminWindow::onExportInvoicePDF()
{
    int row = invoiceView->currentIndex().row();
    if (row < 0) {
        QMessageBox::warning(this, "Sélection",
                             "Sélectionnez une facture à exporter.");
        return;
    }
    int invoiceId = invoiceModel->data(invoiceModel->index(row, 0)).toInt();
    QString numeroFacture = invoiceModel->data(invoiceModel->index(row, 1)).toString();

    qDebug() << "=== EXPORT PDF ===";
    qDebug() << "Invoice ID:" << invoiceId;
    qDebug() << "Numéro:" << numeroFacture;

    // Avant d'appeler generatePDF, construisez le style :
    InvoiceStyle style;
    
    // Valeurs par défaut
    style.primaryColor  = "#4db8e8";
    style.logoPath      = "";
    style.signaturePath = "";
    style.companyName   = "Votre Entreprise";
    style.companyICE    = "-";
    style.companyPhone  = "";
    style.companyEmail  = "";
    style.companyWebsite= "";

    EntrepriseConfigManager *mgr = EntrepriseConfigManager::instance();
    qDebug() << "Manager ptr:" << mgr;
    qDebug() << "Is configured:" << mgr->isConfigured();

    if (mgr->isConfigured()) {
        EntrepriseConfig cfg = mgr->loadConfig();
        qDebug() << "Config chargée:";
        qDebug() << "  nom:" << cfg.nom;
        qDebug() << "  rib:" << cfg.rib;
        qDebug() << "  themeCouleur:" << cfg.themeCouleur.name();
        qDebug() << "  logoPath:" << cfg.logoPath;
        qDebug() << "  signaturePath:" << cfg.signaturePath;
        qDebug() << "  configured:" << cfg.configured;

        if (!cfg.nom.isEmpty())        style.companyName    = cfg.nom;
        if (!cfg.rib.isEmpty())        style.companyICE     = cfg.rib;
        if (!cfg.logoPath.isEmpty())   style.logoPath       = cfg.logoPath;
        if (!cfg.signaturePath.isEmpty()) style.signaturePath = cfg.signaturePath;
        if (cfg.themeCouleur.isValid()) style.primaryColor  = cfg.themeCouleur.name();
    } else {
        qDebug() << "WARNING: Aucune config entreprise trouvée!";
        QMessageBox::warning(this, "Configuration manquante",
            "Aucune configuration entreprise trouvée.\n"
            "Le PDF sera généré avec les valeurs par défaut.\n\n"
            "Veuillez configurer votre entreprise dans Paramètres > Entreprise.");
    }

    qDebug() << "Style final:";
    qDebug() << "  primaryColor:" << style.primaryColor;
    qDebug() << "  logoPath:" << style.logoPath;
    qDebug() << "  signaturePath:" << style.signaturePath;
    qDebug() << "  companyName:" << style.companyName;
    qDebug() << "  companyICE:" << style.companyICE;

    // Choix du chemin de sauvegarde
    QString defaultPath = InvoiceGenerator::getPdfOutputPath() + "/" 
                        + InvoiceGenerator::getInvoiceFileName(numeroFacture);
    
    QString filePath = QFileDialog::getSaveFileName(
        this,
        "Exporter la facture en PDF",
        defaultPath,
        "Fichiers PDF (*.pdf)");

    qDebug() << "Chemin choisi:" << filePath;

    if (filePath.isEmpty()) {
        qDebug() << "Export annulé par l'utilisateur";
        return;
    }

    if (!filePath.endsWith(".pdf", Qt::CaseInsensitive))
        filePath += ".pdf";

    InvoiceGenerator gen;
    bool ok = gen.generatePDF(invoiceId, filePath, style);
    qDebug() << "Résultat generatePDF:" << ok;

    if (ok) {
        QMessageBox::information(this, "Succès",
            QString("PDF exporté avec succès !\n\n%1").arg(filePath));
        QDesktopServices::openUrl(QUrl::fromLocalFile(filePath));
    } else {
        QMessageBox::critical(this, "Erreur",
            "Échec de la génération du PDF.\nVérifiez la console pour plus de détails.");
    }
}