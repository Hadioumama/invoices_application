#ifndef PAYMENT_H
#define PAYMENT_H

#include <QString>
#include <QDate>
#include <QDateTime>

class Payment {
public:
    int id = -1;
    int factureId = -1;
    double montant = 0.0;
    QDate datePaiement;
    QString methode;      // "Espèce", "Virement", "Chèque", "Carte", "Autre"
    QString notes;
    QDateTime createdAt;

    bool isValid() const {
        return factureId > 0 && montant > 0 && !datePaiement.isNull();
    }
};

#endif // PAYMENT_H