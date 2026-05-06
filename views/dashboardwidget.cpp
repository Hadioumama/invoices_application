#include "dashboardwidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QSqlQuery>
#include <QDate>
#include <QFrame>
#include <QtCharts/QChart>
#include <QtCharts/QBarSeries>
#include <QtCharts/QBarSet>
#include <QtCharts/QPieSeries>
#include <QtCharts/QPieSlice>
#include <QtCharts/QBarCategoryAxis>
#include <QtCharts/QValueAxis>

DashboardWidget::DashboardWidget(QWidget *parent)
    : QWidget(parent)
{
    setupUI();

    // Timer rafraîchissement toutes les 30 secondes
    refreshTimer = new QTimer(this);
    connect(refreshTimer, &QTimer::timeout,
            this, &DashboardWidget::refreshData);
    refreshTimer->start(30000);

    // Chargement initial
    refreshData();
}

void DashboardWidget::setupUI()
{
    QVBoxLayout *outerLayout = new QVBoxLayout(this);
    outerLayout->setContentsMargins(0, 0, 0, 0);

    QScrollArea *scroll = new QScrollArea;
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    QWidget *container = new QWidget;
    QVBoxLayout *mainLayout = new QVBoxLayout(container);
    mainLayout->setSpacing(16);
    mainLayout->setContentsMargins(16, 16, 16, 16);

    // Titre
    QLabel *title = new QLabel("📊 Tableau de Bord");
    title->setStyleSheet(
        "font-size:20px;font-weight:bold;color:#1B2A3B;"
        "padding-bottom:8px;");
    mainLayout->addWidget(title);

    setupStatCards();
    mainLayout->addWidget(cardsWidget);

    setupCharts();
    mainLayout->addWidget(chartsWidget);

    scroll->setWidget(container);
    outerLayout->addWidget(scroll);
}
void DashboardWidget::setupStatCards()
{
    cardsWidget = new QWidget;
    QGridLayout *grid = new QGridLayout(cardsWidget);
    grid->setSpacing(12);

    auto makeCard = [](const QString &title,
                       const QString &color,
                       const QString &icon,
                       QLabel *&valueLabel) -> QFrame* {
        QFrame *card = new QFrame;
        card->setMinimumHeight(110);
        card->setStyleSheet(QString(
            "QFrame {"
            "  background: white;"
            "  border-radius: 10px;"
            "  border: 1px solid #E2E8F0;"
            "  border-left: 5px solid %1;"
            "}"
        ).arg(color));

        QVBoxLayout *layout = new QVBoxLayout(card);
        layout->setContentsMargins(16, 12, 16, 12);

        QLabel *iconLabel = new QLabel(icon + "  " + title);
        iconLabel->setStyleSheet(
            "font-size:11px;color:#718096;"
            "font-weight:bold;text-transform:uppercase;"
            "letter-spacing:1px;border:none;");

        valueLabel = new QLabel("--");
        valueLabel->setStyleSheet(QString(
            "font-size:26px;font-weight:900;color:%1;border:none;"
        ).arg(color));

        layout->addWidget(iconLabel);
        layout->addWidget(valueLabel);
        return card;
    };

    // 6 cartes
    grid->addWidget(
        makeCard("Chiffre d'Affaires", "#2B6CB0",
                 "💰", totalCALabel), 0, 0);
    grid->addWidget(
        makeCard("Factures Payées", "#27AE60",
                 "✅", facturesPayeesLabel), 0, 1);
    grid->addWidget(
        makeCard("Factures Impayées", "#E53E3E",
                 "⚠️", facturesImpayeesLabel), 0, 2);
    grid->addWidget(
        makeCard("Total Clients", "#805AD5",
                 "👥", clientsTotalLabel), 1, 0);
    grid->addWidget(
        makeCard("Factures ce Mois", "#DD6B20",
                 "📅", facturesMoisLabel), 1, 1);
    grid->addWidget(
        makeCard("Montant en Attente", "#C05621",
                 "⏳", montantEnAttenteLabel), 1, 2);
}

void DashboardWidget::setupCharts()
{
    chartsWidget = new QWidget;
    QHBoxLayout *layout = new QHBoxLayout(chartsWidget);
    layout->setSpacing(12);

    // Graphique barres — CA par mois
    QGroupBox *barGroup = new QGroupBox(
        "📈 Chiffre d'Affaires par Mois");
    QVBoxLayout *barLayout = new QVBoxLayout(barGroup);
    barChartView = new QChartView;
    barChartView->setMinimumHeight(280);
    barChartView->setRenderHint(QPainter::Antialiasing);
    barLayout->addWidget(barChartView);
    layout->addWidget(barGroup, 6);

    // Graphique camembert — répartition statuts
    QGroupBox *pieGroup = new QGroupBox(
        "🥧 Répartition des Factures");
    QVBoxLayout *pieLayout = new QVBoxLayout(pieGroup);
    pieChartView = new QChartView;
    pieChartView->setMinimumHeight(280);
    pieChartView->setRenderHint(QPainter::Antialiasing);
    pieLayout->addWidget(pieChartView);
    layout->addWidget(pieGroup, 4);
}

void DashboardWidget::refreshData()
{
    updateStats();
    updateBarChart();
    updatePieChart();
}

void DashboardWidget::updateStats()
{
    QSqlQuery q;

    // Total CA (factures payées)
    q.exec("SELECT COALESCE(SUM(total_ttc),0) FROM factures "
           "WHERE statut='Payée'");
    if (q.next())
        totalCALabel->setText(
            QString::number(q.value(0).toDouble(), 'f', 2) + " MAD");

    // Factures payées
    q.exec("SELECT COUNT(*) FROM factures WHERE statut='Payée'");
    if (q.next())
        facturesPayeesLabel->setText(
            QString::number(q.value(0).toInt()));

    // Factures impayées
    q.exec("SELECT COUNT(*) FROM factures "
           "WHERE statut IN ('Brouillon','Envoyée')");
    if (q.next())
        facturesImpayeesLabel->setText(
            QString::number(q.value(0).toInt()));

    // Total clients
    q.exec("SELECT COUNT(*) FROM clients WHERE role='client'");
    if (q.next())
        clientsTotalLabel->setText(
            QString::number(q.value(0).toInt()));

    // Factures ce mois
    QString mois = QDate::currentDate().toString("yyyy-MM");
    q.prepare("SELECT COUNT(*) FROM factures "
              "WHERE strftime('%Y-%m', date_creation) = ?");
    q.addBindValue(mois);
    q.exec();
    if (q.next())
        facturesMoisLabel->setText(
            QString::number(q.value(0).toInt()));

    // Montant en attente
    q.exec("SELECT COALESCE(SUM(total_ttc),0) FROM factures "
           "WHERE statut IN ('Brouillon','Envoyée')");
    if (q.next())
        montantEnAttenteLabel->setText(
            QString::number(q.value(0).toDouble(), 'f', 2) + " MAD");
}

void DashboardWidget::updateBarChart()
{
    QBarSet *set = new QBarSet("CA (MAD)");
    set->setColor(QColor("#2B6CB0"));
    set->setBorderColor(QColor("#1A365D"));

    QStringList moisLabels;
    double maxVal = 0;

    for (int i = 5; i >= 0; i--) {
        QDate date = QDate::currentDate().addMonths(-i);
        QString moisStr = date.toString("yyyy-MM");
        moisLabels << date.toString("MMM yyyy");

        QSqlQuery q;
        q.prepare("SELECT COALESCE(SUM(total_ttc),0) "
                  "FROM factures "
                  "WHERE strftime('%Y-%m', date_creation) = ?");
        q.addBindValue(moisStr);
        q.exec();
        double val = q.next() ? q.value(0).toDouble() : 0;
        *set << val;
        if (val > maxVal) maxVal = val;
    }

    QBarSeries *series = new QBarSeries;
    series->append(set);
    series->setBarWidth(0.6);

    QChart *chart = new QChart;
    chart->addSeries(series);
    chart->setAnimationOptions(QChart::SeriesAnimations);
    chart->legend()->hide();
    chart->setBackgroundBrush(Qt::white);
    chart->setContentsMargins(0, 0, 0, 0);

    // AxeX avec marges suffisantes
    QBarCategoryAxis *axisX = new QBarCategoryAxis;
    axisX->append(moisLabels);
    axisX->setLabelsAngle(-30);  // ← inclinaison pour lisibilité
    axisX->setLabelsColor(QColor("#4A5568"));
    axisX->setGridLineVisible(false);
    chart->addAxis(axisX, Qt::AlignBottom);
    series->attachAxis(axisX);

    // AxeY
    QValueAxis *axisY = new QValueAxis;
    axisY->setLabelFormat("%.0f");
    axisY->setRange(0, maxVal > 0 ? maxVal * 1.2 : 100);
    axisY->setGridLineColor(QColor("#EDF2F7"));
    axisY->setLabelsColor(QColor("#4A5568"));
    chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisY);

    // Marges internes du graphique
    chart->setMargins(QMargins(10, 10, 10, 20));

    barChartView->setChart(chart);
    barChartView->setMinimumHeight(300);
    barChartView->setRenderHint(QPainter::Antialiasing);
}
void DashboardWidget::updatePieChart()
{
    QPieSeries *series = new QPieSeries;

    struct StatutInfo {
        QString label;
        QString color;
        QString query;
    };

    QVector<StatutInfo> statuts = {
        {"Payées",    "#27AE60",
         "SELECT COUNT(*) FROM factures WHERE statut='Payée'"},
        {"Envoyées",  "#3182CE",
         "SELECT COUNT(*) FROM factures WHERE statut='Envoyée'"},
        {"Brouillon", "#A0AEC0",
         "SELECT COUNT(*) FROM factures WHERE statut='Brouillon'"},
        {"Annulées",  "#E53E3E",
         "SELECT COUNT(*) FROM factures WHERE statut='Annulée'"},
    };

    bool hasData = false;
    for (const auto &s : statuts) {
        QSqlQuery q;
        q.exec(s.query);
        if (q.next()) {
            int count = q.value(0).toInt();
            if (count > 0) {
                QPieSlice *slice = series->append(
                    s.label + " (" +
                    QString::number(count) + ")", count);
                slice->setColor(QColor(s.color));
                slice->setLabelVisible(true);
                slice->setLabelColor(QColor(s.color));
                hasData = true;
            }
        }
    }

    if (!hasData)
        series->append("Aucune facture", 1);

    series->setHoleSize(0.35); // donut style

    QChart *chart = new QChart;
    chart->addSeries(series);
    chart->setTitle("");
    chart->setAnimationOptions(QChart::SeriesAnimations);
    chart->legend()->setAlignment(Qt::AlignBottom);
    chart->setBackgroundBrush(Qt::white);

    pieChartView->setChart(chart);
}