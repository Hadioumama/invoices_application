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

    // ============================================
    // 1. TABLE CLIENTS
    // ============================================
    bool ok = query.exec(R"(
        CREATE TABLE IF NOT EXISTS clients (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            nom TEXT NOT NULL,
            prenom TEXT NOT NULL,
            email TEXT NOT NULL UNIQUE,
            mot_de_passe TEXT NOT NULL,
            adresse TEXT,
            telephone TEXT,
            type TEXT NOT NULL CHECK(type IN ('personne', 'entreprise', 'admin')),
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

    // ============================================
    // 2. TABLE ARTICLES
    // ============================================
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
    if (!ok) {
        qDebug() << "Erreur articles:" << query.lastError().text();
        return false;
    }

    // ============================================
    // 3. TABLE FACTURES
    // ============================================
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
    if (!ok) {
        qDebug() << "Erreur factures:" << query.lastError().text();
        return false;
    }

    // ============================================
    // 4. TABLE LIGNES_FACTURE
    // ============================================
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
    if (!ok) {
        qDebug() << "Erreur lignes_facture:" << query.lastError().text();
        return false;
    }

    // ============================================
    // 5. TABLE PAIEMENTS (NOUVEAU)
    // ============================================
    if (!createPaymentsTable()) {
        qDebug() << "Erreur création table paiements";
        return false;
    }

    // ============================================
    // 6. MIGRATION : Ajout colonne designation si absente
    // ============================================
    // SQLite ne supporte pas ALTER TABLE DROP COLUMN, mais ADD COLUMN est OK
    // On ignore l'erreur si la colonne existe déjà
    query.exec("ALTER TABLE lignes_facture ADD COLUMN designation TEXT DEFAULT ''");

    // ============================================
    // 7. INSERTION ADMIN PAR DÉFAUT
    // ============================================
    QSqlQuery insertAdmin;
    insertAdmin.prepare(R"(
        INSERT OR IGNORE INTO clients 
        (nom, prenom, email, mot_de_passe, type, role) 
        VALUES ('Admin', 'System', 'admin@facturation.com', 'admin123', 'admin', 'admin')
    )");
    if (!insertAdmin.exec()) {
        qDebug() << "Erreur insertion admin:" << insertAdmin.lastError().text();
    } else {
        qDebug() << "Admin inséré avec succès.";
    }

    return true;
}

bool Database::createPaymentsTable()
{
    QSqlQuery query(db);
    
    // ============================================
    // TABLE PAIEMENTS
    // ============================================
    QString createPayments = R"(
        CREATE TABLE IF NOT EXISTS paiements (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            facture_id INTEGER NOT NULL,
            montant REAL NOT NULL CHECK(montant > 0),
            date_paiement DATE DEFAULT CURRENT_DATE,
            methode TEXT DEFAULT 'Espèce' CHECK(methode IN ('Espèce', 'Virement', 'Chèque', 'Carte', 'Autre')),
            notes TEXT,
            created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
            FOREIGN KEY(facture_id) REFERENCES factures(id) ON DELETE CASCADE
        )
    )";
    
    if (!query.exec(createPayments)) {
        qDebug() << "Erreur création table paiements:" << query.lastError().text();
        return false;
    }
    
    // ============================================
    // INDEX pour performance
    // ============================================
    query.exec("CREATE INDEX IF NOT EXISTS idx_paiements_facture ON paiements(facture_id)");
    
    // ============================================
    // VUE pour faciliter les requêtes
    // ============================================
    QString createView = R"(
        CREATE VIEW IF NOT EXISTS v_facture_solde AS
        SELECT 
            f.id,
            f.numero,
            f.total_ttc,
            COALESCE(SUM(p.montant), 0) as montant_paye,
            f.total_ttc - COALESCE(SUM(p.montant), 0) as reste_a_payer,
            CASE 
                WHEN COALESCE(SUM(p.montant), 0) = 0 THEN 'Non payée'
                WHEN COALESCE(SUM(p.montant), 0) >= f.total_ttc THEN 'Payée'
                ELSE 'Partiellement payée'
            END as statut_paiement
        FROM factures f
        LEFT JOIN paiements p ON f.id = p.facture_id
        GROUP BY f.id
    )";
    
    if (!query.exec(createView)) {
        qDebug() << "Erreur création vue:" << query.lastError().text();
        // Non critique, on continue
    }
    
    // ============================================
    // TRIGGERS pour auto-update statut facture
    // ============================================
    // Supprimer anciens triggers si existants (pour permettre les mises à jour)
    query.exec("DROP TRIGGER IF EXISTS trg_after_payment_insert");
    query.exec("DROP TRIGGER IF EXISTS trg_after_payment_delete");
    query.exec("DROP TRIGGER IF EXISTS trg_after_payment_update");
    
    // Trigger après INSERT sur paiements
    QString triggerInsert = R"(
        CREATE TRIGGER trg_after_payment_insert
        AFTER INSERT ON paiements
        BEGIN
            UPDATE factures 
            SET statut = CASE
                WHEN (SELECT COALESCE(SUM(montant), 0) FROM paiements WHERE facture_id = NEW.facture_id) >= total_ttc 
                    THEN 'Payée'
                WHEN (SELECT COALESCE(SUM(montant), 0) FROM paiements WHERE facture_id = NEW.facture_id) > 0 
                    THEN 'Partiellement payée'
                ELSE statut
            END
            WHERE id = NEW.facture_id;
        END
    )";
    
    if (!query.exec(triggerInsert)) {
        qDebug() << "Erreur création trigger insert:" << query.lastError().text();
        return false;
    }
    
    // Trigger après DELETE sur paiements
    QString triggerDelete = R"(
        CREATE TRIGGER trg_after_payment_delete
        AFTER DELETE ON paiements
        BEGIN
            UPDATE factures 
            SET statut = CASE
                WHEN (SELECT COALESCE(SUM(montant), 0) FROM paiements WHERE facture_id = OLD.facture_id) >= total_ttc 
                    THEN 'Payée'
                WHEN (SELECT COALESCE(SUM(montant), 0) FROM paiements WHERE facture_id = OLD.facture_id) > 0 
                    THEN 'Partiellement payée'
                ELSE 'Envoyée'
            END
            WHERE id = OLD.facture_id;
        END
    )";
    
    if (!query.exec(triggerDelete)) {
        qDebug() << "Erreur création trigger delete:" << query.lastError().text();
        return false;
    }
    
    // Trigger après UPDATE sur paiements (si montant modifié)
    QString triggerUpdate = R"(
        CREATE TRIGGER trg_after_payment_update
        AFTER UPDATE OF montant ON paiements
        BEGIN
            UPDATE factures 
            SET statut = CASE
                WHEN (SELECT COALESCE(SUM(montant), 0) FROM paiements WHERE facture_id = NEW.facture_id) >= total_ttc 
                    THEN 'Payée'
                WHEN (SELECT COALESCE(SUM(montant), 0) FROM paiements WHERE facture_id = NEW.facture_id) > 0 
                    THEN 'Partiellement payée'
                ELSE 'Envoyée'
            END
            WHERE id = NEW.facture_id;
        END
    )";
    
    if (!query.exec(triggerUpdate)) {
        qDebug() << "Erreur création trigger update:" << query.lastError().text();
        return false;
    }
    
    qDebug() << "✅ Table paiements, vue et triggers créés avec succès";
    return true;
}