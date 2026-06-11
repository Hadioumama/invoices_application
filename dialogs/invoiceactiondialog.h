#ifndef INVOICEACTIONDIALOG_H
#define INVOICEACTIONDIALOG_H

#include <QDialog>

class QPushButton;
class QLineEdit;

class InvoiceActionDialog : public QDialog
{
    Q_OBJECT

public:
    explicit InvoiceActionDialog(int invoiceId, QWidget *parent = nullptr);

private slots:
    void onPrint();
    void onSendEmail();
    void onManagePayments();

private:
    void setupUI();

    int m_invoiceId;
    QPushButton *printBtn;
    QPushButton *emailBtn;
    QPushButton *closeBtn;
    QPushButton *paymentBtn;
    QLineEdit   *emailEdit;
};

#endif // INVOICEACTIONDIALOG_H