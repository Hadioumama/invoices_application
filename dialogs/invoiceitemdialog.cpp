#include "invoiceitemdialog.h"
#include "database/database.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QComboBox>
#include <QPushButton>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

InvoiceItemDialog::InvoiceItemDialog(QWidget *parent)
    : QDialog(parent)
{
    setupUI();
    loadArticles();
}

void InvoiceItemDialog::setupUI()
{
    setWindowTitle("Ajouter un Article à la Facture");
    setGeometry(100, 100, 400, 200);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // Article selection
    QHBoxLayout *articleLayout = new QHBoxLayout;
    articleLayout->addWidget(new QLabel("Article:"));
    articleCombo = new QComboBox;
    articleLayout->addWidget(articleCombo);
    mainLayout->addLayout(articleLayout);

    // Quantity
    QHBoxLayout *quantityLayout = new QHBoxLayout;
    quantityLayout->addWidget(new QLabel("Quantité:"));
    quantitySpinBox = new QSpinBox;
    quantitySpinBox->setMinimum(1);
    quantitySpinBox->setValue(1);
    quantityLayout->addWidget(quantitySpinBox);
    mainLayout->addLayout(quantityLayout);

    // Unit price
    QHBoxLayout *priceLayout = new QHBoxLayout;
    priceLayout->addWidget(new QLabel("Prix Unitaire HT:"));
    prixUnitaireSpinBox = new QDoubleSpinBox;
    prixUnitaireSpinBox->setDecimals(2);
    prixUnitaireSpinBox->setMaximum(999999);
    priceLayout->addWidget(prixUnitaireSpinBox);
    mainLayout->addLayout(priceLayout);

    // Tax rate
    QHBoxLayout *taxLayout = new QHBoxLayout;
    taxLayout->addWidget(new QLabel("Taux TVA (%):"));
    tauxTVASpinBox = new QDoubleSpinBox;
    tauxTVASpinBox->setDecimals(2);
    tauxTVASpinBox->setValue(20.0);
    tauxTVASpinBox->setMaximum(100);
    taxLayout->addWidget(tauxTVASpinBox);
    mainLayout->addLayout(taxLayout);

    // Buttons
    QHBoxLayout *buttonLayout = new QHBoxLayout;
    QPushButton *okBtn = new QPushButton("OK");
    QPushButton *cancelBtn = new QPushButton("Annuler");
    buttonLayout->addWidget(okBtn);
    buttonLayout->addWidget(cancelBtn);
    mainLayout->addLayout(buttonLayout);

    connect(okBtn, &QPushButton::clicked, this, &QDialog::accept);
    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
}

void InvoiceItemDialog::loadArticles()
{
    QSqlQuery query;
    query.exec("SELECT id, reference, designation, prix_ht, taux_tva FROM articles ORDER BY designation");

    if (!query.exec()) {
        qDebug() << "Erreur chargement articles:" << query.lastError().text();
        return;
    }

    while (query.next()) {
        int id = query.value(0).toInt();
        QString reference = query.value(1).toString();
        QString designation = query.value(2).toString();
        double prix = query.value(3).toDouble();

        QString text = QString("%1 (%2) - %3 €").arg(designation, reference, QString::number(prix, 'f', 2));
        articleCombo->addItem(text, id);
    }
}

int InvoiceItemDialog::getArticleId() const
{
    return articleCombo->currentData().toInt();
}

int InvoiceItemDialog::getQuantity() const
{
    return quantitySpinBox->value();
}

double InvoiceItemDialog::getPrixUnitaire() const
{
    return prixUnitaireSpinBox->value();
}

double InvoiceItemDialog::getTauxTVA() const
{
    return tauxTVASpinBox->value();
}

void InvoiceItemDialog::setArticleId(int id)
{
    int index = articleCombo->findData(id);
    if (index >= 0) articleCombo->setCurrentIndex(index);
}

void InvoiceItemDialog::setQuantity(int qty)
{
    quantitySpinBox->setValue(qty);
}

void InvoiceItemDialog::setPrixUnitaire(double prix)
{
    prixUnitaireSpinBox->setValue(prix);
}

void InvoiceItemDialog::setTauxTVA(double taux)
{
    tauxTVASpinBox->setValue(taux);
}