#ifndef INVOICEEDITDIALOG_H
#define INVOICEEDITDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QComboBox>
#include <QDateEdit>
#include <QPushButton>
#include <QLabel>
#include <QTableWidget>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QVector>

struct EditLineItem {
    int id = 0;
    QString designation;
    int quantity = 1;
    double priceHT = 0;
    double taxRate = 20;
};
class InvoiceEditDialog : public QDialog
{
    Q_OBJECT
public:
    explicit InvoiceEditDialog(int invoiceId, QWidget *parent = nullptr);

private slots:
    void onSave();
    void onCancel();

private:
    void setupUI();
    void loadData();

    int m_invoiceId;

    QLineEdit *numeroEdit;
    QLineEdit *clientNomEdit;
    QLineEdit *clientAdresseEdit;
    QLineEdit *clientTelEdit;
    QLineEdit *clientEmailEdit;
    QComboBox *typeCombo;
    QComboBox *statusCombo;
    QDateEdit *dateCreationEdit;
    QDateEdit *dateEcheanceEdit;
    QPushButton *saveBtn;
    QPushButton *cancelBtn;
    QTableWidget *linesTable;
    QLineEdit *desigEdit;
    QSpinBox *qtyEdit;
    QDoubleSpinBox *prixEdit;
    QDoubleSpinBox *tvaEdit;
    QPushButton *addLineBtn;
    QPushButton *removeLineBtn;
    QLabel *totalLabel;
    QVector<EditLineItem> m_lines;
    
    void loadLines();
    void refreshLinesTable();
    void onAddLine();
    void onRemoveLine();
    void saveLines();
};

#endif