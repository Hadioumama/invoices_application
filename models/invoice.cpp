#include "invoice.h"

bool Invoice::isValid() const
{
    return clientId > 0 && !numero.isEmpty() && !type.isEmpty();
}

QString Invoice::getStatusString() const
{
    static const QMap<QString, QString> statusMap = {
        {"Brouillon", "Brouillon"},
        {"Validée", "Validée"},
        {"Payée", "Payée"},
        {"Annulée", "Annulée"}
    };
    return statusMap.value(statut, "Inconnu");
}