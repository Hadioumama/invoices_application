#include "database.h"
#include <QCryptographicHash>
#include <QDebug>

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

    // 1. Table clients (inclut la colonne role dès la création)
    bool ok = query.exec(R"(
    CREATE TABLE IF NOT EXISTS clients (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        nom TEXT NOT NULL,
        prenom TEXT NOT NULL,
        email TEXT NOT NULL UNIQUE,
        mot_de_passe TEXT NOT NULL,
        adresse TEXT,
        telephone TEXT,
        type TEXT NOT NULL CHECK(type IN ('personne', 'entreprise','admin')),
        ice TEXT,
        nom_entreprise TEXT,
        role TEXT DEFAULT 'client',
        date_creation DATETIME DEFAULT CURRENT_TIMESTAMP,
        signature_path TEXT,
        logo_path TEXT
    )
)");
    if (!ok) {
        qDebug() << "Erreur création table clients:" << query.lastError().text();
        return false;
    }
//unité:kg,m,L,boite 
ok = query.exec(R"(
    CREATE TABLE IF NOT EXISTS articles (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        reference TEXT UNIQUE NOT NULL,
        designation TEXT NOT NULL,
        prix_ht REAL NOT NULL,
        taux_tva REAL DEFAULT 20.0,
        stock INTEGER DEFAULT 0,
        unite TEXT DEFAULT 'unité',
        description TEXT
    )
)");
if (!ok) qDebug() << "Erreur articles:" << query.lastError().text();

   ok = query.exec(R"(
    CREATE TABLE IF NOT EXISTS factures (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        numero TEXT UNIQUE NOT NULL,
        type TEXT NOT NULL CHECK(type IN ('Devis', 'Facture')),
        client_id INTEGER,
        client_nom TEXT,
        client_adresse TEXT,
        client_tel TEXT,
        client_email TEXT,
        date_creation DATETIME DEFAULT CURRENT_TIMESTAMP,
        date_echeance DATE,
        date_validite DATE,
        total_ht REAL DEFAULT 0,
        total_tva REAL DEFAULT 0,
        total_ttc REAL DEFAULT 0,
        statut TEXT DEFAULT 'Brouillon',
        facture_source_id INTEGER
    )
)");
    if (!ok) qDebug() << "Erreur factures:" << query.lastError().text();

ok = query.exec(R"(
    CREATE TABLE IF NOT EXISTS lignes_facture (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        facture_id INTEGER NOT NULL,
        article_id INTEGER,
        designation TEXT NOT NULL,
        quantite INTEGER NOT NULL,
        prix_unitaire_ht REAL NOT NULL,
        taux_tva REAL NOT NULL,
        FOREIGN KEY(facture_id) REFERENCES factures(id)
    )
)");
// Ajoute la colonne designation si elle n'existe pas
query.exec("ALTER TABLE lignes_facture ADD COLUMN designation TEXT DEFAULT ''");
if (!ok) qDebug() << "Erreur lignes_facture:" << query.lastError().text();
 QSqlQuery insertAdmin;
insertAdmin.prepare("INSERT OR IGNORE INTO clients "
                    "(nom, prenom, email, mot_de_passe, type, role) "
                    "VALUES ('Admin', 'System', 'admin@facturation.com', 'admin123', 'admin', 'admin')");
if (!insertAdmin.exec()) {
    qDebug() << "Erreur insertion admin:" << insertAdmin.lastError().text();
} else {
    qDebug() << "Admin inséré avec succès.";
}
return true;
}