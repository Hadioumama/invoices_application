#include <QApplication>
#include "mainwindow.h"
#include "database/database.h"


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    if (!Database::instance().connect("facturation.db")) {
        return -1;
    }
    Database::instance().initializeTables();

    MainWindow w;
    w.show();
    return a.exec();
}