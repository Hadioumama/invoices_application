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
    void onExportPDF();
    void onPrint();
    void onSendEmail();

private:
    void setupUI();
    
    int m_invoiceId;
    QPushButton *pdfBtn;
    QPushButton *printBtn;
    QPushButton *emailBtn;
    QPushButton *closeBtn;
    QLineEdit *emailEdit;
};

#endif // INVOICEACTIONDIALOG_H