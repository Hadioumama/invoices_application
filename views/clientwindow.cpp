#include "clientwindow.h"
#include <QLabel>
#include <QVBoxLayout>

ClientWindow::ClientWindow(const QString &email, QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("Espace Client - FacturationApp");
    QWidget *central = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(central);
    layout->addWidget(new QLabel("Bienvenue " + email));
    layout->addWidget(new QLabel("Ici vous pourrez consulter vos factures."));
    setCentralWidget(central);
}