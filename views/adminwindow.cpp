#include "adminwindow.h"
#include "dialogs/client_edit_dialog.h"
#include <QTableView>
#include <QHeaderView>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QSqlQuery>
#include <QSqlError>
#include <QLineEdit>

#include <QLabel>

AdminWindow::AdminWindow(QWidget *parent) : QMainWindow(parent)
{
    setupUI();
    refreshModel();
}

AdminWindow::~AdminWindow() {}

void AdminWindow::setupUI()
{
    QWidget *central = new QWidget(this);
    setCentralWidget(central);
    QVBoxLayout *mainLayout = new QVBoxLayout(central);
     QHBoxLayout *searchLayout = new QHBoxLayout;

    searchLayout->addWidget(new QLabel("Rechercher par prénom :"));
    searchEdit = new QLineEdit;
    searchEdit->setPlaceholderText("Saisir le prénom...");
    searchButton = new QPushButton("Rechercher");
    QPushButton *resetButton = new QPushButton("Réinitialiser");
    searchLayout->addWidget(searchEdit);
    searchLayout->addWidget(searchButton);
    searchLayout->addWidget(resetButton);
    mainLayout->addLayout(searchLayout);

    clientModel = new QSqlTableModel(this);
    clientModel->setTable("clients");
    clientModel->setEditStrategy(QSqlTableModel::OnManualSubmit);
    clientModel->select();

    clientView = new QTableView;
    clientView->setModel(clientModel);
    clientView->setSelectionBehavior(QAbstractItemView::SelectRows);
    clientView->setSelectionMode(QAbstractItemView::SingleSelection);
    clientView->horizontalHeader()->setStretchLastSection(true);
    mainLayout->addWidget(clientView);

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    addButton = new QPushButton("Ajouter");
    editButton = new QPushButton("Modifier");
    deleteButton = new QPushButton("Supprimer");
    refreshButton = new QPushButton("Actualiser");
    buttonLayout->addWidget(addButton);
    buttonLayout->addWidget(editButton);
    buttonLayout->addWidget(deleteButton);
    buttonLayout->addWidget(refreshButton);
    mainLayout->addLayout(buttonLayout);


    // Connexions
    connect(addButton, &QPushButton::clicked, this, &AdminWindow::onAddClient);
    connect(editButton, &QPushButton::clicked, this, &AdminWindow::onEditClient);
    connect(deleteButton, &QPushButton::clicked, this, &AdminWindow::onDeleteClient);
    connect(refreshButton, &QPushButton::clicked, this, &AdminWindow::refreshModel);
    connect(searchButton, &QPushButton::clicked, this, &AdminWindow::onSearch);
       connect(resetButton, &QPushButton::clicked, [this]() {
        searchEdit->clear();
        clientModel->setFilter("");
        refreshModel();
    });
}

void AdminWindow::refreshModel()
{
    clientModel->select();
}
void AdminWindow::onSearch()
{
    QString prenom = searchEdit->text().trimmed();
    if (prenom.isEmpty()) {
        clientModel->setFilter("");
    } else {
        // Filtrer sur la colonne "prenom" (nom exact dans la base)
        clientModel->setFilter(QString("prenom LIKE '%%1%'").arg(prenom));
    }
     clientModel->select();
       qDebug() << "Nombre de lignes après filtre:" << clientModel->rowCount();
}
void AdminWindow::onAddClient()
{
    ClientEditDialog dlg(this);
    if (dlg.exec() == QDialog::Accepted) {
        refreshModel();
    }
}

void AdminWindow::onEditClient()
{
    int row = clientView->currentIndex().row();
    if (row < 0) {
        QMessageBox::warning(this, "Sélection", "Veuillez sélectionner un client à modifier.");
        return;
    }
    int clientId = clientModel->data(clientModel->index(row, 0)).toInt();
    ClientEditDialog dlg(this);
    dlg.setClientId(clientId);
    if (dlg.exec() == QDialog::Accepted) {
        refreshModel();
    }
}

void AdminWindow::onDeleteClient()
{
    int row = clientView->currentIndex().row();
    if (row < 0) {
        QMessageBox::warning(this, "Sélection", "Veuillez sélectionner un client à supprimer.");
        return;
    }
    int clientId = clientModel->data(clientModel->index(row, 0)).toInt();
    QMessageBox::StandardButton reply = QMessageBox::question(this, "Confirmation",
        "Supprimer définitivement ce client ?", QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::Yes) {
        QSqlQuery query;
        query.prepare("DELETE FROM clients WHERE id = ?");
        query.addBindValue(clientId);
        if (query.exec()) {
            refreshModel();
        } else {
            QMessageBox::critical(this, "Erreur", "Échec suppression : " + query.lastError().text());
        }
    }
}