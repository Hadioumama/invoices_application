#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "dialogs/registerdialog.h"
#include <QMenuBar>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Création du menu Fichier
    QMenu *menuFichier = menuBar()->addMenu("&Fichier");
    QAction *actionInscription = new QAction("&Inscription client", this);
    menuFichier->addAction(actionInscription);
    connect(actionInscription, &QAction::triggered, this, &MainWindow::on_actionInscription_triggered);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_actionInscription_triggered()
{
    RegisterDialog dlg(this);
    if (dlg.exec() == QDialog::Accepted) {
        QMessageBox::information(this, "Succès", "Client enregistré avec succès !");
    }
}