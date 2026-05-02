#ifndef INVOICESENDERDIALOG_H
#define INVOICESENDERDIALOG_H

#include <QDialog>

class QLineEdit;
class QPushButton;
class QCheckBox;
class QLabel;

class InvoiceSenderDialog : public QDialog
{
    Q_OBJECT

public:
    explicit InvoiceSenderDialog(int invoiceId, const QString &clientEmail, 
                                const QString &invoiceNumber, QWidget *parent = nullptr);
    
    bool wasSent() const { return m_wasSent; }

private slots:
    void onSendEmail();
    void onCancel();

private:
    void setupUI();
    bool sendEmail();

    int m_invoiceId;
    QString m_clientEmail;
    QString m_invoiceNumber;
    bool m_wasSent;

    QLineEdit *emailEdit;
    QCheckBox *confirmCheckBox;
    QPushButton *sendBtn;
    QPushButton *cancelBtn;
    QLabel *statusLabel;
};

#endif // INVOICESENDERDIALOG_H