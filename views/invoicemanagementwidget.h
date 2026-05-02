#ifndef INVOICEMANAGEMENTWIDGET_H
#define INVOICEMANAGEMENTWIDGET_H

#include <QWidget>
#include <QSqlTableModel>

class QTableView;
class QPushButton;
class QLineEdit;
class QDateEdit;
class QComboBox;

class InvoiceManagementWidget : public QWidget
{
    Q_OBJECT

public:
    explicit InvoiceManagementWidget(QWidget *parent = nullptr);
    ~InvoiceManagementWidget();

private slots:
    void onCreateInvoice();
    void onEditInvoice();
    void onDeleteInvoice();
    void onSearchByClient();
    void onSearchByDate();
    void onRefresh();
    void onChangeStatus();

private:
    void setupUI();
    void loadInvoices();

    QSqlTableModel *invoiceModel;
    QTableView *invoiceView;
    QPushButton *createBtn, *editBtn, *deleteBtn, *refreshBtn, *changeStatusBtn;
    QLineEdit *clientIdEdit;
    QDateEdit *dateEdit;
    QPushButton *searchClientBtn, *searchDateBtn;
    QComboBox *statusCombo;
};

#endif // INVOICEMANAGEMENTWIDGET_H