#include "dashboardwidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSqlQuery>
#include <QDate>
#include <QPainter>
#include <QGraphicsDropShadowEffect>
#include <QtCharts/QChart>
#include <QtCharts/QSplineSeries>
#include <QtCharts/QAreaSeries>
#include <QtCharts/QPieSlice>
#include <QtCharts/QBarCategoryAxis>
#include <QtCharts/QValueAxis>
#include <QLinearGradient>

// ─────────────────────────────────────────────────────────────────────────────
namespace T {
    constexpr auto SB_BG        = "#ffffff";
    constexpr auto SB_HOVER     = "#ebf0f7";
    constexpr auto SB_ACTIVE_BG = "#1D3461";
    constexpr auto SB_BORDER    = "#000006";
    constexpr auto SB_LOGO      = "#60A5FA";
    constexpr auto SB_SECTION   = "#13458b";
    constexpr auto SB_TEXT      = "#030000";
    constexpr auto SB_TEXT_ACT  = "#ffffff";
    constexpr auto SB_ACCENT    = "#3B82F6";
    constexpr auto SB_LOGOUT    = "#ffffff";
    constexpr auto BG           = "#F1F5F9";
    constexpr auto CARD_BG      = "#FFFFFF";
    constexpr auto CARD_BORDER  = "#E2E8F0";
    constexpr auto C_BLUE       = "#2563EB";
    constexpr auto C_GREEN      = "#16A34A";
    constexpr auto C_RED        = "#DC2626";
    constexpr auto C_VIOLET     = "#7C3AED";
    constexpr auto C_AMBER      = "#D97706";
    constexpr auto C_ORANGE     = "#C2410C";
    constexpr auto CHART_LINE   = "#3B82F6";
}

static QGraphicsDropShadowEffect* softShadow(int blur = 18, int alpha = 25)
{
    auto *e = new QGraphicsDropShadowEffect;
    e->setBlurRadius(blur);
    e->setOffset(0, 3);
    e->setColor(QColor(0, 0, 0, alpha));
    return e;
}

// ─────────────────────────────────────────────────────────────────────────────
DashboardWidget::DashboardWidget(QWidget *parent) : QWidget(parent)
{
    // Build sidebar and content separately; do NOT add them to `this` layout
    // because AdminWindow will pull them apart via sidebarOnly()/contentArea().
    setupSidebar();   // creates sidebarWidget
    setupStatCards(); // creates cardsWidget
    setupCharts();    // creates chartsWidget

    // ── Build the right-side content area ────────────────────────────────────
    m_contentArea = new QWidget;
    m_contentArea->setStyleSheet(QString("background:%1;").arg(T::BG));
    QVBoxLayout *cv = new QVBoxLayout(m_contentArea);
    cv->setContentsMargins(28, 20, 28, 16);
    cv->setSpacing(16);

    // Header bar
    QWidget *header = new QWidget;
    header->setStyleSheet("background:transparent;");
    QHBoxLayout *hl = new QHBoxLayout(header);
    hl->setContentsMargins(0,0,0,0);

    QLabel *pageTitle = new QLabel("Vue d'ensemble");
    pageTitle->setStyleSheet(
        "font-family:'Segoe UI Semibold','SF Pro Display',sans-serif;"
        "font-size:30px;font-weight:900;color:SB_BORDER;");

    QLabel *dateLbl = new QLabel(
        QDate::currentDate().toString("dddd d MMMM yyyy"));
    dateLbl->setStyleSheet("font-size:11px;color:SB_BORDER;");

    

    hl->addWidget(pageTitle);
    hl->addStretch();
    hl->addWidget(dateLbl);
    hl->addSpacing(10);
  

    cv->addWidget(header);
    cv->addWidget(cardsWidget);
    cv->addWidget(chartsWidget, 1);

    // Timer
    refreshTimer = new QTimer(this);
    connect(refreshTimer, &QTimer::timeout, this, &DashboardWidget::refreshData);
    refreshTimer->start(30000);
    refreshData();
}

// DashboardWidget itself has NO layout — AdminWindow composes the two parts.
void DashboardWidget::setupUI() {}

// ─────────────────────────────────────────────────────────────────────────────
//  Sidebar
// ─────────────────────────────────────────────────────────────────────────────
void DashboardWidget::setupSidebar()
{
    sidebarWidget = new QWidget;
    sidebarWidget->setFixedWidth(230);
    sidebarWidget->setStyleSheet(
        QString("background:%1;").arg(T::SB_BG));

    QVBoxLayout *sl = new QVBoxLayout(sidebarWidget);
    sl->setContentsMargins(0, 0, 0, 0);
    sl->setSpacing(0);

    // Logo
    QWidget *logoArea = new QWidget;
    logoArea->setFixedHeight(64);
    logoArea->setStyleSheet(
        QString("background:%1;border-bottom:1px solid %2;")
            .arg(T::SB_BG, T::SB_BORDER));
    QHBoxLayout *ll = new QHBoxLayout(logoArea);
    ll->setContentsMargins(20, 0, 16, 0);

    QLabel *logoIcon = new QLabel("F");
    logoIcon->setFixedSize(32, 32);
    logoIcon->setAlignment(Qt::AlignCenter);
    logoIcon->setStyleSheet(
        QString("background:%1;border-radius:8px;"
                "font-size:16px;font-weight:800;color:#0F172A;")
            .arg(T::SB_LOGO));

    QLabel *logoText = new QLabel("FacturPro");
    logoText->setStyleSheet(
        QString("font-size:16px;font-weight:800;color:%1;letter-spacing:0.3px;")
            .arg(T::SB_LOGO));

    ll->addWidget(logoIcon);
    ll->addSpacing(8);
    ll->addWidget(logoText);
    ll->addStretch();
    sl->addWidget(logoArea);

    // Section label helper
    auto addSection = [&](const QString &label) {
        QLabel *sec = new QLabel(label.toUpper());
        sec->setFixedHeight(32);
        sec->setStyleSheet(
            QString("font-size:9px;font-weight:700;color:%1;"
                    "letter-spacing:1.8px;padding:0 20px;")
                .arg(T::SB_SECTION));
        sl->addWidget(sec);
    };

    // Nav item helper
    auto addNavItem = [&](const QString &icon,
                          const QString &label,
                          const QString &page,
                          bool isActive = false) -> QPushButton*
    {
        QPushButton *btn = new QPushButton;
        btn->setFixedHeight(46);
        btn->setCursor(Qt::PointingHandCursor);
        btn->setFlat(true);

        QHBoxLayout *bl = new QHBoxLayout(btn);
        bl->setContentsMargins(0, 0, 0, 0);
        bl->setSpacing(0);

        QFrame *bar = new QFrame;
        bar->setFixedWidth(3);
        bar->setStyleSheet(
            QString("background:%1;border-radius:0;")
                .arg(isActive ? T::SB_ACCENT : "transparent"));

        QLabel *ico = new QLabel(icon);
        ico->setFixedWidth(36);
        ico->setAlignment(Qt::AlignCenter);
        ico->setStyleSheet("font-size:15px;background:transparent;");

        QLabel *txt = new QLabel(label);
        txt->setStyleSheet(
            QString("font-size:15px;font-weight:%1;color:%2;background:transparent;")
                .arg(isActive ? "800" : "700")
                .arg(isActive ? T::SB_TEXT_ACT : T::SB_TEXT));

        bl->addWidget(bar);
        bl->addSpacing(10);
        bl->addWidget(ico);
        bl->addWidget(txt);
        bl->addStretch();

        btn->setStyleSheet(
            QString("QPushButton{background:%1;border:none;}"
                    "QPushButton:hover{background:%2;}")
                .arg(isActive ? T::SB_ACTIVE_BG : "transparent", T::SB_HOVER));

        sl->addWidget(btn);

        connect(btn, &QPushButton::clicked, this,
                [this, btn, bar, txt, page]()
        {
            if (activeNavBtn && activeNavBtn != btn) {
                // Reset previous (find its bar/text children)
                QHBoxLayout *prevL =
                    qobject_cast<QHBoxLayout*>(activeNavBtn->layout());
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
                            QString("font-size:13px;font-weight:400;color:%1;"
                                    "background:transparent;").arg(T::SB_TEXT));
                    activeNavBtn->setStyleSheet(
                        QString("QPushButton{background:transparent;border:none;}"
                                "QPushButton:hover{background:%1;}").arg(T::SB_HOVER));
                }
            }
            activeNavBtn = btn;
            bar->setStyleSheet(
                QString("background:%1;border-radius:0;").arg(T::SB_ACCENT));
            txt->setStyleSheet(
                QString("font-size:13px;font-weight:600;color:%1;"
                        "background:transparent;").arg(T::SB_TEXT_ACT));
            btn->setStyleSheet(
                QString("QPushButton{background:%1;border:none;}"
                        "QPushButton:hover{background:%1;}").arg(T::SB_ACTIVE_BG));
            emit navigateTo(page);
        });

        if (isActive) activeNavBtn = btn;
        return btn;
    };

    sl->addSpacing(8);

    addSection("Principal");
    addNavItem("▦",  "Tableau de bord",  "dashboard",  true);
    addNavItem("🧾", "Gestion Factures", "factures");
    addNavItem("👥", "Gestion Clients",  "clients");
    addNavItem("📦", "Gestion Articles", "articles");

    sl->addSpacing(8);
    

    sl->addStretch();

    // Divider
    QFrame *div = new QFrame;
    div->setFixedHeight(1);
    div->setStyleSheet(
        QString("background:%1;margin:0 16px;").arg(T::SB_BORDER));
    sl->addWidget(div);
    sl->addSpacing(10);

    // User chip
    QWidget *chip = new QWidget;
    chip->setFixedHeight(50);
    chip->setStyleSheet(
        QString("background:%1;border-radius:10px;margin:0 12px;")
            .arg(T::SB_HOVER));
    QHBoxLayout *ul = new QHBoxLayout(chip);
    ul->setContentsMargins(10, 0, 10, 0);
    ul->setSpacing(10);

    QLabel *avatar = new QLabel("A");
    avatar->setFixedSize(30, 30);
    avatar->setAlignment(Qt::AlignCenter);
    avatar->setStyleSheet(
        QString("background:%1;border-radius:15px;"
                "font-size:13px;font-weight:700;color:#0F172A;")
            .arg(T::SB_LOGO));

    QVBoxLayout *uinfo = new QVBoxLayout;
    uinfo->setSpacing(1);
    QLabel *uname = new QLabel("Administrateur");
    uname->setStyleSheet(
        QString("font-size:11px;font-weight:600;color:%1;background:#2563EB;")
            .arg(T::SB_ACCENT ));
    QLabel *urole = new QLabel("Super Admin");
    urole->setStyleSheet(
        QString("font-size:9px;color:%1;background:transparent;")
            .arg(T::SB_SECTION));
    uinfo->addWidget(uname);
    uinfo->addWidget(urole);

    ul->addWidget(avatar);
    ul->addLayout(uinfo);
    ul->addStretch();
    sl->addWidget(chip);
    sl->addSpacing(10);

    // Logout button
    QPushButton *logoutBtn = new QPushButton("  🔓  Déconnexion");
    logoutBtn->setFixedHeight(42);
    logoutBtn->setCursor(Qt::PointingHandCursor);
    logoutBtn->setFlat(true);
    logoutBtn->setStyleSheet(
        QString("QPushButton{"
                "  background:rgba(226, 8, 8, 0.89);"
                "  border:1px solid rgba(239,68,68,0.25);"
                "  border-radius:8px;"
                "  margin:0 12px;"
                "  font-size:14px;font-weight:700;color:%1;"
                "}"
                "QPushButton:hover{background:rgba(194, 20, 20, 0.18);}")
            .arg(T::SB_LOGOUT));
    connect(logoutBtn, &QPushButton::clicked,
            this, &DashboardWidget::logoutRequested);
    sl->addWidget(logoutBtn);
    sl->addSpacing(14);
}

// ─────────────────────────────────────────────────────────────────────────────
//  Stat cards
// ─────────────────────────────────────────────────────────────────────────────
void DashboardWidget::setupStatCards()
{
    cardsWidget = new QWidget;
    cardsWidget->setStyleSheet("background:transparent;");
    QHBoxLayout *row = new QHBoxLayout(cardsWidget);
    row->setSpacing(12);
    row->setContentsMargins(0,0,0,0);

    struct CardDef { QString title, icon, accent; QLabel **ptr; };
    QVector<CardDef> defs = {
        {"Chiffre d'Affaires", "💰", T::C_BLUE,   &totalCALabel},
        {"Factures Payées",    "✅", T::C_GREEN,  &facturesPayeesLabel},
        {"Factures Impayées",  "⚠",  T::C_RED,    &facturesImpayeesLabel},
        {"Total Clients",      "👥", T::C_VIOLET, &clientsTotalLabel},
        {"Factures / Mois",    "📅", T::C_AMBER,  &facturesMoisLabel},
        {"Montant en Attente", "⏳", T::C_ORANGE, &montantEnAttenteLabel},
    };

    for (auto &d : defs) {
        QFrame *card = new QFrame;
        card->setStyleSheet(
            QString("QFrame{"
                    "  background:%1;"
                    "  border-radius:12px;"
                    "  border:1px solid %2;"
                    "  border-top:3px solid %3;"
                    "}").arg(T::CARD_BG, T::CARD_BORDER, d.accent));
        card->setMinimumHeight(96);
        card->setGraphicsEffect(softShadow());

        QVBoxLayout *cl = new QVBoxLayout(card);
        cl->setContentsMargins(14, 12, 14, 12);
        cl->setSpacing(5);

        QLabel *lbl = new QLabel(d.icon + "  " + d.title);
        lbl->setStyleSheet(
            "font-size:12px;font-weight:800;color:#64748B;"
            "letter-spacing:0.7px;background:transparent;border:none;");

        QLabel *val = new QLabel("—");
        val->setStyleSheet(
            QString("font-size:20px;font-weight:800;color:%1;"
                    "background:transparent;border:none;"
                    "font-family:'Segoe UI Semibold','SF Pro Display',sans-serif;")
                .arg(d.accent));

        *d.ptr = val;
        cl->addWidget(lbl);
        cl->addWidget(val);
        row->addWidget(card, 1);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
//  Charts
// ─────────────────────────────────────────────────────────────────────────────
void DashboardWidget::setupCharts()
{
    chartsWidget = new QWidget;
    chartsWidget->setStyleSheet("background:transparent;");
    QHBoxLayout *layout = new QHBoxLayout(chartsWidget);
    layout->setSpacing(14);
    layout->setContentsMargins(0,0,0,0);

    auto makeCard = [](const QString &hdr, QChartView *&view) -> QFrame* {
        QFrame *f = new QFrame;
        f->setStyleSheet(
            QString("QFrame{background:%1;border-radius:14px;border:1px solid %2;}")
                .arg(T::CARD_BG, T::CARD_BORDER));
        auto *fx = new QGraphicsDropShadowEffect;
        fx->setBlurRadius(20); fx->setOffset(0,3); fx->setColor(QColor(0,0,0,22));
        f->setGraphicsEffect(fx);

        QVBoxLayout *vl = new QVBoxLayout(f);
        vl->setContentsMargins(18,14,18,14);
        vl->setSpacing(10);

        QLabel *title = new QLabel(hdr);
        title->setStyleSheet(
            "font-size:12px;font-weight:700;color:#0F172A;"
            "background:transparent;border:none;");

        view = new QChartView;
        view->setRenderHint(QPainter::Antialiasing);
        view->setFrameStyle(QFrame::NoFrame);
        view->setBackgroundBrush(Qt::transparent);
        view->setStyleSheet("background:transparent;border:none;");

        vl->addWidget(title);
        vl->addWidget(view, 1);
        return f;
    };

    layout->addWidget(makeCard("📈  Chiffre d'Affaires — 6 derniers mois", barChartView), 6);
    layout->addWidget(makeCard("Répartition des Factures", pieChartView), 4);
}

// ─────────────────────────────────────────────────────────────────────────────
//  Runtime — logique inchangée
// ─────────────────────────────────────────────────────────────────────────────
void DashboardWidget::refreshData()
{
    updateStats();
    updateBarChart();
    updatePieChart();
}

void DashboardWidget::updateStats()
{
    QSqlQuery q;
    q.exec("SELECT COALESCE(SUM(total_ttc),0) FROM factures WHERE statut='Payée'");
    if (q.next()) totalCALabel->setText(QString::number(q.value(0).toDouble(),'f',2)+" MAD");
    q.exec("SELECT COUNT(*) FROM factures WHERE statut='Payée'");
    if (q.next()) facturesPayeesLabel->setText(QString::number(q.value(0).toInt()));
    q.exec("SELECT COUNT(*) FROM factures WHERE statut IN ('Brouillon','Envoyée')");
    if (q.next()) facturesImpayeesLabel->setText(QString::number(q.value(0).toInt()));
    q.exec("SELECT COUNT(*) FROM clients WHERE role='client'");
    if (q.next()) clientsTotalLabel->setText(QString::number(q.value(0).toInt()));
    QString mois = QDate::currentDate().toString("yyyy-MM");
    q.prepare("SELECT COUNT(*) FROM factures WHERE strftime('%Y-%m', date_creation) = ?");
    q.addBindValue(mois); q.exec();
    if (q.next()) facturesMoisLabel->setText(QString::number(q.value(0).toInt()));
    q.exec("SELECT COALESCE(SUM(total_ttc),0) FROM factures WHERE statut IN ('Brouillon','Envoyée')");
    if (q.next()) montantEnAttenteLabel->setText(QString::number(q.value(0).toDouble(),'f',2)+" MAD");
}

void DashboardWidget::updateBarChart()
{
    QStringList moisLabels;
    QVector<double> values;
    double maxVal = 0;

    for (int i = 5; i >= 0; i--) {
        QDate date = QDate::currentDate().addMonths(-i);
        moisLabels << date.toString("MMM yyyy");
        QSqlQuery q;
        q.prepare("SELECT COALESCE(SUM(total_ttc),0) FROM factures "
                  "WHERE strftime('%Y-%m', date_creation) = ?");
        q.addBindValue(date.toString("yyyy-MM")); q.exec();
        double val = q.next() ? q.value(0).toDouble() : 0;
        values << val;
        if (val > maxVal) maxVal = val;
    }

    QSplineSeries *line = new QSplineSeries;
    line->setPen(QPen(QColor(T::CHART_LINE), 2.5));
    for (int i = 0; i < values.size(); i++) line->append(i, values[i]);

    QAreaSeries *area = new QAreaSeries(line);
    QPen pen(QColor(T::CHART_LINE)); pen.setWidth(2);
    area->setPen(pen);
    QLinearGradient g(0,0,0,1);
    g.setCoordinateMode(QGradient::ObjectBoundingMode);
    g.setColorAt(0.0, QColor(59,130,246,80));
    g.setColorAt(1.0, QColor(59,130,246,4));
    area->setBrush(g);

    QChart *chart = new QChart;
    chart->addSeries(area);
    chart->setAnimationOptions(QChart::SeriesAnimations);
    chart->legend()->hide();
    chart->setBackgroundBrush(Qt::transparent);
    chart->setContentsMargins(0,0,0,0);
    chart->setMargins(QMargins(4,4,4,4));

    QBarCategoryAxis *axX = new QBarCategoryAxis;
    axX->append(moisLabels);
    axX->setLabelsAngle(-25);
    axX->setLabelsColor(QColor("#94A3B8"));
    axX->setGridLineVisible(false);
    axX->setLinePenColor(QColor("#E2E8F0"));
    axX->setLabelsFont(QFont("Segoe UI",8));
    chart->addAxis(axX, Qt::AlignBottom);
    area->attachAxis(axX);

    QValueAxis *axY = new QValueAxis;
    axY->setLabelFormat("%.0f");
    axY->setRange(0, maxVal > 0 ? maxVal*1.25 : 1000);
    axY->setTickCount(5);
    axY->setGridLineColor(QColor("#F1F5F9"));
    axY->setLabelsColor(QColor("#94A3B8"));
    axY->setLabelsFont(QFont("Segoe UI",8));
    axY->setLinePenColor(Qt::transparent);
    chart->addAxis(axY, Qt::AlignLeft);
    area->attachAxis(axY);

    barChartView->setChart(chart);
    barChartView->setBackgroundBrush(Qt::transparent);
    barChartView->setFrameStyle(QFrame::NoFrame);
}

void DashboardWidget::updatePieChart()
{
    QPieSeries *series = new QPieSeries;
    struct SI { QString label, color, query; };
    QVector<SI> statuts = {
        {"Payées",    "#16A34A", "SELECT COUNT(*) FROM factures WHERE statut='Payée'"},
        {"Envoyées",  "#2563EB", "SELECT COUNT(*) FROM factures WHERE statut='Envoyée'"},
        {"Brouillon", "#94A3B8", "SELECT COUNT(*) FROM factures WHERE statut='Brouillon'"},
        {"Annulées",  "#DC2626", "SELECT COUNT(*) FROM factures WHERE statut='Annulée'"},
    };
    bool hasData = false;
    for (const auto &s : statuts) {
        QSqlQuery q; q.exec(s.query);
        if (q.next()) {
            int n = q.value(0).toInt();
            if (n > 0) {
                QPieSlice *sl = series->append(s.label+" ("+QString::number(n)+")", n);
                sl->setColor(QColor(s.color));
                sl->setLabelVisible(true);
                sl->setLabelColor(QColor(s.color));
                hasData = true;
            }
        }
    }
    if (!hasData) series->append("Aucune facture", 1);
    series->setHoleSize(0.42);

    QChart *chart = new QChart;
    chart->addSeries(series);
    chart->setAnimationOptions(QChart::SeriesAnimations);
    chart->legend()->setAlignment(Qt::AlignBottom);
    chart->legend()->setFont(QFont("Segoe UI",9));
    chart->legend()->setColor(QColor("#475569"));
    chart->setBackgroundBrush(Qt::transparent);
    chart->setContentsMargins(0,0,0,0);
    chart->setMargins(QMargins(4,4,4,4));

    pieChartView->setChart(chart);
    pieChartView->setBackgroundBrush(Qt::transparent);
    pieChartView->setFrameStyle(QFrame::NoFrame);
}