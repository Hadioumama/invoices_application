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
    light.setColor(QPalette::Window,          QColor(247, 250, 252));
    light.setColor(QPalette::WindowText,      QColor( 26,  32,  44));
    light.setColor(QPalette::Base,            QColor(255, 255, 255));
    light.setColor(QPalette::AlternateBase,   QColor(237, 242, 247));
    light.setColor(QPalette::Text,            QColor( 26,  32,  44));
    light.setColor(QPalette::Button,          QColor(237, 242, 247));
    light.setColor(QPalette::ButtonText,      QColor( 26,  32,  44));
    light.setColor(QPalette::PlaceholderText, QColor(160, 174, 192));
    light.setColor(QPalette::Highlight,       QColor( 49, 130, 206));
    light.setColor(QPalette::HighlightedText, QColor(255, 255, 255));
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