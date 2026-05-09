#ifndef PAYMENTDIALOG_H
#define PAYMENTDIALOG_H

#include <QDialog>

class QDoubleSpinBox;
class QDateEdit;
class QComboBox;
class QTextEdit;
class QPushButton;
class QLabel;
class QTableWidget;

class PaymentDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PaymentDialog(int invoiceId, QWidget *parent = nullptr);

private slots:
    void onAddPayment();
    void onCancelPayment();     // ← NOUVEAU : Annuler dernier paiement
    void refreshPaymentsList();
    void updateResteDisplay();

private:
    void setupUI();
    void loadInvoiceInfo();
    double getTotalPaid() const;
    void updateButtonsState();  // ← NOUVEAU : Gérer état des boutons
    
    int m_invoiceId;
    double m_totalTTC = 0.0;
    double m_totalPaid = 0.0;

    QDoubleSpinBox *montantSpinBox;
    QDateEdit *dateEdit;
    QComboBox *methodeCombo;
    QTextEdit *notesEdit;
    QPushButton *addBtn;
    QPushButton *cancelPaymentBtn;  // ← NOUVEAU
    QPushButton *closeBtn;
    
    QLabel *factureNumLabel;
    QLabel *totalTTCLabel;
    QLabel *dejaPayeLabel;
    QLabel *resteLabel;
    QLabel *statutLabel;
    
    QTableWidget *paymentsTable;
};

#endif // PAYMENTDIALOG_H