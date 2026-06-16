#include "clientwindow.h"
#include "mainwindow.h"
#include <QButtonGroup>
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
#include <QSqlQuery>
#include <QSqlError>
#include <QDate>
#include <QHeaderView>
#include <QDesktopServices>
#include <QUrl>
#include <QCryptographicHash>
#include <QDebug>
#include <QGraphicsDropShadowEffect>

// ── Design tokens identiques à AdminWindow ────────────
namespace C {
    constexpr auto BG       = "#F1F5F9";
    constexpr auto CARD     = "#FFFFFF";
    constexpr auto BORDER   = "#E2E8F0";
    constexpr auto TXT_HEAD = "#0F172A";     // ← noir (comme admin)
    constexpr auto TXT_SUB  = "#64748B";
    constexpr auto BLUE     = "#2563EB";
    constexpr auto GREEN    = "#16A34A";
    constexpr auto RED      = "#DC2626";
    constexpr auto GRAY     = "#64748B";
    constexpr auto AMBER    = "#D97706";
    constexpr auto VIOLET   = "#7C3AED";
    constexpr auto TH_BG    = "#1E3A5F";
    constexpr auto TH_TXT   = "#FFFFFF";
    constexpr auto SIDEBAR  = "#ffffff";    // ← blanc (comme admin)
    constexpr auto SB_TXT   = "#030000";     // ← noir (comme admin)
    constexpr auto SB_ACT   = "#1D3461";   // ← bleu foncé (comme admin)
}

static QGraphicsDropShadowEffect* mkShadow()
{
    auto *e = new QGraphicsDropShadowEffect;
    e->setBlurRadius(16);
    e->setOffset(0, 3);
    e->setColor(QColor(0, 0, 0, 22));
    return e;
}

// ── Helper bouton ─────────────────────────────────────
static QPushButton* makeBtn(const QString &label,
                             const QString &color,
                             int minW = 80)
{
    QPushButton *b = new QPushButton(label);
    b->setFixedHeight(34);
    b->setMinimumWidth(minW);
    b->setCursor(Qt::PointingHandCursor);
    b->setStyleSheet(
        QString("QPushButton{background:%1;color:white;"
                "font-weight:600;border:none;"
                "border-radius:7px;padding:0 12px;"
                "font-size:12px;}"
                "QPushButton:hover{opacity:0.9;}")
        .arg(color));
    return b;
}

ClientWindow::ClientWindow(int clientId, QWidget *parent)
    : QWidget(parent), m_clientId(clientId)
{
    loadClientInfo();
    setupUI();
    refreshDashboard();
}

void ClientWindow::loadClientInfo()
{
    QSqlQuery q;
    q.prepare("SELECT nom, prenom, email "
              "FROM clients WHERE id = ?");
    q.addBindValue(m_clientId);
    if (q.exec() && q.next()) {
        m_clientNom   = q.value(0).toString() + " " +
                        q.value(1).toString();
        m_clientEmail = q.value(2).toString();
    }
}

// ═════════════════════════════════════════════════════
// setupUI — Sidebar + QStackedWidget
// ═════════════════════════════════════════════════════
void ClientWindow::setupUI()
{
    setStyleSheet(
        QString("background:%1;").arg(C::BG));

    QHBoxLayout *root = new QHBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

    // Sidebar
    root->addWidget(buildSidebar());

    // Pages
    m_pageStack = new QStackedWidget;
    m_pageStack->setStyleSheet(
        QString("background:%1;").arg(C::BG));

    m_pageStack->addWidget(buildDashboardPage()); // 0
    m_pageStack->addWidget(buildFacturesPage());  // 1
    m_pageStack->addWidget(buildPaiementsPage()); // 2
    m_pageStack->addWidget(buildProfilPage());    // 3
    m_pageStack->addWidget(buildContactPage());   // 4

    m_pageStack->setCurrentIndex(0);
    root->addWidget(m_pageStack, 1);
}

// ═════════════════════════════════════════════════════
// SIDEBAR — identique visuellement à AdminWindow
// ═════════════════════════════════════════════════════
QWidget* ClientWindow::buildSidebar()
{
    QWidget *sidebar = new QWidget;
    sidebar->setFixedWidth(230);
    sidebar->setStyleSheet(
        QString("background:%1;").arg(C::SIDEBAR));  // ← #ffffff

    QVBoxLayout *sl = new QVBoxLayout(sidebar);
    sl->setContentsMargins(0, 0, 0, 0);
    sl->setSpacing(0);

    // ── Logo (fixe en haut) ──────────────────────────────────────────────
    QWidget *logoArea = new QWidget;
    logoArea->setFixedHeight(64);
    logoArea->setStyleSheet(
        QString("background:%1;border-bottom:1px solid #000006;")
            .arg(C::SIDEBAR));
    QHBoxLayout *ll = new QHBoxLayout(logoArea);
    ll->setContentsMargins(20, 0, 16, 0);

    QLabel *logoIcon = new QLabel("F");
    logoIcon->setFixedSize(32, 32);
    logoIcon->setAlignment(Qt::AlignCenter);
    logoIcon->setStyleSheet(
        QString("background:#60A5FA;border-radius:8px;"
                "font-size:16px;font-weight:800;color:#0F172A;"));

    QLabel *logoText = new QLabel("FacturationApp");
    logoText->setStyleSheet(
        QString("font-size:16px;font-weight:800;color:#60A5FA;"
                "letter-spacing:0.3px;"));

    ll->addWidget(logoIcon);
    ll->addSpacing(8);
    ll->addWidget(logoText);
    ll->addStretch();
    sl->addWidget(logoArea);

    // ── ZONE SCROLLABLE pour la navigation ───────────────────────────────
    QScrollArea *scrollArea = new QScrollArea;
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameStyle(QFrame::NoFrame);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollArea->setStyleSheet(
        "QScrollArea { background:transparent; border:none; }"
        "QScrollBar:vertical { width:4px; background:transparent; }"
        "QScrollBar::handle:vertical { background:#CBD5E1; border-radius:2px; min-height:30px; }"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height:0px; }"
        "QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical { background:transparent; }");

    QWidget *navContainer = new QWidget;
    navContainer->setStyleSheet("background:transparent;");
    QVBoxLayout *navLayout = new QVBoxLayout(navContainer);
    navLayout->setContentsMargins(0, 0, 0, 0);
    navLayout->setSpacing(0);

    // Section label helper
    auto addSection = [&](const QString &label) {
        QLabel *sec = new QLabel(label.toUpper());
        sec->setFixedHeight(24);
        sec->setStyleSheet(
            QString("font-size:9px;font-weight:700;color:#13458b;"
                    "letter-spacing:1.8px;padding:0 20px;"));
        navLayout->addWidget(sec);
    };

    // Nav item helper — Style Admin avec barre d'accent
    auto addNavItem = [&](const QString &icon,
                          const QString &label,
                          const QString &page,
                          bool isActive = false) -> QPushButton*
    {
        QPushButton *btn = new QPushButton;
        btn->setFixedHeight(38);
        btn->setCursor(Qt::PointingHandCursor);
        btn->setFlat(true);

        QHBoxLayout *bl = new QHBoxLayout(btn);
        bl->setContentsMargins(0, 0, 0, 0);
        bl->setSpacing(0);

        // Barre d'accent bleue (3px) à gauche
        QFrame *bar = new QFrame;
        bar->setFixedWidth(3);
        bar->setStyleSheet(
            QString("background:%1;border-radius:0;")
                .arg(isActive ? "#3B82F6" : "transparent"));

        QLabel *ico = new QLabel(icon);
        ico->setFixedWidth(36);
        ico->setAlignment(Qt::AlignCenter);
        ico->setStyleSheet("font-size:15px;background:transparent;");

        QLabel *txt = new QLabel(label);
        txt->setStyleSheet(
            QString("font-size:15px;font-weight:%1;color:%2;background:transparent;")
                .arg(isActive ? "800" : "700")
                .arg(isActive ? "#ffffff" : "#030000"));

        bl->addWidget(bar);
        bl->addSpacing(10);
        bl->addWidget(ico);
        bl->addWidget(txt);
        bl->addStretch();

        btn->setStyleSheet(
            QString("QPushButton{background:%1;border:none;}"
                    "QPushButton:hover{background:#ebf0f7;}")
                .arg(isActive ? "#1D3461" : "transparent"));

        navLayout->addWidget(btn);

        connect(btn, &QPushButton::clicked, this,
                [this, btn, bar, txt, page]()
        {
            // Désactiver précédent
            if (activeNavBtn && activeNavBtn != btn) {
                QHBoxLayout *prevL = qobject_cast<QHBoxLayout*>(activeNavBtn->layout());
                if (prevL) {
                    QFrame *prevBar = qobject_cast<QFrame*>(prevL->itemAt(0)->widget());
                    QLabel *prevTxt = nullptr;
                    for (int i = 0; i < prevL->count(); i++) {
                        QLabel *l = qobject_cast<QLabel*>(prevL->itemAt(i)->widget());
                        if (l && l->text().length() > 2) { prevTxt = l; break; }
                    }
                    if (prevBar)
                        prevBar->setStyleSheet("background:transparent;border-radius:0;");
                    if (prevTxt)
                        prevTxt->setStyleSheet(
                            "font-size:15px;font-weight:700;color:#030000;"
                            "background:transparent;");
                    activeNavBtn->setStyleSheet(
                        "QPushButton{background:transparent;border:none;}"
                        "QPushButton:hover{background:#ebf0f7;}");
                }
            }
            activeNavBtn = btn;
            bar->setStyleSheet("background:#3B82F6;border-radius:0;");
            txt->setStyleSheet(
                "font-size:15px;font-weight:800;color:#ffffff;"
                "background:transparent;");
            btn->setStyleSheet(
                "QPushButton{background:#1D3461;border:none;}"
                "QPushButton:hover{background:#1D3461;}");
            onNavigateTo(page);
        });

        if (isActive) activeNavBtn = btn;
        return btn;
    };

    // ── SECTION PRINCIPAL ────────────────────────────────────────────────
    navLayout->addSpacing(4);
    addSection("Principal");
    addNavItem("🏠", "Accueil",        "dashboard",  true);
    addNavItem("📄", "Mes Factures",   "factures");
    addNavItem("💰", "Paiements",      "paiements");
    addNavItem("⚙️", "Mon Profil",     "profil");
    addNavItem("📧", "Contact",        "contact");

    navLayout->addStretch();
    scrollArea->setWidget(navContainer);
    sl->addWidget(scrollArea);
    sl->setStretchFactor(scrollArea, 1);

    // ── BAS DE SIDEBAR (fixe, toujours visible) ──────────────────────────
    QFrame *div = new QFrame;
    div->setFixedHeight(1);
    div->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    div->setStyleSheet("background:#000006;margin:0 16px;");
    sl->addWidget(div);
    sl->addSpacing(10);

    // User chip bleu
    QWidget *chip = new QWidget;
    chip->setFixedHeight(50);
    chip->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    chip->setStyleSheet(
        "background:#3B82F6;border-radius:10px;margin:0 12px;");
    QHBoxLayout *ul = new QHBoxLayout(chip);
    ul->setContentsMargins(10, 0, 10, 0);
    ul->setSpacing(10);

    QLabel *avatar = new QLabel("A");
    avatar->setFixedSize(30, 30);
    avatar->setAlignment(Qt::AlignCenter);
    avatar->setStyleSheet(
        "border-radius:11px;font-size:15px;font-weight:700;color:#0F172A;");

    QVBoxLayout *uinfo = new QVBoxLayout;
    uinfo->setSpacing(1);
    QLabel *uname = new QLabel(m_clientNom);
    uname->setStyleSheet(
        "font-size:11px;font-weight:600;color:#FFFFFF;background:transparent;");
    QLabel *urole = new QLabel("Client");
    urole->setStyleSheet(
        "font-size:10px;color:rgba(255,255,255,0.75);background:transparent;");
    uinfo->addWidget(uname);
    uinfo->addWidget(urole);

    ul->addWidget(avatar);
    ul->addLayout(uinfo);
    ul->addStretch();
    sl->addWidget(chip);
    sl->addSpacing(10);

    // Logout button rouge
    QPushButton *logoutBtn = new QPushButton("  🔓  Déconnexion");
    logoutBtn->setFixedHeight(42);
    logoutBtn->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    logoutBtn->setCursor(Qt::PointingHandCursor);
    logoutBtn->setFlat(true);
    logoutBtn->setStyleSheet(
        "QPushButton{"
        "  background:rgba(226, 8, 8, 0.89);"
        "  border:1px solid rgba(239,68,68,0.25);"
        "  border-radius:8px;"
        "  margin:0 12px;"
        "  font-size:14px;font-weight:700;color:#ffffff;"
        "}"
        "QPushButton:hover{background:rgba(231, 14, 14, 0.73);}");
    connect(logoutBtn, &QPushButton::clicked,
            this, &ClientWindow::onLogout);
    sl->addWidget(logoutBtn);
    sl->addSpacing(14);

    return sidebar;
}
void ClientWindow::onNavigateTo(const QString &page)
{
    if      (page == "dashboard") m_pageStack->setCurrentIndex(0);
    else if (page == "factures")  m_pageStack->setCurrentIndex(1);
    else if (page == "paiements") m_pageStack->setCurrentIndex(2);
    else if (page == "profil")    m_pageStack->setCurrentIndex(3);
    else if (page == "contact")   m_pageStack->setCurrentIndex(4);
}

// ═════════════════════════════════════════════════════
// Helper: page wrapper avec header
// ═════════════════════════════════════════════════════
static QVBoxLayout* buildPageLayout(
    QWidget *page,
    const QString &title,
    const QString &subtitle)
{
    page->setStyleSheet(
        QString("background:%1;").arg(C::BG));
    QVBoxLayout *vl = new QVBoxLayout(page);
    vl->setContentsMargins(28, 22, 28, 20);
    vl->setSpacing(16);
    QLabel *ttl = new QLabel(title);
    ttl->setStyleSheet(
        QString("font-size:20px;font-weight:700;"
                "color:%1;").arg(C::TXT_HEAD));
    QLabel *sub = new QLabel(subtitle);
    sub->setStyleSheet(
        QString("font-size:12px;color:%1;")
        .arg(C::TXT_SUB));
    vl->addWidget(ttl);
    vl->addWidget(sub);
    return vl;
}

static QFrame* buildCard()
{
    QFrame *card = new QFrame;
    card->setStyleSheet(
        QString("QFrame{background:%1;"
                "border-radius:14px;"
                "border:1px solid %2;}")
        .arg(C::CARD, C::BORDER));
    card->setGraphicsEffect(mkShadow());
    return card;
}

static QString tableStyle()
{
    return QString(
        "QTableView{"
        "border:none;gridline-color:#F1F5F9;"
        "selection-background-color:#DBEAFE;"
        "selection-color:#1E3A5F;font-size:12px;}"
        "QHeaderView::section{"
        "background:%1;color:%2;"
        "font-weight:700;padding:8px 6px;"
        "border:none;font-size:11px;}"
        "QTableView::item{"
        "padding:6px;border-bottom:1px solid #F1F5F9;}"
    ).arg(C::TH_BG, C::TH_TXT);
}

// ═════════════════════════════════════════════════════
// DASHBOARD PAGE
// ═════════════════════════════════════════════════════
QWidget* ClientWindow::buildDashboardPage()
{
    QWidget *page = new QWidget;
    QVBoxLayout *vl = buildPageLayout(page,
        "🏠  Mon Tableau de Bord",
        "Vue d'ensemble de votre compte");

    // Cartes stats
    QWidget *cardsWidget = new QWidget;
    QGridLayout *grid =
        new QGridLayout(cardsWidget);
    grid->setSpacing(14);

    auto makeCard = [](const QString &icon,
                       const QString &title,
                       const QString &color,
                       QLabel *&valueLabel) -> QFrame* {
        QFrame *card = new QFrame;
        card->setMinimumHeight(100);
        card->setStyleSheet(
            QString("QFrame{background:white;"
                    "border-radius:12px;"
                    "border:1px solid #E2E8F0;"
                    "border-left:5px solid %1;}")
            .arg(color));
        card->setGraphicsEffect([color](){
            auto *e = new QGraphicsDropShadowEffect;
            e->setBlurRadius(12);
            e->setOffset(0,2);
            e->setColor(QColor(0,0,0,18));
            return e;
        }());
        QVBoxLayout *vl = new QVBoxLayout(card);
        vl->setContentsMargins(16, 12, 16, 12);
        QLabel *tl = new QLabel(icon + "  " + title);
        tl->setStyleSheet(
            "font-size:10px;color:#718096;"
            "font-weight:bold;letter-spacing:1px;"
            "border:none;");
        valueLabel = new QLabel("--");
        valueLabel->setStyleSheet(
            QString("font-size:22px;font-weight:900;"
                    "color:%1;border:none;")
            .arg(color));
        vl->addWidget(tl);
        vl->addWidget(valueLabel);
        return card;
    };

    grid->addWidget(makeCard("📄","Total Factures",
        C::BLUE,  statTotalFactures),  0, 0);
    grid->addWidget(makeCard("💰","Montant Dû",
        C::RED,   statMontantDu),      0, 1);
    grid->addWidget(makeCard("✅","Total Payé",
        C::GREEN, statTotalPaye),      0, 2);
    grid->addWidget(makeCard("📅","Ce Mois",
        C::AMBER, statFacturesMois),   1, 0);
    grid->addWidget(makeCard("🕐","Dernière Facture",
        C::VIOLET,statDerniereFacture),1, 1);
    grid->addWidget(makeCard("🏷️","Statut Compte",
        C::BLUE,  statStatutCompte),   1, 2);
    vl->addWidget(cardsWidget);

    // Dernières factures
    QFrame *card = buildCard();
    QVBoxLayout *cl = new QVBoxLayout(card);
    cl->setContentsMargins(20, 16, 20, 16);
    cl->setSpacing(10);
    QLabel *rttl = new QLabel("📋  Dernières Factures");
    rttl->setStyleSheet(
        "font-size:14px;font-weight:bold;"
        "color:#1E293B;");
    QPushButton *refreshBtn = makeBtn(
        "🔄 Actualiser", C::BLUE, 110);
    QHBoxLayout *rh = new QHBoxLayout;
    rh->addWidget(rttl);
    rh->addStretch();
    rh->addWidget(refreshBtn);
    cl->addLayout(rh);

    recentModel = new QSqlQueryModel(this);
    recentTable = new QTableView;
    recentTable->setModel(recentModel);
    recentTable->verticalHeader()->setVisible(false);
    recentTable->horizontalHeader()
               ->setStretchLastSection(true);
    recentTable->setEditTriggers(
        QAbstractItemView::NoEditTriggers);
    recentTable->setSelectionBehavior(
        QAbstractItemView::SelectRows);
    recentTable->setAlternatingRowColors(true);
    recentTable->setStyleSheet(tableStyle());
    cl->addWidget(recentTable);
    vl->addWidget(card, 1);

    connect(refreshBtn, &QPushButton::clicked,
            this, &ClientWindow::refreshDashboard);
    return page;
}

// ═════════════════════════════════════════════════════
// FACTURES PAGE
// ═════════════════════════════════════════════════════
QWidget* ClientWindow::buildFacturesPage()
{
    QWidget *page = new QWidget;
    QVBoxLayout *vl = buildPageLayout(page,
        "📄  Mes Factures",
        "Consultez et téléchargez vos factures");

    QFrame *card = buildCard();
    QVBoxLayout *cl = new QVBoxLayout(card);
    cl->setContentsMargins(20, 16, 20, 16);
    cl->setSpacing(12);

    // Filtres
    QHBoxLayout *filterRow = new QHBoxLayout;
    filterRow->setSpacing(8);

    filterStatut = new QComboBox;
    filterStatut->addItems({
        "Tous","Brouillon","Envoyée",
        "Partiellement payée","Payée","Annulée"});
    filterStatut->setStyleSheet(
        "padding:5px;border:1px solid #CBD5E0;"
        "border-radius:6px;font-size:12px;");
    filterStatut->setFixedHeight(34);

    filterDateDebut = new QDateEdit(
        QDate::currentDate().addMonths(-6));
    filterDateDebut->setCalendarPopup(true);
    filterDateDebut->setDisplayFormat("dd/MM/yyyy");
    filterDateDebut->setFixedHeight(34);
    filterDateDebut->setStyleSheet(
        "padding:5px;border:1px solid #CBD5E0;"
        "border-radius:6px;font-size:12px;");

    filterDateFin = new QDateEdit(
        QDate::currentDate().addDays(1));
    filterDateFin->setCalendarPopup(true);
    filterDateFin->setDisplayFormat("dd/MM/yyyy");
    filterDateFin->setFixedHeight(34);
    filterDateFin->setStyleSheet(
        "padding:5px;border:1px solid #CBD5E0;"
        "border-radius:6px;font-size:12px;");

    searchFacture = new QLineEdit;
    searchFacture->setPlaceholderText("🔍 N° facture…");
    searchFacture->setFixedHeight(34);
    searchFacture->setStyleSheet(
        "padding:5px 10px;border:1px solid #CBD5E0;"
        "border-radius:6px;font-size:12px;");

    QPushButton *filterBtn =
        makeBtn("🔍 Filtrer", C::BLUE, 90);
    QPushButton *resetBtn =
        makeBtn("✕ Reset", C::GRAY, 70);

    filterRow->addWidget(new QLabel("Statut:"));
    filterRow->addWidget(filterStatut);
    filterRow->addWidget(new QLabel("Du:"));
    filterRow->addWidget(filterDateDebut);
    filterRow->addWidget(new QLabel("Au:"));
    filterRow->addWidget(filterDateFin);
    filterRow->addWidget(searchFacture, 1);
    filterRow->addWidget(filterBtn);
    filterRow->addWidget(resetBtn);
    cl->addLayout(filterRow);

    // Boutons actions
    QHBoxLayout *actRow = new QHBoxLayout;
    actRow->setSpacing(8);
    QPushButton *pdfBtn =
        makeBtn("📄 Télécharger PDF", C::BLUE, 150);
    QPushButton *detailBtn =
        makeBtn("👁️ Voir Détails", C::VIOLET, 120);
    actRow->addWidget(pdfBtn);
    actRow->addWidget(detailBtn);
    actRow->addStretch();
    cl->addLayout(actRow);

    // Tableau
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
    facturesTable->setStyleSheet(tableStyle());
    cl->addWidget(facturesTable, 1);
    vl->addWidget(card, 1);

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
    return page;
}

// ═════════════════════════════════════════════════════
// PAIEMENTS PAGE
// ═════════════════════════════════════════════════════
QWidget* ClientWindow::buildPaiementsPage()
{
    QWidget *page = new QWidget;
    QVBoxLayout *vl = buildPageLayout(page,
        "💰  Mes Paiements",
        "Historique de vos paiements");

    // Cartes résumé
    QWidget *resumeWidget = new QWidget;
    QHBoxLayout *rl = new QHBoxLayout(resumeWidget);
    rl->setSpacing(14);

    auto makeFinCard = [](const QString &t,
                          const QString &color,
                          QLabel *&lbl) -> QFrame* {
        QFrame *f = new QFrame;
        f->setFixedHeight(80);
        f->setStyleSheet(
            QString("QFrame{background:white;"
                    "border-radius:12px;"
                    "border:1px solid #E2E8F0;"
                    "border-top:4px solid %1;}")
            .arg(color));
        QVBoxLayout *vl = new QVBoxLayout(f);
        vl->setContentsMargins(14, 8, 14, 8);
        QLabel *tl = new QLabel(t);
        tl->setStyleSheet(
            "font-size:10px;color:#718096;"
            "font-weight:bold;border:none;");
        lbl = new QLabel("--");
        lbl->setStyleSheet(
            QString("font-size:18px;font-weight:900;"
                    "color:%1;border:none;")
            .arg(color));
        vl->addWidget(tl);
        vl->addWidget(lbl);
        return f;
    };

    rl->addWidget(makeFinCard(
        "Total Facturé", C::BLUE, totalDuLabel));
    rl->addWidget(makeFinCard(
        "Total Payé", C::GREEN, totalPayeLabel));
    rl->addWidget(makeFinCard(
        "Reste à Payer", C::RED, resteLabel));
    vl->addWidget(resumeWidget);

    // Tableau
    QFrame *card = buildCard();
    QVBoxLayout *cl = new QVBoxLayout(card);
    cl->setContentsMargins(20, 16, 20, 16);
    cl->setSpacing(10);

    QHBoxLayout *th = new QHBoxLayout;
    QLabel *htitle = new QLabel("📋  Détail des Paiements");
    htitle->setStyleSheet(
        "font-size:14px;font-weight:bold;color:#1E293B;");
    QPushButton *refreshBtn =
        makeBtn("🔄 Actualiser", C::BLUE, 110);
    th->addWidget(htitle);
    th->addStretch();
    th->addWidget(refreshBtn);
    cl->addLayout(th);

    paiementsModel = new QSqlQueryModel(this);
    paiementsTable = new QTableView;
    paiementsTable->setModel(paiementsModel);
    paiementsTable->verticalHeader()->setVisible(false);
    paiementsTable->horizontalHeader()
                  ->setStretchLastSection(true);
    paiementsTable->horizontalHeader()
                  ->setSectionResizeMode(
                      QHeaderView::Stretch);
    paiementsTable->setEditTriggers(
        QAbstractItemView::NoEditTriggers);
    paiementsTable->setSelectionBehavior(
        QAbstractItemView::SelectRows);
    paiementsTable->setAlternatingRowColors(true);
    paiementsTable->setStyleSheet(tableStyle());
    cl->addWidget(paiementsTable, 1);
    vl->addWidget(card, 1);

    refreshPaiements();
    connect(refreshBtn, &QPushButton::clicked,
            this, &ClientWindow::refreshPaiements);
    return page;
}

// ═════════════════════════════════════════════════════
// PROFIL PAGE
// ═════════════════════════════════════════════════════
QWidget* ClientWindow::buildProfilPage()
{
    QScrollArea *scroll = new QScrollArea;
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setStyleSheet(
        QString("background:%1;").arg(C::BG));

    QWidget *page = new QWidget;
    QVBoxLayout *vl = buildPageLayout(page,
        "⚙️  Mon Profil",
        "Gérez vos informations personnelles");

    // Infos
    QFrame *card1 = buildCard();
    QVBoxLayout *c1l = new QVBoxLayout(card1);
    c1l->setContentsMargins(24, 20, 24, 20);
    c1l->setSpacing(14);
    QLabel *ct1 = new QLabel("👤 Informations Personnelles");
    ct1->setStyleSheet(
        "font-size:14px;font-weight:bold;"
        "color:#1E293B;");
    c1l->addWidget(ct1);

    QString inputStyle =
        "border:1px solid #CBD5E0;border-radius:8px;"
        "padding:8px 12px;font-size:13px;"
        "background:white;min-height:34px;";

    QFormLayout *form = new QFormLayout;
    form->setSpacing(10);
    form->setLabelAlignment(Qt::AlignRight);
    nomEdit     = new QLineEdit;
    prenomEdit  = new QLineEdit;
    emailEdit   = new QLineEdit;
    telEdit     = new QLineEdit;
    adresseEdit = new QLineEdit;
    for (auto *e : {nomEdit,prenomEdit,emailEdit,
                    telEdit,adresseEdit})
        e->setStyleSheet(inputStyle);
    form->addRow("Nom :*",     nomEdit);
    form->addRow("Prénom :*",  prenomEdit);
    form->addRow("Email :*",   emailEdit);
    form->addRow("Téléphone:", telEdit);
    form->addRow("Adresse:",   adresseEdit);
    c1l->addLayout(form);
    QPushButton *saveBtn =
        makeBtn("💾 Enregistrer", C::GREEN, 160);
    QHBoxLayout *sh = new QHBoxLayout;
    sh->addStretch();
    sh->addWidget(saveBtn);
    c1l->addLayout(sh);
    vl->addWidget(card1);

    // Mot de passe
    QFrame *card2 = buildCard();
    QVBoxLayout *c2l = new QVBoxLayout(card2);
    c2l->setContentsMargins(24, 20, 24, 20);
    c2l->setSpacing(14);
    QLabel *ct2 = new QLabel("🔒 Changer le Mot de Passe");
    ct2->setStyleSheet(
        "font-size:14px;font-weight:bold;"
        "color:#1E293B;");
    c2l->addWidget(ct2);

    QFormLayout *pwdForm = new QFormLayout;
    pwdForm->setSpacing(10);
    pwdForm->setLabelAlignment(Qt::AlignRight);
    oldPasswordEdit     = new QLineEdit;
    newPasswordEdit     = new QLineEdit;
    confirmPasswordEdit = new QLineEdit;
    oldPasswordEdit->setEchoMode(QLineEdit::Password);
    newPasswordEdit->setEchoMode(QLineEdit::Password);
    confirmPasswordEdit->setEchoMode(QLineEdit::Password);
    for (auto *e : {oldPasswordEdit,
                    newPasswordEdit,
                    confirmPasswordEdit})
        e->setStyleSheet(inputStyle);
    pwdForm->addRow("Actuel :*",    oldPasswordEdit);
    pwdForm->addRow("Nouveau :*",   newPasswordEdit);
    pwdForm->addRow("Confirmer :*", confirmPasswordEdit);
    c2l->addLayout(pwdForm);
    QPushButton *pwdBtn =
        makeBtn("🔑 Changer", C::VIOLET, 140);
    QHBoxLayout *ph = new QHBoxLayout;
    ph->addStretch();
    ph->addWidget(pwdBtn);
    c2l->addLayout(ph);
    vl->addWidget(card2);
    vl->addStretch();

    scroll->setWidget(page);

    // Charger données
    QSqlQuery q;
    q.prepare("SELECT nom,prenom,email,telephone,"
              "adresse FROM clients WHERE id=?");
    q.addBindValue(m_clientId);
    if (q.exec() && q.next()) {
        nomEdit->setText(q.value(0).toString());
        prenomEdit->setText(q.value(1).toString());
        emailEdit->setText(q.value(2).toString());
        telEdit->setText(q.value(3).toString());
        adresseEdit->setText(q.value(4).toString());
    }

    connect(saveBtn, &QPushButton::clicked,
            this, &ClientWindow::onSaveProfil);
    connect(pwdBtn,  &QPushButton::clicked,
            this, &ClientWindow::onChangePassword);
    return scroll;
}

// ═════════════════════════════════════════════════════
// CONTACT PAGE
// ═════════════════════════════════════════════════════
QWidget* ClientWindow::buildContactPage()
{
    QWidget *page = new QWidget;
    QVBoxLayout *vl = buildPageLayout(page,
        "📧  Contact",
        "Envoyez un message à l'administrateur");

    QFrame *card = buildCard();
    QVBoxLayout *cl = new QVBoxLayout(card);
    cl->setContentsMargins(24, 20, 24, 20);
    cl->setSpacing(14);

    QString inputStyle =
        "border:1px solid #CBD5E0;border-radius:8px;"
        "padding:8px 12px;font-size:13px;background:white;";

    QLabel *stl = new QLabel("✉️ Nouveau Message");
    stl->setStyleSheet(
        "font-size:14px;font-weight:bold;"
        "color:#1E293B;");
    cl->addWidget(stl);

    QFormLayout *form = new QFormLayout;
    form->setSpacing(10);
    form->setLabelAlignment(Qt::AlignRight);
    sujetEdit = new QLineEdit;
    sujetEdit->setStyleSheet(inputStyle);
    sujetEdit->setPlaceholderText("Objet du message...");
    messageEdit = new QTextEdit;
    messageEdit->setStyleSheet(inputStyle);
    messageEdit->setMinimumHeight(160);
    messageEdit->setPlaceholderText(
        "Votre message...");
    form->addRow("Sujet :*",   sujetEdit);
    form->addRow("Message :*", messageEdit);
    cl->addLayout(form);

    QPushButton *sendBtn =
        makeBtn("📧 Envoyer", C::BLUE, 130);
    QHBoxLayout *bh = new QHBoxLayout;
    bh->addStretch();
    bh->addWidget(sendBtn);
    cl->addLayout(bh);
    vl->addWidget(card);
    vl->addStretch();

    connect(sendBtn, &QPushButton::clicked,
            this, &ClientWindow::onSendMessage);
    return page;
}

// ═════════════════════════════════════════════════════
// SLOTS — mêmes implémentations qu'avant
// ═════════════════════════════════════════════════════
void ClientWindow::refreshDashboard()
{
    QSqlQuery q;
    q.prepare("SELECT COUNT(*) FROM factures "
              "WHERE client_id=?");
    q.addBindValue(m_clientId);
    q.exec();
    statTotalFactures->setText(
        q.next() ?
        QString::number(q.value(0).toInt()) : "0");

    q.prepare(
        "SELECT COALESCE(SUM(f.total_ttc),0)-"
        "COALESCE(SUM(p.montant),0) "
        "FROM factures f "
        "LEFT JOIN paiements p ON p.facture_id=f.id "
        "WHERE f.client_id=?");
    q.addBindValue(m_clientId);
    q.exec();
    statMontantDu->setText(
        q.next() ?
        QString::number(
            qMax(0.0,q.value(0).toDouble()),
            'f',2)+" MAD" : "0 MAD");

    q.prepare(
        "SELECT COALESCE(SUM(p.montant),0) "
        "FROM paiements p "
        "JOIN factures f ON p.facture_id=f.id "
        "WHERE f.client_id=?");
    q.addBindValue(m_clientId);
    q.exec();
    statTotalPaye->setText(
        q.next() ?
        QString::number(q.value(0).toDouble(),'f',2)+
        " MAD" : "0 MAD");

    QString mois =
        QDate::currentDate().toString("yyyy-MM");
    q.prepare(
        "SELECT COUNT(*) FROM factures "
        "WHERE client_id=? AND "
        "strftime('%Y-%m',date_creation)=?");
    q.addBindValue(m_clientId);
    q.addBindValue(mois);
    q.exec();
    statFacturesMois->setText(
        q.next() ?
        QString::number(q.value(0).toInt()) : "0");

    q.prepare(
        "SELECT numero FROM factures "
        "WHERE client_id=? "
        "ORDER BY date_creation DESC LIMIT 1");
    q.addBindValue(m_clientId);
    q.exec();
    statDerniereFacture->setText(
        q.next() ?
        "N°"+q.value(0).toString() : "Aucune");

    q.prepare(
        "SELECT COUNT(*) FROM factures "
        "WHERE client_id=? AND "
        "statut NOT IN ('Payée','Annulée')");
    q.addBindValue(m_clientId);
    q.exec();
    int imp = q.next() ? q.value(0).toInt() : 0;
    statStatutCompte->setText(
        imp==0 ? "✅ À jour" :
        QString("⚠️ %1 en attente").arg(imp));

    recentModel->setQuery(QString(
        "SELECT f.numero AS 'N°',"
        "date(f.date_creation) AS 'Date',"
        "f.total_ttc AS 'Total TTC',"
        "COALESCE(SUM(p.montant),0) AS 'Payé',"
        "f.total_ttc-COALESCE(SUM(p.montant),0)"
        " AS 'Reste',"
        "f.statut AS 'Statut' "
        "FROM factures f "
        "LEFT JOIN paiements p ON p.facture_id=f.id "
        "WHERE f.client_id=%1 "
        "GROUP BY f.id "
        "ORDER BY f.date_creation DESC LIMIT 5"
    ).arg(m_clientId));
    recentTable->resizeColumnsToContents();
    recentTable->horizontalHeader()
               ->setStretchLastSection(true);
}

void ClientWindow::onFilterFactures()
{
    QString where = QString(
        "WHERE f.client_id=%1").arg(m_clientId);
    QString statut = filterStatut->currentText();
    if (statut != "Tous")
        where += QString(" AND f.statut='%1'")
                 .arg(statut);
    where += QString(
        " AND date(f.date_creation) BETWEEN "
        "'%1' AND '%2'")
        .arg(filterDateDebut->date()
             .toString("yyyy-MM-dd"),
             filterDateFin->date()
             .toString("yyyy-MM-dd"));
    QString s = searchFacture->text().trimmed();
    if (!s.isEmpty())
        where += QString(" AND f.numero LIKE '%%1%'")
                 .arg(s);

    facturesModel->setQuery(QString(
        "SELECT f.id AS 'ID',"
        "f.numero AS 'N° Facture',"
        "date(f.date_creation) AS 'Date',"
        "date(f.date_echeance) AS 'Échéance',"
        "f.total_ttc AS 'Total TTC',"
        "COALESCE(SUM(p.montant),0) AS 'Payé',"
        "f.total_ttc-COALESCE(SUM(p.montant),0)"
        " AS 'Reste',"
        "f.statut AS 'Statut' "
        "FROM factures f "
        "LEFT JOIN paiements p ON p.facture_id=f.id "
        "%1 GROUP BY f.id "
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
        QMessageBox::warning(this,"Sélection",
            "Sélectionnez une facture.");
        return;
    }
    int id = facturesModel->data(
        facturesModel->index(row,0)).toInt();
    InvoiceStyle style;
    InvoiceGenerator gen;
    QString path =
        InvoiceGenerator::getPdfOutputPath() +
        "/Facture_" + QString::number(id) + ".pdf";
    if (gen.generatePDF(id, path, style)) {
        QMessageBox::information(this,"✅ Succès",
            "PDF généré:\n"+path);
        QDesktopServices::openUrl(
            QUrl::fromLocalFile(path));
    } else {
        QMessageBox::critical(this,"Erreur",
            "Impossible de générer le PDF.");
    }
}

void ClientWindow::onViewDetails()
{
    int row = facturesTable->currentIndex().row();
    if (row < 0) {
        QMessageBox::warning(this,"Sélection",
            "Sélectionnez une facture.");
        return;
    }
    int id = facturesModel->data(
        facturesModel->index(row,0)).toInt();
    QString num = facturesModel->data(
        facturesModel->index(row,1)).toString();
    QSqlQuery q;
    q.prepare(
        "SELECT designation,quantite,"
        "prix_unitaire_ht,taux_tva "
        "FROM lignes_facture WHERE facture_id=?");
    q.addBindValue(id);
    QString html =
        QString("<h3>Facture N° %1</h3>"
                "<table border='1' cellpadding='5' "
                "style='border-collapse:collapse;"
                "width:100%%;'>"
                "<tr style='background:#2B6CB0;"
                "color:white;'>"
                "<th>Désignation</th><th>Qté</th>"
                "<th>Prix HT</th><th>TVA</th>"
                "<th>Total</th></tr>")
        .arg(num);
    if (q.exec()) {
        while (q.next()) {
            double ht = q.value(1).toInt() *
                        q.value(2).toDouble();
            html += QString(
                "<tr><td>%1</td><td>%2</td>"
                "<td>%3 MAD</td><td>%4%%</td>"
                "<td>%5 MAD</td></tr>")
                .arg(q.value(0).toString())
                .arg(q.value(1).toInt())
                .arg(QString::number(
                    q.value(2).toDouble(),'f',2))
                .arg(q.value(3).toDouble())
                .arg(QString::number(ht,'f',2));
        }
    }
    html += "</table>";
    QMessageBox dlg(this);
    dlg.setWindowTitle("Détails N°"+num);
    dlg.setTextFormat(Qt::RichText);
    dlg.setText(html);
    dlg.exec();
}

void ClientWindow::refreshPaiements()
{
    QSqlQuery q;
    q.prepare(
        "SELECT "
        "COALESCE(SUM(f.total_ttc),0),"
        "COALESCE(SUM(p.montant),0),"
        "COALESCE(SUM(f.total_ttc),0)-"
        "COALESCE(SUM(p.montant),0) "
        "FROM factures f "
        "LEFT JOIN paiements p ON p.facture_id=f.id "
        "WHERE f.client_id=?");
    q.addBindValue(m_clientId);
    if (q.exec() && q.next()) {
        totalDuLabel->setText(
            QString::number(
                q.value(0).toDouble(),'f',2)+" MAD");
        totalPayeLabel->setText(
            QString::number(
                q.value(1).toDouble(),'f',2)+" MAD");
        resteLabel->setText(
            QString::number(
                qMax(0.0,q.value(2).toDouble()),
                'f',2)+" MAD");
    }
    paiementsModel->setQuery(QString(
        "SELECT f.numero AS 'N° Facture',"
        "date(p.date_paiement) AS 'Date',"
        "p.montant AS 'Montant (MAD)',"
        "p.methode AS 'Méthode',"
        "p.notes AS 'Notes',"
        "f.statut AS 'Statut Facture' "
        "FROM paiements p "
        "JOIN factures f ON p.facture_id=f.id "
        "WHERE f.client_id=%1 "
        "ORDER BY p.date_paiement DESC,p.id DESC"
    ).arg(m_clientId));
    paiementsTable->resizeColumnsToContents();
    paiementsTable->horizontalHeader()
                  ->setStretchLastSection(true);
}

void ClientWindow::onSaveProfil()
{
    if (nomEdit->text().trimmed().isEmpty() ||
        prenomEdit->text().trimmed().isEmpty() ||
        emailEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(this,"Erreur",
            "Nom, prénom et email obligatoires.");
        return;
    }
    QSqlQuery q;
    q.prepare(
        "UPDATE clients SET nom=?,prenom=?,"
        "email=?,telephone=?,adresse=? WHERE id=?");
    q.addBindValue(nomEdit->text().trimmed());
    q.addBindValue(prenomEdit->text().trimmed());
    q.addBindValue(emailEdit->text().trimmed());
    q.addBindValue(telEdit->text().trimmed());
    q.addBindValue(adresseEdit->text().trimmed());
    q.addBindValue(m_clientId);
    if (q.exec())
        QMessageBox::information(this,"✅ Succès",
            "Profil mis à jour !");
    else
        QMessageBox::critical(this,"Erreur",
            q.lastError().text());
}

void ClientWindow::onChangePassword()
{
    QString oldPwd = oldPasswordEdit->text();
    QString newPwd = newPasswordEdit->text();
    QString confPwd = confirmPasswordEdit->text();
    if (oldPwd.isEmpty() || newPwd.isEmpty()) {
        QMessageBox::warning(this,"Erreur",
            "Remplissez tous les champs.");
        return;
    }
    if (newPwd != confPwd) {
        QMessageBox::warning(this,"Erreur",
            "Mots de passe différents.");
        return;
    }
    if (newPwd.length() < 6) {
        QMessageBox::warning(this,"Erreur",
            "Minimum 6 caractères.");
        return;
    }
    QString hashedOld =
        LoginDialog::hashPassword(oldPwd);
    QSqlQuery chk;
    chk.prepare(
        "SELECT id FROM clients "
        "WHERE id=? AND mot_de_passe=?");
    chk.addBindValue(m_clientId);
    chk.addBindValue(hashedOld);
    if (!chk.exec() || !chk.next()) {
        QMessageBox::warning(this,"Erreur",
            "Mot de passe actuel incorrect.");
        return;
    }
    QSqlQuery upd;
    upd.prepare(
        "UPDATE clients SET mot_de_passe=? "
        "WHERE id=?");
    upd.addBindValue(
        LoginDialog::hashPassword(newPwd));
    upd.addBindValue(m_clientId);
    if (upd.exec()) {
        oldPasswordEdit->clear();
        newPasswordEdit->clear();
        confirmPasswordEdit->clear();
        QMessageBox::information(this,"✅ Succès",
            "Mot de passe changé !");
    } else {
        QMessageBox::critical(this,"Erreur",
            upd.lastError().text());
    }
}

void ClientWindow::onSendMessage()
{
    if (sujetEdit->text().trimmed().isEmpty() ||
        messageEdit->toPlainText().trimmed().isEmpty()){
        QMessageBox::warning(this,"Erreur",
            "Sujet et message obligatoires.");
        return;
    }
    EmailSender sender;
    bool ok = sender.sendSmtp(
        "hadi.oumama@gmail.com",
        "[FacturationApp] "+sujetEdit->text(),
        QString("De: %1 (%2)\n\n%3")
        .arg(m_clientNom,m_clientEmail,
             messageEdit->toPlainText()));
    if (ok) {
        sujetEdit->clear();
        messageEdit->clear();
        QMessageBox::information(this,"✅ Envoyé",
            "Message envoyé !");
    } else {
        QMessageBox::warning(this,"Attention",
            "Envoi échoué.");
    }
}

void ClientWindow::onLogout()
{
    if (QMessageBox::question(this,"Déconnexion",
            "Se déconnecter ?",
            QMessageBox::Yes|QMessageBox::No)
            == QMessageBox::Yes)
        emit logoutRequested();
}