#ifndef DATABASE_H
#define DATABASE_H

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

class Database {
public:
    static Database& instance();
    bool connect(const QString &dbName = "facturation.db");
    void disconnect();
    QSqlDatabase getConnection() const { return db; }
    bool initializeTables();
     bool createPaymentsTable();
private:
    Database() = default;
    QSqlDatabase db;
};

#endif