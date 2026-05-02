#ifndef INVOICEITEMDIALOG_H
#define INVOICEITEMDIALOG_H

#include <QDialog>

class QLineEdit;
class QSpinBox;
class QDoubleSpinBox;
class QComboBox;

class InvoiceItemDialog : public QDialog
{
    Q_OBJECT

public:
    explicit InvoiceItemDialog(QWidget *parent = nullptr);
    
    // Getters
    int getArticleId() const;
    int getQuantity() const;
    double getPrixUnitaire() const;
    double getTauxTVA() const;

    // Setters
    void setArticleId(int id);
    void setQuantity(int qty);
    void setPrixUnitaire(double prix);
    void setTauxTVA(double taux);

private:
    void setupUI();
    void loadArticles();

    QComboBox *articleCombo;
    QSpinBox *quantitySpinBox;
    QDoubleSpinBox *prixUnitaireSpinBox;
    QDoubleSpinBox *tauxTVASpinBox;
};

#endif // INVOICEITEMDIALOG_H