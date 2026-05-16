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
class QFrame;
class QScrollArea;

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
    void onClientSelected(int index);

public slots:
    void onArticleFromCatalog(int id, const QString &name, double price, double taxRate);
protected:
    void resizeEvent(QResizeEvent *event) override;
    void showEvent(QShowEvent *event) override;

private:
    QWidget *m_footer = nullptr;
    QScrollArea *m_scroll = nullptr;
private:
    void setupUI();
    void loadClients();
    void loadArticles();
    void loadInvoiceLines();
    void updateLineData();
    void refreshLineTable();
    void calculateTotals();

    // ===== HELPERS DE DESIGN =====
    QFrame* createCard();                                    // ← CHANGÉ : sans paramètres
    QLabel* createSectionTitle(const QString &title, const QString &icon);  // ← NOUVEAU
    QLineEdit* createStyledLineEdit(const QString &placeholder);
    QLabel* createFieldLabel(const QString &text);

    // Infos facture
    QLineEdit *numeroEdit;
    QComboBox *typeCombo;
    QComboBox *clientComboBox;
    QLineEdit *clientNomEdit;
    QDateEdit *dateCreationEdit;
    QDateEdit *dateEcheanceEdit;
    QLineEdit *clientAdresseEdit;
    QComboBox *statusCombo;
    QLineEdit *logoPathEdit;
    QLineEdit *signaturePathEdit;
    QPushButton *logoBtn;
    QLineEdit *clientTelEdit;
    QPushButton *signatureBtn;
    QLabel *logoPreview;
    QLineEdit *clientEmailEdit;
    QLabel *signaturePreview;
    QString m_logoPath;
    QString m_signaturePath;

    // Table lignes
    QTableWidget *linesTable;

    // Saisie ligne
    QComboBox *designationEdit;
    QSpinBox *quantitySpinBox;
    QDoubleSpinBox *priceHTSpinBox;
    QDoubleSpinBox *taxRateSpinBox;

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

#endif // INVOICECREATEDIALOG_H