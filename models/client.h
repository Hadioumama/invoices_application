#ifndef CLIENT_H
#define CLIENT_H

#include <QString>

class Client {
public:
    int id = -1;
    QString nom;
    QString prenom;
    QString email;
    QString adresse;
    QString telephone;
    QString motDePasse;
    QString type;           // "personne" ou "entreprise"
    QString ice;
    QString nomEntreprise;

    bool isValid() const;
};

#endif