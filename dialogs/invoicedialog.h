#ifndef INVOICEDIALOG_H
#define INVOICEDIALOG_H

#include <QDialog>
#include <QDate>
#include "models/invoice.h"

class QLineEdit;
class QDateEdit;
class QComboBox;
class QTableWidget;
class QPushButton;
class QDoubleSpinBox;
class QLabel;

class InvoiceDialog : public QDialog
{
    Q_OBJECT

public:
    explicit InvoiceDialog(QWidget *parent = nullptr);
    
    // For creating new invoice
    void setCreateMode();
    
    // For editing existing invoice
    void setEditMode(int invoiceId);

    // Get the invoice object
    Invoice getInvoice() const { return m_invoice; }

private slots:
    void onAddItem();
    void onRemoveItem();
    void onClientSelected();
    void onSave();
    void onCancel();

private:
    void setupUI();
    void loadInvoiceData();
    void loadClients();
    void loadInvoiceItems();
    void updateTotals();
    void calculateTotals();

    // UI components
    QLineEdit *numeroEdit;
    QComboBox *typeCombo;
    QComboBox *clientCombo;
    QDateEdit *dateCreationEdit;
    QDateEdit *dateEcheanceEdit;
    QDateEdit *dateValiditeEdit;
    QComboBox *statutCombo;
    QTableWidget *itemsTable;
    QLabel *totalHTLabel;
    QLabel *totalTVALabel;
    QLabel *totalTTCLabel;
    QPushButton *addItemBtn;
    QPushButton *removeItemBtn;
    QPushButton *saveBtn;
    QPushButton *cancelBtn;

    Invoice m_invoice;
    bool m_isEditMode;
    int m_invoiceId;
};

#endif // INVOICEDIALOG_H