#include <QApplication>
#include <QMessageBox>
#include <QDebug>
#include "database/database.h"
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    if (!Database::instance().connect("facturation.db")) {
        QMessageBox::critical(nullptr, "Erreur", 
            "Impossible de se connecter à la base de données.");
        return -1;
    }
    Database::instance().initializeTables();

    MainWindow w;
    w.show();
    
    qDebug() << "MainWindow shown, starting exec...";
    int result = a.exec();
    qDebug() << "Application exiting with code:" << result;
    
    return result;
}