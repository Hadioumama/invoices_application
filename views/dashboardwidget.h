#ifndef DASHBOARDWIDGET_H
#define DASHBOARDWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QTimer>
#include <QFrame>
#include <QPushButton>
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

    // Returns the 230px sidebar widget (to be placed in AdminWindow's root layout)
    QWidget* sidebarOnly() const { return sidebarWidget; }

    // Returns the right-hand dashboard content widget (goes into QStackedWidget)
    QWidget* contentArea() const { return m_contentArea; }

signals:
    void navigateTo(const QString &page);
    void logoutRequested();

private slots:
    void refreshData();

private:
    void setupUI();
    void setupSidebar();
    void setupStatCards();
    void setupCharts();
    void updateStats();
    void updateBarChart();
    void updatePieChart();

    // Sidebar
    QWidget     *sidebarWidget;
    QPushButton *activeNavBtn = nullptr;

    // Content area (right panel — exposed via contentArea())
    QWidget *m_contentArea;

    // Stat cards
    QLabel *totalCALabel;
    QLabel *facturesPayeesLabel;
    QLabel *facturesImpayeesLabel;
    QLabel *clientsTotalLabel;
    QLabel *facturesMoisLabel;
    QLabel *montantEnAttenteLabel;

    // Charts
    QChartView *barChartView;
    QChartView *pieChartView;
    QWidget    *cardsWidget;
    QWidget    *chartsWidget;

    // Timer
    QTimer *refreshTimer;
};

#endif