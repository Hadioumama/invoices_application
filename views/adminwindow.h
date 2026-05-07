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
    explicit AdminWindow(QWidget *parent = nullptr);
    ~AdminWindow();

private slots:
    void onAddClient();
    void onEditClient();
    void onDeleteClient();
    void refreshModel();
    void onSearch();
    
    // NOUVELLES SLOTS POUR FACTURES
    void onCreateInvoice();
    void onEditInvoice();
    void onDeleteInvoice();
    void onInvoiceActions();
    void onRefreshInvoices();
    void onSearchInvoice();

private:
    void setupUI();

   QSqlQueryModel *invoiceModel;
   QSqlTableModel *clientModel;
   QComboBox *clientComboBox;
    QTableView *clientView;
    QPushButton *addButton, *editButton, *deleteButton, *refreshButton;
    QLineEdit *searchEdit;
    QPushButton *searchButton;
    
    // WIDGETS FACTURES
    QTabWidget *tabWidget;

    QTableView *invoiceView;
    QPushButton *createInvoiceBtn, *editInvoiceBtn, *deleteInvoiceBtn, *actionsBtn, *refreshInvoicesBtn;
    QLineEdit *invoiceSearchEdit;
    QPushButton *invoiceSearchBtn;
    InvoiceCreateDialog *m_invoiceDialog;
};

#endif // ADMINWINDOW_H