#include "clientwindow.h"
#include "mainwindow.h"
#include "utils/invoicegenerator.h"
#include "utils/emailsender.h"
#include "dialogs/logindialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QFormLayout>
#include <QMessageBox>
#include <QScrollArea>
#include <QFrame>
#include <QSqlQuery>
#include <QSqlError>
#include <QDate>
#include <QHeaderView>
#include <QDesktopServices>
#include <QUrl>
#include <QCryptographicHash>
#include <QDebug>
#include <QMenuBar>
#include <QStatusBar>
#include <QProgressBar>

ClientWindow::ClientWindow(int clientId, QWidget *parent)
    : QMainWindow(parent), m_clientId(clientId)
{
    loadClientInfo();
    setupUI();
    refreshDashboard();
}

void ClientWindow::loadClientInfo()
{
    QSqlQuery q;
    q.prepare("SELECT nom, prenom, email, telephone, "
              "adresse FROM clients WHERE id = ?");
    q.addBindValue(m_clientId);
    if (q.exec() && q.next()) {
        m_clientNom   = q.value(0).toString() + " " +
                        q.value(1).toString();
        m_clientEmail = q.value(2).toString();
    }
}

void ClientWindow::setupUI()
{
    setWindowTitle("👤 Espace Client — " + m_clientNom);
    setMinimumSize(1000, 700);
    resize(1100, 750);

    setStyleSheet(
        "QMainWindow { background:#F7FAFC; }"
        "QTabWidget::pane {"
        "  border:1px solid #E2E8F0;background:white;"
        "  border-radius:0 0 8px 8px;"
        "}"
        "QTabBar::tab {"
        "  padding:10px 20px;font-size:12px;"
        "  font-weight:bold;color:#718096;"
        "  border:1px solid #E2E8F0;"
        "  border-bottom:none;border-radius:6px 6px 0 0;"
        "  background:#F7FAFC;margin-right:2px;"
        "}"
        "QTabBar::tab:selected {"
        "  color:#2B6CB0;background:white;"
        "  border-bottom:2px solid #2B6CB0;"
        "}"
        "QGroupBox {"
        "  font-weight:bold;font-size:12px;"
        "  border:1px solid #E2E8F0;border-radius:8px;"
        "  margin-top:8px;padding-top:10px;"
        "  background:white;color:#2B6CB0;"
        "}"
        "QTableView {"
        "  border:1px solid #E2E8F0;"
        "  gridline-color:#EDF2F7;"
        "  selection-background-color:#BEE3F8;"
        "  selection-color:#2D3748;background:white;"
        "}"
        "QHeaderView::section {"
        "  background:#2B6CB0;color:white;"
        "  font-weight:bold;padding:8px;border:none;"
        "}"
        "QLineEdit, QComboBox, QDateEdit, QTextEdit {"
        "  border:1px solid #CBD5E0;border-radius:4px;"
        "  padding:6px;background:white;min-height:28px;"
        "}"
        "QLineEdit:focus { border:1px solid #3182CE; }"
        "QPushButton {"
        "  border-radius:4px;border:none;"
        "  font-weight:bold;padding:7px 16px;"
        "}"
    );

    // Barre de menu
    QMenuBar *menuBar = new QMenuBar(this);
    QLabel *userLabel = new QLabel(
        "  👤 " + m_clientNom + "  ");
    userLabel->setStyleSheet(
        "color:#2B6CB0;font-weight:bold;padding:4px;");
    menuBar->setCornerWidget(userLabel);
    setMenuBar(menuBar);

    // Widget central
    QWidget *central = new QWidget(this);
    setCentralWidget(central);
    QVBoxLayout *mainLayout = new QVBoxLayout(central);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    // Bandeau haut
    QFrame *header = new QFrame;
    header->setFixedHeight(60);
    header->setStyleSheet(
        "QFrame { background:#1B2A3B; }");
    QHBoxLayout *headerLayout = new QHBoxLayout(header);
    QLabel *appTitle = new QLabel(
        "🏢 FacturationApp — Espace Client");
    appTitle->setStyleSheet(
        "color:white;font-size:16px;font-weight:bold;");
    QPushButton *logoutBtn = new QPushButton(
        "🚪 Déconnexion");
    logoutBtn->setStyleSheet(
        "background:#3182CE;color:white;padding:6px 14px;");
    headerLayout->addWidget(appTitle);
    headerLayout->addStretch();
    headerLayout->addWidget(logoutBtn);
    mainLayout->addWidget(header);

    // Onglets
    tabWidget = new QTabWidget;
    tabWidget->setContentsMargins(8, 8, 8, 8);
    mainLayout->addWidget(tabWidget);

    setupDashboard();
    setupFactures();
    setupPaiements();
    setupProfil();
    setupContact();

    connect(logoutBtn, &QPushButton::clicked,
            this, &ClientWindow::onLogout);

    statusBar()->showMessage(
        "Connecté en tant que: " + m_clientNom);
}

// ═══════════════════════════════════════════════════════
// DASHBOARD
// ═══════════════════════════════════════════════════════
void ClientWindow::setupDashboard()
{
    QScrollArea *scroll = new QScrollArea;
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);

    QWidget *container = new QWidget;
    QVBoxLayout *layout = new QVBoxLayout(container);
    layout->setSpacing(14);
    layout->setContentsMargins(14, 14, 14, 14);

    // Titre + bouton refresh
    QHBoxLayout *titleRow = new QHBoxLayout;
    QLabel *title = new QLabel("📊 Mon Tableau de Bord");
    title->setStyleSheet(
        "font-size:18px;font-weight:bold;color:#1B2A3B;");
    QPushButton *refreshBtn = new QPushButton(
        "🔄 Actualiser");
    refreshBtn->setStyleSheet(
        "background:#3182CE;color:white;");
    titleRow->addWidget(title);
    titleRow->addStretch();
    titleRow->addWidget(refreshBtn);
    layout->addLayout(titleRow);

    // Cartes statistiques
    QWidget *cardsWidget = new QWidget;
    QGridLayout *cardsGrid = new QGridLayout(cardsWidget);
    cardsGrid->setSpacing(12);

    auto makeCard = [](const QString &icon,
                       const QString &title,
                       const QString &color,
                       QLabel *&valueLabel) -> QFrame* {
        QFrame *card = new QFrame;
        card->setMinimumHeight(100);
        card->setStyleSheet(QString(
            "QFrame {"
            "  background:white;"
            "  border-radius:10px;"
            "  border:1px solid #E2E8F0;"
            "  border-left:5px solid %1;"
            "}"
        ).arg(color));

        QVBoxLayout *vl = new QVBoxLayout(card);
        vl->setContentsMargins(14, 10, 14, 10);

        QLabel *titleLbl = new QLabel(icon + "  " + title);
        titleLbl->setStyleSheet(
            "font-size:10px;color:#718096;"
            "font-weight:bold;text-transform:uppercase;"
            "letter-spacing:1px;border:none;");

        valueLabel = new QLabel("--");
        valueLabel->setStyleSheet(QString(
            "font-size:22px;font-weight:900;"
            "color:%1;border:none;").arg(color));

        vl->addWidget(titleLbl);
        vl->addWidget(valueLabel);
        return card;
    };

    cardsGrid->addWidget(
        makeCard("📄","Total Factures","#2B6CB0",
                 statTotalFactures), 0, 0);
    cardsGrid->addWidget(
        makeCard("💰","Montant Total Dû","#E53E3E",
                 statMontantDu), 0, 1);
    cardsGrid->addWidget(
        makeCard("✅","Total Payé","#27AE60",
                 statTotalPaye), 0, 2);
    cardsGrid->addWidget(
        makeCard("📅","Factures ce Mois","#DD6B20",
                 statFacturesMois), 1, 0);
    cardsGrid->addWidget(
        makeCard("🕐","Dernière Facture","#805AD5",
                 statDerniereFacture), 1, 1);
    cardsGrid->addWidget(
        makeCard("🏷️","Statut Compte","#3182CE",
                 statStatutCompte), 1, 2);

    layout->addWidget(cardsWidget);

    // Dernières factures
    QGroupBox *recentGroup = new QGroupBox(
        "📋 Dernières Factures");
    QVBoxLayout *recentLayout =
        new QVBoxLayout(recentGroup);

    recentModel = new QSqlQueryModel(this);
    recentTable = new QTableView;
    recentTable->setModel(recentModel);
    recentTable->setMaximumHeight(200);
    recentTable->verticalHeader()->setVisible(false);
    recentTable->horizontalHeader()
               ->setStretchLastSection(true);
    recentTable->setEditTriggers(
        QAbstractItemView::NoEditTriggers);
    recentTable->setSelectionBehavior(
        QAbstractItemView::SelectRows);
    recentTable->setAlternatingRowColors(true);
    recentLayout->addWidget(recentTable);
    layout->addWidget(recentGroup);

    scroll->setWidget(container);
    tabWidget->addTab(scroll, "🏠 Accueil");

    connect(refreshBtn, &QPushButton::clicked,
            this, &ClientWindow::refreshDashboard);
}

void ClientWindow::refreshDashboard()
{
    QSqlQuery q;

    // Total factures
    q.prepare("SELECT COUNT(*) FROM factures "
              "WHERE client_id = ?");
    q.addBindValue(m_clientId);
    q.exec();
    statTotalFactures->setText(
        q.next() ?
        QString::number(q.value(0).toInt()) : "0");

    // Montant total dû — calculé depuis paiements
    q.prepare(
        "SELECT "
        "COALESCE(SUM(f.total_ttc), 0) - "
        "COALESCE(SUM(p.montant), 0) "
        "FROM factures f "
        "LEFT JOIN paiements p ON p.facture_id = f.id "
        "WHERE f.client_id = ?");
    q.addBindValue(m_clientId);
    q.exec();
    double montantDu = q.next() ?
        qMax(0.0, q.value(0).toDouble()) : 0;
    statMontantDu->setText(
        QString::number(montantDu, 'f', 2) + " MAD");

    // Total payé — somme des paiements
    q.prepare(
        "SELECT COALESCE(SUM(p.montant), 0) "
        "FROM paiements p "
        "JOIN factures f ON p.facture_id = f.id "
        "WHERE f.client_id = ?");
    q.addBindValue(m_clientId);
    q.exec();
    statTotalPaye->setText(
        q.next() ?
        QString::number(q.value(0).toDouble(),
                        'f', 2) + " MAD" : "0 MAD");

    // Factures ce mois
    QString mois = QDate::currentDate()
                   .toString("yyyy-MM");
    q.prepare(
        "SELECT COUNT(*) FROM factures "
        "WHERE client_id = ? AND "
        "strftime('%Y-%m', date_creation) = ?");
    q.addBindValue(m_clientId);
    q.addBindValue(mois);
    q.exec();
    statFacturesMois->setText(
        q.next() ?
        QString::number(q.value(0).toInt()) : "0");

    // Dernière facture
    q.prepare(
        "SELECT numero FROM factures "
        "WHERE client_id = ? "
        "ORDER BY date_creation DESC LIMIT 1");
    q.addBindValue(m_clientId);
    q.exec();
    statDerniereFacture->setText(
        q.next() ?
        "N°" + q.value(0).toString() : "Aucune");

    // Statut compte
    q.prepare(
        "SELECT COUNT(*) FROM factures "
        "WHERE client_id = ? AND "
        "statut NOT IN ('Payée','Annulée')");
    q.addBindValue(m_clientId);
    q.exec();
    int impayees = q.next() ?
                   q.value(0).toInt() : 0;
    statStatutCompte->setText(
        impayees == 0 ? "✅ À jour" :
        QString("⚠️ %1 en attente").arg(impayees));

    // Tableau dernières factures
    recentModel->setQuery(QString(
        "SELECT "
        "f.numero AS 'N°', "
        "date(f.date_creation) AS 'Date', "
        "f.total_ttc AS 'Total TTC', "
        "COALESCE(SUM(p.montant), 0) AS 'Payé', "
        "f.total_ttc - COALESCE(SUM(p.montant),0) "
        "  AS 'Reste', "
        "f.statut AS 'Statut' "
        "FROM factures f "
        "LEFT JOIN paiements p ON p.facture_id = f.id "
        "WHERE f.client_id = %1 "
        "GROUP BY f.id "
        "ORDER BY f.date_creation DESC LIMIT 5"
    ).arg(m_clientId));

    recentTable->resizeColumnsToContents();
    recentTable->horizontalHeader()
               ->setStretchLastSection(true);
}

// ═══════════════════════════════════════════════════════
// FACTURES
// ═══════════════════════════════════════════════════════
void ClientWindow::setupFactures()
{
    QScrollArea *scroll = new QScrollArea;
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);

    QWidget *widget = new QWidget;
    QVBoxLayout *layout = new QVBoxLayout(widget);
    layout->setSpacing(10);
    layout->setContentsMargins(14, 14, 14, 14);

    QLabel *title = new QLabel("📄 Mes Factures");
    title->setStyleSheet(
        "font-size:18px;font-weight:bold;color:#1B2A3B;");
    layout->addWidget(title);

    // Filtres
    QGroupBox *filterGroup = new QGroupBox("🔍 Filtres");
    QHBoxLayout *filterLayout = new QHBoxLayout(filterGroup);

    filterLayout->addWidget(new QLabel("Statut:"));
    filterStatut = new QComboBox;
    filterStatut->addItems({
        "Tous","Brouillon","Envoyée",
        "Partiellement payée","Payée","Annulée"});
    filterLayout->addWidget(filterStatut);

    filterLayout->addWidget(new QLabel("Du:"));
    filterDateDebut = new QDateEdit(
        QDate::currentDate().addMonths(-6));
    filterDateDebut->setCalendarPopup(true);
    filterDateDebut->setDisplayFormat("dd/MM/yyyy");
    filterLayout->addWidget(filterDateDebut);

    filterLayout->addWidget(new QLabel("Au:"));
    filterDateFin = new QDateEdit(
        QDate::currentDate().addDays(1));
    filterDateFin->setCalendarPopup(true);
    filterDateFin->setDisplayFormat("dd/MM/yyyy");
    filterLayout->addWidget(filterDateFin);

    filterLayout->addWidget(new QLabel("N°:"));
    searchFacture = new QLineEdit;
    searchFacture->setPlaceholderText("Numéro...");
    searchFacture->setMaximumWidth(120);
    filterLayout->addWidget(searchFacture);

    QPushButton *filterBtn = new QPushButton("🔍 Filtrer");
    filterBtn->setStyleSheet("background:#3182CE;color:white;");
    QPushButton *resetBtn = new QPushButton("✕ Reset");
    resetBtn->setStyleSheet("background:#718096;color:white;");
    filterLayout->addWidget(filterBtn);
    filterLayout->addWidget(resetBtn);
    layout->addWidget(filterGroup);

    // Tableau — hauteur fixe pas minimum
    facturesModel = new QSqlQueryModel(this);
    facturesTable = new QTableView;
    facturesTable->setModel(facturesModel);
    facturesTable->verticalHeader()->setVisible(false);
    facturesTable->horizontalHeader()
                 ->setStretchLastSection(true);
    facturesTable->setEditTriggers(
        QAbstractItemView::NoEditTriggers);
    facturesTable->setSelectionBehavior(
        QAbstractItemView::SelectRows);
    facturesTable->setAlternatingRowColors(true);
    facturesTable->setFixedHeight(280); // ← FIXE pas minimum
    layout->addWidget(facturesTable);

   // ── Boutons juste sous le tableau ─────────────────
QHBoxLayout *btnLayout = new QHBoxLayout;
btnLayout->setSpacing(10);
btnLayout->setContentsMargins(0, 6, 0, 6);

QPushButton *pdfBtn = new QPushButton(
    "📄 Télécharger PDF");
QPushButton *detailBtn = new QPushButton(
    "👁️ Voir Détails");

pdfBtn->setFixedHeight(36);
detailBtn->setFixedHeight(36);

pdfBtn->setStyleSheet(
    "background:#2B6CB0;color:white;"
    "font-size:12px;padding:0 16px;");
detailBtn->setStyleSheet(
    "background:#805AD5;color:white;"
    "font-size:12px;padding:0 16px;");

btnLayout->addWidget(pdfBtn);
btnLayout->addWidget(detailBtn);
btnLayout->addStretch();

// ← Ajouter DIRECTEMENT après le tableau
//   sans QGroupBox ni QScrollArea intermédiaire
layout->addLayout(btnLayout);   // ← ici, pas addWidget
layout->addStretch();           // ← pousse tout vers le haut

    scroll->setWidget(widget);
    tabWidget->addTab(scroll, "📄 Mes Factures");

    onResetFilter();

    connect(filterBtn,    &QPushButton::clicked,
            this, &ClientWindow::onFilterFactures);
    connect(resetBtn,     &QPushButton::clicked,
            this, &ClientWindow::onResetFilter);
    connect(pdfBtn,       &QPushButton::clicked,
            this, &ClientWindow::onDownloadPDF);
    connect(detailBtn,    &QPushButton::clicked,
            this, &ClientWindow::onViewDetails);
    connect(searchFacture,&QLineEdit::returnPressed,
            this, &ClientWindow::onFilterFactures);
}
void ClientWindow::onFilterFactures()
{
    QString where = QString(
        "WHERE client_id = %1").arg(m_clientId);

    QString statut = filterStatut->currentText();
    if (statut != "Tous")
        where += QString(
            " AND statut = '%1'").arg(statut);

    QString dateDebut = filterDateDebut->date()
                        .toString("yyyy-MM-dd");
    QString dateFin   = filterDateFin->date()
                        .toString("yyyy-MM-dd");
    where += QString(
        " AND date(date_creation) BETWEEN "
        "'%1' AND '%2'").arg(dateDebut, dateFin);

    QString search = searchFacture->text().trimmed();
    if (!search.isEmpty())
        where += QString(
            " AND numero LIKE '%%1%'").arg(search);

  facturesModel->setQuery(QString(
    "SELECT "
    "f.id AS 'ID', "
    "f.numero AS 'N° Facture', "
    "date(f.date_creation) AS 'Date', "
    "date(f.date_echeance) AS 'Échéance', "
    "f.total_ttc AS 'Total TTC', "
    "COALESCE(SUM(p.montant), 0) AS 'Payé', "
    "f.total_ttc - COALESCE(SUM(p.montant),0) AS 'Reste', "
    "f.statut AS 'Statut' "
    "FROM factures f "
    "LEFT JOIN paiements p ON p.facture_id = f.id "
    "%1 "
    "GROUP BY f.id "
    "ORDER BY f.date_creation DESC"
).arg(where));

    facturesTable->setColumnHidden(0, true);
    facturesTable->resizeColumnsToContents();
    facturesTable->horizontalHeader()
                 ->setStretchLastSection(true);
}

void ClientWindow::onResetFilter()
{
    filterStatut->setCurrentIndex(0);
    filterDateDebut->setDate(
        QDate::currentDate().addMonths(-6));
    filterDateFin->setDate(
        QDate::currentDate().addDays(1));
    searchFacture->clear();
    onFilterFactures();
}

void ClientWindow::onDownloadPDF()
{
    int row = facturesTable->currentIndex().row();
    if (row < 0) {
        QMessageBox::warning(this, "Sélection",
            "Sélectionnez une facture.");
        return;
    }
    int invoiceId = facturesModel->data(
        facturesModel->index(row, 0)).toInt();

    InvoiceStyle style;
    InvoiceGenerator gen;
    QString path = InvoiceGenerator::getPdfOutputPath()
                   + "/Facture_"
                   + QString::number(invoiceId)
                   + ".pdf";

    if (gen.generatePDF(invoiceId, path, style)) {
        QMessageBox::information(this, "✅ Succès",
            "PDF généré:\n" + path);
        QDesktopServices::openUrl(
            QUrl::fromLocalFile(path));
    } else {
        QMessageBox::critical(this, "Erreur",
            "Impossible de générer le PDF.");
    }
}

void ClientWindow::onViewDetails()
{
    int row = facturesTable->currentIndex().row();
    if (row < 0) {
        QMessageBox::warning(this, "Sélection",
            "Sélectionnez une facture.");
        return;
    }

    int invoiceId = facturesModel->data(
        facturesModel->index(row, 0)).toInt();
    QString numero = facturesModel->data(
        facturesModel->index(row, 1)).toString();

    // Récupérer les lignes de la facture
    QSqlQuery q;
    q.prepare(
        "SELECT designation, quantite, "
        "prix_unitaire_ht, taux_tva "
        "FROM lignes_facture WHERE facture_id = ?");
    q.addBindValue(invoiceId);

    QString details = QString(
        "<h3>Facture N° %1</h3>"
        "<table border='1' cellpadding='5' "
        "style='border-collapse:collapse;width:100%%;'>"
        "<tr style='background:#2B6CB0;color:white;'>"
        "<th>Désignation</th><th>Qté</th>"
        "<th>Prix HT</th><th>TVA</th><th>Total</th>"
        "</tr>"
    ).arg(numero);

    double total = 0;
    if (q.exec()) {
        while (q.next()) {
            double ht  = q.value(1).toInt() *
                         q.value(2).toDouble();
            double tva = q.value(3).toDouble();
            total += ht;
            details += QString(
                "<tr><td>%1</td><td>%2</td>"
                "<td>%3 MAD</td><td>%4%%</td>"
                "<td>%5 MAD</td></tr>"
            ).arg(q.value(0).toString())
             .arg(q.value(1).toInt())
             .arg(QString::number(
                 q.value(2).toDouble(),'f',2))
             .arg(tva)
             .arg(QString::number(ht,'f',2));
        }
    }
    details += "</table>";

    QMessageBox dlg(this);
    dlg.setWindowTitle("Détails Facture N° " + numero);
    dlg.setTextFormat(Qt::RichText);
    dlg.setText(details);
    dlg.exec();
}

// ═══════════════════════════════════════════════════════
// PAIEMENTS
// ═══════════════════════════════════════════════════════
void ClientWindow::setupPaiements()
{
    QWidget *widget = new QWidget;
    QVBoxLayout *layout = new QVBoxLayout(widget);
    layout->setSpacing(10);
    layout->setContentsMargins(14, 14, 14, 14);

    QLabel *title = new QLabel("💰 Historique des Paiements");
    title->setStyleSheet(
        "font-size:18px;font-weight:bold;color:#1B2A3B;");
    layout->addWidget(title);

    // ── Résumé financier ──────────────────────────────
    QGroupBox *resumeGroup = new QGroupBox(
        "📊 Résumé Financier");
    QHBoxLayout *resumeLayout =
        new QHBoxLayout(resumeGroup);
    resumeLayout->setSpacing(12);
    resumeLayout->setContentsMargins(10, 14, 10, 14);

    auto makeFinCard = [](const QString &t,
                          const QString &color,
                          QLabel *&lbl) -> QFrame* {
        QFrame *f = new QFrame;
        f->setFixedHeight(80);
        f->setSizePolicy(QSizePolicy::Expanding,
                         QSizePolicy::Fixed);
        f->setStyleSheet(QString(
            "QFrame{"
            "  background:white;"
            "  border-radius:8px;"
            "  border:1px solid #E2E8F0;"
            "  border-top:4px solid %1;"
            "}").arg(color));
        QVBoxLayout *vl = new QVBoxLayout(f);
        vl->setContentsMargins(14, 8, 14, 8);
        QLabel *tl = new QLabel(t);
        tl->setStyleSheet(
            "font-size:10px;color:#718096;"
            "font-weight:bold;border:none;");
        lbl = new QLabel("--");
        lbl->setStyleSheet(QString(
            "font-size:18px;font-weight:900;"
            "color:%1;border:none;").arg(color));
        vl->addWidget(tl);
        vl->addWidget(lbl);
        return f;
    };

    resumeLayout->addWidget(
        makeFinCard("Total Facturé",
                    "#2B6CB0", totalDuLabel));
    resumeLayout->addWidget(
        makeFinCard("Total Payé",
                    "#27AE60", totalPayeLabel));
    resumeLayout->addWidget(
        makeFinCard("Reste à Payer",
                    "#E53E3E", resteLabel));
    layout->addWidget(resumeGroup);

    // ── Tableau pleine largeur ─────────────────────────
    QGroupBox *histGroup = new QGroupBox(
        "📋 Détail des Paiements");
    histGroup->setSizePolicy(QSizePolicy::Expanding,
                              QSizePolicy::Expanding);
    QVBoxLayout *histLayout =
        new QVBoxLayout(histGroup);
    histLayout->setContentsMargins(10, 10, 10, 10);
    histLayout->setSpacing(8);

    paiementsModel = new QSqlQueryModel(this);

    paiementsTable = new QTableView;
    paiementsTable->setModel(paiementsModel);
    paiementsTable->verticalHeader()->setVisible(false);
    paiementsTable->horizontalHeader()
                  ->setStretchLastSection(true);
    paiementsTable->horizontalHeader()
                  ->setSectionResizeMode(
                      QHeaderView::Stretch); // ← pleine largeur
    paiementsTable->setEditTriggers(
        QAbstractItemView::NoEditTriggers);
    paiementsTable->setSelectionBehavior(
        QAbstractItemView::SelectRows);
    paiementsTable->setAlternatingRowColors(true);
    paiementsTable->setSizePolicy(
        QSizePolicy::Expanding,
        QSizePolicy::Expanding); // ← s'étend
    histLayout->addWidget(paiementsTable);

    // ── Bouton actualiser DANS le groupbox ────────────
    QPushButton *refreshBtn = new QPushButton(
        "🔄 Actualiser");
    refreshBtn->setFixedHeight(36);
    refreshBtn->setFixedWidth(160);
    refreshBtn->setStyleSheet(
        "background:#3182CE;"
        "color:white;"
        "font-size:12px;"
        "font-weight:bold;"
        "border-radius:4px;"
        "border:none;");

    QHBoxLayout *btnRow = new QHBoxLayout;
    btnRow->addWidget(refreshBtn);
    btnRow->addStretch();
    histLayout->addLayout(btnRow); // ← dans le même groupbox

    layout->addWidget(histGroup);

    // ← PAS de stretch final pour que le tableau s'étende
    tabWidget->addTab(widget, "💰 Paiements");

    refreshPaiements();
    connect(refreshBtn, &QPushButton::clicked,
            this, &ClientWindow::refreshPaiements);
}
void ClientWindow::refreshPaiements()
{
    // ── Résumé financier ──────────────────────────────
    QSqlQuery q;
    q.prepare(
        "SELECT "
        "COALESCE(SUM(f.total_ttc), 0), "
        "COALESCE(SUM(p.montant), 0), "
        "COALESCE(SUM(f.total_ttc), 0) - "
        "COALESCE(SUM(p.montant), 0) "
        "FROM factures f "
        "LEFT JOIN paiements p ON p.facture_id = f.id "
        "WHERE f.client_id = ?");
    q.addBindValue(m_clientId);

    if (q.exec() && q.next()) {
        totalDuLabel->setText(
            QString::number(q.value(0).toDouble(),
                            'f', 2) + " MAD");
        totalPayeLabel->setText(
            QString::number(q.value(1).toDouble(),
                            'f', 2) + " MAD");
        double reste = qMax(0.0, q.value(2).toDouble());
        resteLabel->setText(
            QString::number(reste, 'f', 2) + " MAD");
    }

    // ── Historique paiements ──────────────────────────
    // Colonnes correctes : methode et notes
    // (pas mode_paiement ni reference)
    paiementsModel->setQuery(QString(
        "SELECT "
        "f.numero        AS 'N° Facture', "
        "date(p.date_paiement) AS 'Date', "
        "p.montant       AS 'Montant (MAD)', "
        "p.methode       AS 'Méthode', "
        "p.notes         AS 'Notes', "
        "f.statut        AS 'Statut Facture' "
        "FROM paiements p "
        "JOIN factures f ON p.facture_id = f.id "
        "WHERE f.client_id = %1 "
        "ORDER BY p.date_paiement DESC, p.id DESC"
    ).arg(m_clientId));

    if (paiementsModel->lastError().isValid()) {
        qDebug() << "Erreur paiements client:"
                 << paiementsModel->lastError().text();
    }

    paiementsTable->resizeColumnsToContents();
    paiementsTable->horizontalHeader()
                  ->setStretchLastSection(true);
}
// ═══════════════════════════════════════════════════════
// PROFIL
// ═══════════════════════════════════════════════════════
void ClientWindow::setupProfil()
{
    QScrollArea *scroll = new QScrollArea;
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);

    QWidget *container = new QWidget;
    QVBoxLayout *layout = new QVBoxLayout(container);
    layout->setSpacing(14);
    layout->setContentsMargins(14, 14, 14, 14);

    QLabel *title = new QLabel("⚙️ Mon Profil");
    title->setStyleSheet(
        "font-size:18px;font-weight:bold;color:#1B2A3B;");
    layout->addWidget(title);

    // Infos personnelles
    QGroupBox *infoGroup = new QGroupBox(
        "👤 Informations Personnelles");
    QFormLayout *infoForm = new QFormLayout(infoGroup);
    infoForm->setSpacing(10);
    infoForm->setLabelAlignment(Qt::AlignRight);

    nomEdit    = new QLineEdit;
    prenomEdit = new QLineEdit;
    emailEdit  = new QLineEdit;
    telEdit    = new QLineEdit;
    adresseEdit= new QLineEdit;

    infoForm->addRow("Nom :*", nomEdit);
    infoForm->addRow("Prénom :*", prenomEdit);
    infoForm->addRow("Email :*", emailEdit);
    infoForm->addRow("Téléphone :", telEdit);
    infoForm->addRow("Adresse :", adresseEdit);

    QPushButton *saveInfoBtn = new QPushButton(
        "💾 Enregistrer les modifications");
    saveInfoBtn->setFixedHeight(36);
    saveInfoBtn->setStyleSheet(
        "background:#27AE60;color:white;"
        "font-weight:bold;");
    infoForm->addRow("", saveInfoBtn);
    layout->addWidget(infoGroup);

    // Changer mot de passe
    QGroupBox *pwdGroup = new QGroupBox(
        "🔒 Changer le Mot de Passe");
    QFormLayout *pwdForm = new QFormLayout(pwdGroup);
    pwdForm->setSpacing(10);
    pwdForm->setLabelAlignment(Qt::AlignRight);

    oldPasswordEdit  = new QLineEdit;
    newPasswordEdit  = new QLineEdit;
    confirmPasswordEdit = new QLineEdit;
    oldPasswordEdit->setEchoMode(QLineEdit::Password);
    newPasswordEdit->setEchoMode(QLineEdit::Password);
    confirmPasswordEdit->setEchoMode(QLineEdit::Password);
    oldPasswordEdit->setPlaceholderText(
        "Mot de passe actuel...");
    newPasswordEdit->setPlaceholderText(
        "Nouveau mot de passe (min 6 caractères)...");
    confirmPasswordEdit->setPlaceholderText(
        "Confirmer le nouveau mot de passe...");

    pwdForm->addRow("Actuel :*", oldPasswordEdit);
    pwdForm->addRow("Nouveau :*", newPasswordEdit);
    pwdForm->addRow("Confirmer :*", confirmPasswordEdit);

    QPushButton *changePwdBtn = new QPushButton(
        "🔑 Changer le Mot de Passe");
    changePwdBtn->setFixedHeight(36);
    changePwdBtn->setStyleSheet(
        "background:#805AD5;color:white;"
        "font-weight:bold;");
    pwdForm->addRow("", changePwdBtn);
    layout->addWidget(pwdGroup);

    scroll->setWidget(container);
    tabWidget->addTab(scroll, "⚙️ Mon Profil");

    // Charger les données
    QSqlQuery q;
    q.prepare("SELECT nom, prenom, email, "
              "telephone, adresse "
              "FROM clients WHERE id = ?");
    q.addBindValue(m_clientId);
    if (q.exec() && q.next()) {
        nomEdit->setText(q.value(0).toString());
        prenomEdit->setText(q.value(1).toString());
        emailEdit->setText(q.value(2).toString());
        telEdit->setText(q.value(3).toString());
        adresseEdit->setText(q.value(4).toString());
    }

    connect(saveInfoBtn, &QPushButton::clicked,
            this, &ClientWindow::onSaveProfil);
    connect(changePwdBtn, &QPushButton::clicked,
            this, &ClientWindow::onChangePassword);
}

void ClientWindow::onSaveProfil()
{
    if (nomEdit->text().trimmed().isEmpty() ||
        prenomEdit->text().trimmed().isEmpty() ||
        emailEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "Erreur",
            "Nom, prénom et email sont obligatoires.");
        return;
    }

    QSqlQuery q;
    q.prepare(
        "UPDATE clients SET "
        "nom=?, prenom=?, email=?, "
        "telephone=?, adresse=? WHERE id=?");
    q.addBindValue(nomEdit->text().trimmed());
    q.addBindValue(prenomEdit->text().trimmed());
    q.addBindValue(emailEdit->text().trimmed());
    q.addBindValue(telEdit->text().trimmed());
    q.addBindValue(adresseEdit->text().trimmed());
    q.addBindValue(m_clientId);

    if (q.exec()) {
        m_clientNom = nomEdit->text() + " " +
                      prenomEdit->text();
        setWindowTitle("👤 Espace Client — " +
                       m_clientNom);
        QMessageBox::information(this, "✅ Succès",
            "Profil mis à jour avec succès !");
    } else {
        QMessageBox::critical(this, "Erreur",
            q.lastError().text());
    }
}

void ClientWindow::onChangePassword()
{
    QString oldPwd = oldPasswordEdit->text();
    QString newPwd = newPasswordEdit->text();
    QString confPwd = confirmPasswordEdit->text();

    if (oldPwd.isEmpty() || newPwd.isEmpty()) {
        QMessageBox::warning(this, "Erreur",
            "Remplissez tous les champs.");
        return;
    }
    if (newPwd != confPwd) {
        QMessageBox::warning(this, "Erreur",
            "Les mots de passe ne correspondent pas.");
        return;
    }
    if (newPwd.length() < 6) {
        QMessageBox::warning(this, "Erreur",
            "Mot de passe trop court (min 6 caractères).");
        return;
    }

    // ✅ Hashage des mots de passe
    QString hashedOld = LoginDialog::hashPassword(oldPwd);
    QString hashedNew = LoginDialog::hashPassword(newPwd);

    // Vérifier l'ancien mot de passe (comparaison hashée)
    QSqlQuery checkQ;
    checkQ.prepare(
        "SELECT id FROM clients "
        "WHERE id = ? AND mot_de_passe = ?");
    checkQ.addBindValue(m_clientId);
    checkQ.addBindValue(hashedOld);

    if (!checkQ.exec() || !checkQ.next()) {
        QMessageBox::warning(this, "Erreur",
            "Mot de passe actuel incorrect.");
        return;
    }

    // Mettre à jour avec le nouveau hash
    QSqlQuery upd;
    upd.prepare(
        "UPDATE clients SET mot_de_passe = ? "
        "WHERE id = ?");
    upd.addBindValue(hashedNew);
    upd.addBindValue(m_clientId);

    if (upd.exec()) {
        oldPasswordEdit->clear();
        newPasswordEdit->clear();
        confirmPasswordEdit->clear();
        QMessageBox::information(this, "✅ Succès",
            "Mot de passe changé avec succès !");
    } else {
        QMessageBox::critical(this, "Erreur",
            upd.lastError().text());
    }
}

// ═══════════════════════════════════════════════════════
// CONTACT
// ═══════════════════════════════════════════════════════
void ClientWindow::setupContact()
{
    QWidget *widget = new QWidget;
    QVBoxLayout *layout = new QVBoxLayout(widget);
    layout->setSpacing(14);
    layout->setContentsMargins(14, 14, 14, 14);

    QLabel *title = new QLabel(
        "📧 Contacter l'Administrateur");
    title->setStyleSheet(
        "font-size:18px;font-weight:bold;color:#1B2A3B;");
    layout->addWidget(title);

    QGroupBox *msgGroup = new QGroupBox(
        "✉️ Nouveau Message");
    QFormLayout *msgForm = new QFormLayout(msgGroup);
    msgForm->setSpacing(10);
    msgForm->setLabelAlignment(Qt::AlignRight);

    sujetEdit = new QLineEdit;
    sujetEdit->setPlaceholderText(
        "Objet de votre message...");
    messageEdit = new QTextEdit;
    messageEdit->setMinimumHeight(200);
    messageEdit->setPlaceholderText(
        "Écrivez votre message ici...\n\n"
        "Nous vous répondrons dans les plus brefs délais.");

    msgForm->addRow("Sujet :*", sujetEdit);
    msgForm->addRow("Message :*", messageEdit);

    QPushButton *sendBtn = new QPushButton(
        "📧 Envoyer le Message");
    sendBtn->setFixedHeight(40);
    sendBtn->setStyleSheet(
        "background:#2B6CB0;color:white;"
        "font-size:13px;font-weight:bold;");
    msgForm->addRow("", sendBtn);
    layout->addWidget(msgGroup);

    // Info
    QLabel *info = new QLabel(
        "💡 Votre message sera envoyé par email "
        "à l'administrateur.\n"
        "Une copie sera envoyée à votre adresse: " +
        m_clientEmail);
    info->setStyleSheet(
        "color:#718096;font-size:10px;"
        "font-style:italic;");
    info->setWordWrap(true);
    layout->addWidget(info);
    layout->addStretch();

    tabWidget->addTab(widget, "📧 Contact");

    connect(sendBtn, &QPushButton::clicked,
            this, &ClientWindow::onSendMessage);
}

void ClientWindow::onSendMessage()
{
    if (sujetEdit->text().trimmed().isEmpty() ||
        messageEdit->toPlainText().trimmed().isEmpty()) {
        QMessageBox::warning(this, "Erreur",
            "Sujet et message sont obligatoires.");
        return;
    }

    QString sujet = "[FacturationApp] " +
                    sujetEdit->text().trimmed();
    QString body  = QString(
        "Message de: %1 (%2)\n\n"
        "Sujet: %3\n\n"
        "%4\n\n"
        "---\nEnvoyé depuis FacturationApp"
    ).arg(m_clientNom, m_clientEmail,
          sujetEdit->text(), 
          messageEdit->toPlainText());

    // Email vers admin
    EmailSender sender;
    bool ok = sender.sendSmtp(
        "admin@facturation.com", sujet, body);

    if (ok) {
        sujetEdit->clear();
        messageEdit->clear();
        QMessageBox::information(this, "✅ Envoyé",
            "Votre message a été envoyé à "
            "l'administrateur.\n"
            "Nous vous répondrons rapidement !");
    } else {
        QMessageBox::warning(this, "⚠️ Attention",
            "Message enregistré.\n"
            "L'envoi email a échoué mais "
            "l'administrateur sera notifié.");
    }
}


void ClientWindow::onLogout()
{
    auto reply = QMessageBox::question(this,
        "Déconnexion",
        "Voulez-vous vraiment vous déconnecter ?",
        QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        // Retourner à la page de login
        if (MainWindow::instance())
            MainWindow::instance()->showLogin();
    }
}