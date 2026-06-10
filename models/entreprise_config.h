#ifndef ENTREPRISE_CONFIG_H
#define ENTREPRISE_CONFIG_H

#include <QString>
#include <QColor>

struct EntrepriseConfig {
    QString nom;           // ← OBLIGATOIRE
    QString rib;           // ← OBLIGATOIRE
    QString logoPath;      // ← Optionnel
    QString signaturePath; // ← Optionnel
    QColor  themeCouleur = QColor("#2563EB");
    bool    configured = false;
    
        inline  bool isValid() const {
        return !nom.isEmpty() && !rib.isEmpty();
    }
};

#endif