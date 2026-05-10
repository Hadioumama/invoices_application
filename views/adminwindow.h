#ifndef ADMINWINDOW_H
#define ADMINWINDOW_H

#include "dialogs/invoicecreatedialog.h" 
#include <QMainWindow>
#include <QComboBox>
#include <QSqlTableModel>
#include <QSqlQueryModel>
#include <QTabWidget>

class QTableView;
class QPushButton;
class QLineEdit;
class InvoiceEditDialog;   

class AdminWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit AdminWindow(int adminId, QWidget *parent = nullptr);
signals:
    void logoutRequested();
private slots:
    void onAddClient();
    void onEditClient();
    void onDeleteClient();
    void refreshModel();
    void onSearch();
    void onLogout();  
    // FACTURES
    void onCreateInvoice();
    void onEditInvoice();
    void onDeleteInvoice();
    void onInvoiceActions();
    void onRefreshInvoices();
    void onSearchInvoice();
    void onPaymentClicked();  // ← AJOUTÉ

private:
    void setupUI();
      
    int m_adminId;
    QSqlQueryModel *invoiceModel;
    QSqlTableModel *clientModel;
    QComboBox *clientComboBox;
    QTableView *clientView;
    QPushButton *addButton, *editButton, *deleteButton, *refreshButton;
    QLineEdit *searchEdit;
    QPushButton *searchButton;
    
    // FACTURES
    QTabWidget *tabWidget;
    QTableView *invoiceView;
    QPushButton *createInvoiceBtn;
    QPushButton *editInvoiceBtn;
    QPushButton *deleteInvoiceBtn;
    QPushButton *actionsBtn;
    QPushButton *refreshInvoicesBtn;
    QPushButton *paymentBtn;        // ← AJOUTÉ
    QLineEdit *invoiceSearchEdit;
    QPushButton *invoiceSearchBtn;
    InvoiceCreateDialog *m_invoiceDialog;
};

#endif // ADMINWINDOW_H