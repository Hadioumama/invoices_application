#ifndef ADMINWINDOW_H
#define ADMINWINDOW_H

#include <QWidget>  // ✅ Reste QWidget pour QStackedWidget
#include <QTabWidget>
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

    int m_adminId;
    
    // Widgets factures
    QLineEdit *invoiceSearchEdit;
    QPushButton *invoiceSearchBtn;
    QPushButton *createInvoiceBtn;
    QPushButton *editInvoiceBtn;
    QPushButton *deleteInvoiceBtn;
    QPushButton *actionsBtn;
    QPushButton *paymentBtn;
    QPushButton *refreshInvoicesBtn;
    QTableView *invoiceView;
    QSqlQueryModel *invoiceModel;
    
    // Widgets clients
    QLineEdit *searchEdit;
    QPushButton *searchButton;
    QPushButton *addButton;
    QPushButton *editButton;
    QPushButton *deleteButton;
    QPushButton *refreshButton;
    QTableView *clientView;
    QSqlTableModel *clientModel;
    
    // Dialog
    InvoiceCreateDialog *m_invoiceDialog;
};

#endif // ADMINWINDOW_H