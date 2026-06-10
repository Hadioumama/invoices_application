#ifndef ADMINWINDOW_H
#define ADMINWINDOW_H

#include <QWidget>
#include <QStackedWidget>
#include "views/dashboardwidget.h"
#include "views/articleswidget.h"
#include "views/entreprise_config_widget.h"
#include "models/entreprise_config.h"

#include "dialogs/invoiceeditdialog.h"
#include "dialogs/client_edit_dialog.h"
#include "dialogs/invoicecreatedialog.h"
#include "dialogs/invoiceactiondialog.h"
#include "dialogs/paymentdialog.h"

#include <QTableView>
#include <QSqlQueryModel>
#include <QSqlTableModel>

class AdminWindow : public QWidget {
    Q_OBJECT
public:
    explicit AdminWindow(int adminId, QWidget *parent = nullptr);
    ~AdminWindow();

signals:
    void logoutRequested();

private slots:
    void onNavigateTo(const QString &page);
    void onLogout();
    void onConfigSaved(const EntrepriseConfig &cfg);

    // Invoice slots
    void onSearchInvoice();
    void onCreateInvoice();
    void onEditInvoice();
    void onDeleteInvoice();
    void onInvoiceActions();
    void onPaymentClicked();
    void onRefreshInvoices();
      void onExportInvoicePDF(); 
    // Client slots
    void refreshModel();
    void onSearch();
    void onAddClient();
    void onEditClient();
    void onDeleteClient();

private:
    void setupUI();
    QWidget* buildInvoicePage();
    QWidget* buildClientPage();

    // ── ORDRE CORRECT (doit matcher l'ordre d'initialisation dans le constructeur) ──
    int m_adminId;                          // 1
    DashboardWidget *m_dashboard;           // 2
    QStackedWidget *m_pageStack;            // 3
    ArticlesWidget *m_articles;             // 4
    QWidget *m_invoicePage;                 // 5
    QWidget *m_clientPage;                  // 6
    EntrepriseConfigWidget *m_entrepriseConfig;  // 7
    InvoiceCreateDialog *m_invoiceDialog;   // 8 ← APRÈS m_entrepriseConfig

    // Invoice page widgets
    QTableView *invoiceView;
    QSqlQueryModel *invoiceModel;
    QLineEdit *invoiceSearchEdit;
    QPushButton *invoiceSearchBtn;
    QPushButton *createInvoiceBtn;
    QPushButton *editInvoiceBtn;
    QPushButton *deleteInvoiceBtn;
    QPushButton *actionsBtn;
    QPushButton *paymentBtn;
    QPushButton *refreshInvoicesBtn;

    // Client page widgets
    QTableView *clientView;
    QSqlTableModel *clientModel;
    QLineEdit *searchEdit;
    QPushButton *searchButton;
    QPushButton *addButton;
    QPushButton *editButton;
    QPushButton *deleteButton;
    QPushButton *refreshButton;
};

#endif