#include <QApplication>
#include <QMessageBox>
#include <QStyleFactory>  // ← AJOUTE CET INCLUDE
#include <QPalette> 
#include <QDebug>
#include "database/database.h"
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
 a.setStyle(QStyleFactory::create("Fusion"));
    QPalette light;
    light.setColor(QPalette::Window,          QColor("#F7FAFC"));
    light.setColor(QPalette::WindowText,      QColor("#1A202C"));
    light.setColor(QPalette::Base,            QColor("#FFFFFF"));
    light.setColor(QPalette::AlternateBase,   QColor("#EDF2F7"));
    light.setColor(QPalette::Text,            QColor("#1A202C"));
    light.setColor(QPalette::Button,          QColor("#EDF2F7"));
    light.setColor(QPalette::ButtonText,      QColor("#1A202C"));
    light.setColor(QPalette::PlaceholderText, QColor("#A0AEC0"));
    light.setColor(QPalette::Highlight,       QColor("#3182CE"));
    light.setColor(QPalette::HighlightedText, QColor("#FFFFFF"));
    a.setPalette(light);
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