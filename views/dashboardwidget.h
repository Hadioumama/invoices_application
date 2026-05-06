#ifndef DASHBOARDWIDGET_H
#define DASHBOARDWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QTimer>
#include <QtCharts/QChartView>
#include <QtCharts/QBarSeries>
#include <QtCharts/QPieSeries>
#include <QtCharts/QBarSet>
#include <QtCharts/QBarCategoryAxis>
#include <QtCharts/QValueAxis>

class DashboardWidget : public QWidget
{
    Q_OBJECT

public:
    explicit DashboardWidget(QWidget *parent = nullptr);

private slots:
    void refreshData();

private:
    void setupUI();
    void setupStatCards();
    void setupCharts();
    void updateStats();
    void updateBarChart();
    void updatePieChart();

    // Cartes statistiques
    QLabel *totalCALabel;
    QLabel *facturesPayeesLabel;
    QLabel *facturesImpayeesLabel;
    QLabel *clientsTotalLabel;
    QLabel *facturesMoisLabel;
    QLabel *montantEnAttenteLabel;

    // Graphiques
    QChartView *barChartView;
    QChartView *pieChartView;
    QWidget *cardsWidget;
QWidget *chartsWidget;

    // Timer pour rafraîchissement auto
    QTimer *refreshTimer;
};

#endif