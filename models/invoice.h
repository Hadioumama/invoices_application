#ifndef INVOICE_H
#define INVOICE_H

#include <QString>
#include <QDate>

class Invoice
{
public:
    int id = -1;
    QString numero;
    QString type;           // "Devis" ou "Facture"
    int clientId = -1;
    QDate dateCreation;
    QDate dateEcheance;
    QDate dateValidite;
    double totalHT = 0.0;
    double totalTVA = 0.0;
    double totalTTC = 0.0;
    QString statut;         // "Brouillon", "Validée", "Payée", "Annulée"
    int factureSourceId = -1;

    bool isValid() const;
    QString getStatusString() const;
    double getMontantPaye() const;
    double getResteAPayer() const;
    bool isFullyPaid() const;
};

#endif // INVOICE_H