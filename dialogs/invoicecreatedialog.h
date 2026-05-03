#ifndef INVOICECREATEDIALOG_H
#define INVOICECREATEDIALOG_H
#include <QDialog>
#include <QVector>

class QLineEdit;
class QDateEdit;
class QComboBox;
class QTableWidget;
class QPushButton;
class QLabel;
class QDoubleSpinBox;
class QSpinBox;

struct InvoiceLineItem {
    int articleId = 0;
    QString designation;
    int quantity = 1;
    double priceHT = 0;
    double taxRate = 20;
};

class InvoiceCreateDialog : public QDialog
{
    Q_OBJECT
public:
    explicit InvoiceCreateDialog(int invoiceId = -1, QWidget *parent = nullptr);
    int getInvoiceId() const { return m_invoiceId; }
    QString getLogoPath() const { return m_logoPath; }
QString getSignaturePath() const { return m_signaturePath; }
private slots:
    void onAddLine();
    void onRemoveLine();
    void onEditLine();
    void onArticleSelected(int index);
    void onLineDataChanged();
    void onSave();
    void onCancel();

private:
    void setupUI();
    void loadClients();
    void loadArticles();
    void loadInvoiceLines();
    void updateLineData();
    void refreshLineTable();
    void calculateTotals();

    // Infos facture
    QLineEdit *numeroEdit;
    QLineEdit *clientNomEdit;      // ← nom client libre
    QLineEdit *clientAdresseEdit;  // ← adresse client
    QLineEdit *clientTelEdit;      // ← téléphone
    QLineEdit *clientEmailEdit;    // ← email client
    QComboBox *typeCombo;
    QDateEdit *dateCreationEdit;
    QDateEdit *dateEcheanceEdit;
    QComboBox *statusCombo;
    QLineEdit *logoPathEdit;
    QLineEdit *signaturePathEdit;
    QPushButton *logoBtn;
    QPushButton *signatureBtn;
    QLabel *logoPreview;
    QLabel *signaturePreview;
    QString m_logoPath;
QString m_signaturePath;
    // Table lignes
    QTableWidget *linesTable;

    // Saisie ligne
    QLineEdit *designationEdit;
    QSpinBox *quantitySpinBox;
    QDoubleSpinBox *priceHTSpinBox;
    QDoubleSpinBox *taxRateSpinBox;
    QComboBox *clientComboBox;
   


    // Totaux
    QLabel *totalHTLabel;
    QLabel *totalTVALabel;
    QLabel *totalTTCLabel;

    // Boutons
    QPushButton *addLineBtn;
    QPushButton *removeLineBtn;
    QPushButton *saveBtn;
    QPushButton *cancelBtn;

    int m_invoiceId;
    bool m_isEditMode;
    QVector<InvoiceLineItem> m_lineItems;
};
#endif