
#include "database.h"

Database& Database::instance() {
    static Database instance;
    return instance;
}

bool Database::connect(const QString &dbName) {
    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(dbName);
    if (!db.open()) {
        qDebug() << "Erreur ouverture SQLite:" << db.lastError().text();
        return false;
    }
    qDebug() << "Connexion SQLite réussie.";
    return true;
}

void Database::disconnect() {
    db.close();
}

bool Database::initializeTables() {
    QSqlQuery query;

    // Table clients
    bool ok = query.exec(R"(
        CREATE TABLE IF NOT EXISTS clients (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            nom TEXT NOT NULL,
            prenom TEXT NOT NULL,
            email TEXT NOT NULL UNIQUE,
           mot_de_passe TEXT NOT NULL, 
            adresse TEXT,
            telephone TEXT,
            type TEXT NOT NULL CHECK(type IN ('personne', 'entreprise')),
            ice TEXT,
            nom_entreprise TEXT,
            date_creation DATETIME DEFAULT CURRENT_TIMESTAMP
        )
    )");
    if (!ok) qDebug() << "Erreur clients:" << query.lastError().text();

    // Table articles (pour la suite)
    ok = query.exec(R"(
        CREATE TABLE IF NOT EXISTS articles (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            reference TEXT UNIQUE NOT NULL,
            designation TEXT NOT NULL,
            prix_ht REAL NOT NULL,
            taux_tva REAL DEFAULT 20.0,
            stock INTEGER DEFAULT 0
        )
    )");
    if (!ok) qDebug() << "Erreur articles:" << query.lastError().text();

    // Table factures (pour la suite)
    ok = query.exec(R"(
        CREATE TABLE IF NOT EXISTS factures (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            numero TEXT UNIQUE NOT NULL,
            type TEXT NOT NULL CHECK(type IN ('Devis', 'Facture')),
            client_id INTEGER NOT NULL,
            date_creation DATETIME DEFAULT CURRENT_TIMESTAMP,
            date_echeance DATE,
            date_validite DATE,
            total_ht REAL,
            total_tva REAL,
            total_ttc REAL,
            statut TEXT DEFAULT 'Brouillon',
            facture_source_id INTEGER,
            FOREIGN KEY(client_id) REFERENCES clients(id),
            FOREIGN KEY(facture_source_id) REFERENCES factures(id)
        )
    )");
    if (!ok) qDebug() << "Erreur factures:" << query.lastError().text();

    // Table lignes_facture
    ok = query.exec(R"(
        CREATE TABLE IF NOT EXISTS lignes_facture (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            facture_id INTEGER NOT NULL,
            article_id INTEGER NOT NULL,
            quantite INTEGER NOT NULL,
            prix_unitaire_ht REAL NOT NULL,
            taux_tva REAL NOT NULL,
            FOREIGN KEY(facture_id) REFERENCES factures(id),
            FOREIGN KEY(article_id) REFERENCES articles(id)
        )
    )");
    if (!ok) qDebug() << "Erreur lignes_facture:" << query.lastError().text();

    return true;
}