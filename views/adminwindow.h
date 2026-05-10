#ifndef ADMINWINDOW_H
#define ADMINWINDOW_H

#include <QWidget>  // ✅ PAS QMainWindow !
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTableView>
#include <QPushButton>
#include <QLineEdit>
#include <QTabWidget>
#include <QSqlQueryModel>
#include <QSqlTableModel>

// Forward declarations
class ArticlesWidget;
class DashboardWidget;
class InvoiceCreateDialog;
class PaymentDialog;

class AdminWindow : public QWidget  // ✅ QWidget, pas QMainWindow
{
    Q_OBJECT
public:
    explicit AdminWindow(int adminId, QWidget *parent = nullptr);
    ~AdminWindow();

signals:
    void logoutRequested();

private slots:
    // Factures
    void onCreateInvoice();
    void onEditInvoice();
    void onDeleteInvoice();
    void onInvoiceActions();
    void onPaymentClicked();
    void onRefreshInvoices();
    void onSearchInvoice();
    
    // Clients
    void onAddClient();
    void onEditClient();
    void onDeleteClient();
    void refreshModel();
    void onSearch();
    
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