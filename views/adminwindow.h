#ifndef ADMINWINDOW_H
#define ADMINWINDOW_H

#include <QWidget>
#include <QStackedWidget>
#include <QTableView>
#include <QSqlQueryModel>
#include <QSqlTableModel>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>

class DashboardWidget;
class ArticlesWidget;
class InvoiceCreateDialog;

class AdminWindow : public QWidget
{
    Q_OBJECT

public:
    explicit AdminWindow(int adminId, QWidget *parent = nullptr);
    ~AdminWindow();

signals:
    void logoutRequested();

private slots:
    // Navigation
    void onNavigateTo(const QString &page);

    // Factures
    void onSearchInvoice();
    void onCreateInvoice();
    void onEditInvoice();
    void onDeleteInvoice();
    void onInvoiceActions();
    void onPaymentClicked();
    void onRefreshInvoices();

    // Clients
    void refreshModel();
    void onSearch();
    void onAddClient();
    void onEditClient();
    void onDeleteClient();

    // Logout
    void onLogout();

private:
    void setupUI();
    QWidget* buildInvoicePage();
    QWidget* buildClientPage();
    QWidget* buildPlaceholderPage(const QString &icon,
                                  const QString &title,
                                  const QString &subtitle);

    int m_adminId;

    // Core layout
    DashboardWidget  *m_dashboard;
    ArticlesWidget   *m_articles;
    QStackedWidget   *m_pageStack;   // centre droit

    // Pages
    QWidget *m_invoicePage;
    QWidget *m_clientPage;
    QWidget *m_rapportsPage;
    QWidget *m_parametresPage;

    // ── Factures widgets ─────────────────────────────────────────────────────
    QLineEdit    *invoiceSearchEdit;
    QPushButton  *invoiceSearchBtn;
    QPushButton  *createInvoiceBtn;
    QPushButton  *editInvoiceBtn;
    QPushButton  *deleteInvoiceBtn;
    QPushButton  *actionsBtn;
    QPushButton  *paymentBtn;
    QPushButton  *refreshInvoicesBtn;
    QTableView   *invoiceView;
    QSqlQueryModel *invoiceModel;

    // ── Clients widgets ──────────────────────────────────────────────────────
    QLineEdit    *searchEdit;
    QPushButton  *searchButton;
    QPushButton  *addButton;
    QPushButton  *editButton;
    QPushButton  *deleteButton;
    QPushButton  *refreshButton;
    QTableView   *clientView;
    QSqlTableModel *clientModel;

    // Dialog
    InvoiceCreateDialog *m_invoiceDialog;
};

#endif // ADMINWINDOW_H