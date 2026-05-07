#include "articleswidget.h"
#include "dialogs/articleeditdialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QSqlQuery>
#include <QSqlError>
#include <QHeaderView>
#include <QDebug>

ArticlesWidget::ArticlesWidget(QWidget *parent)
    : QWidget(parent)
{
    setupUI();
    refreshModel();
}

void ArticlesWidget::setupUI()
{
    setStyleSheet(
        "QWidget { background: white; }"
        "QTableView {"
        "  border: 1px solid #E2E8F0;"
        "  gridline-color: #EDF2F7;"
        "  selection-background-color: #BEE3F8;"
        "  selection-color: #2D3748;"
        "}"
        "QHeaderView::section {"
        "  background: #2B6CB0;"
        "  color: white;"
        "  font-weight: bold;"
        "  padding: 7px;"
        "  border: none;"
        "}"
        "QPushButton {"
        "  padding: 6px 14px;"
        "  border-radius: 4px;"
        "  border: none;"
        "  font-weight: bold;"
        "}"
    );

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(10);
    mainLayout->setContentsMargins(12, 12, 12, 12);

    // Titre
    QLabel *title = new QLabel("📦 Catalogue Articles");
    title->setStyleSheet(
        "font-size:16px;font-weight:bold;color:#1B2A3B;");
    mainLayout->addWidget(title);

    // Recherche
    QHBoxLayout *searchLayout = new QHBoxLayout;
    searchEdit = new QLineEdit;
    searchEdit->setPlaceholderText(
        "🔍 Rechercher par désignation ou référence...");
    searchEdit->setStyleSheet(
        "padding:7px;border:1px solid #CBD5E0;"
        "border-radius:4px;font-size:11px;");
    QPushButton *searchBtn = new QPushButton("Rechercher");
    searchBtn->setStyleSheet(
        "background:#3182CE;color:white;");
    QPushButton *resetBtn = new QPushButton("✕ Réinitialiser");
    resetBtn->setStyleSheet(
        "background:#718096;color:white;");
    searchLayout->addWidget(searchEdit);
    searchLayout->addWidget(searchBtn);
    searchLayout->addWidget(resetBtn);
    mainLayout->addLayout(searchLayout);

    // Tableau
    model = new QSqlTableModel(this);
    model->setTable("articles");
    model->setEditStrategy(QSqlTableModel::OnManualSubmit);
    model->setHeaderData(0, Qt::Horizontal, "ID");
    model->setHeaderData(1, Qt::Horizontal, "Référence");
    model->setHeaderData(2, Qt::Horizontal, "Désignation");
    model->setHeaderData(3, Qt::Horizontal, "Prix HT");
    model->setHeaderData(4, Qt::Horizontal, "TVA %");
    model->setHeaderData(5, Qt::Horizontal, "Stock");
    model->setHeaderData(6, Qt::Horizontal, "Unité");
    model->setHeaderData(7, Qt::Horizontal, "Description");

    tableView = new QTableView;
    tableView->setModel(model);
    tableView->setSelectionBehavior(
        QAbstractItemView::SelectRows);
    tableView->setSelectionMode(
        QAbstractItemView::SingleSelection);
    tableView->setEditTriggers(
        QAbstractItemView::NoEditTriggers);
    tableView->horizontalHeader()->setStretchLastSection(true);
    tableView->verticalHeader()->setVisible(false);
    tableView->setAlternatingRowColors(true);
    tableView->setColumnWidth(0, 40);
    tableView->setColumnWidth(1, 100);
    tableView->setColumnWidth(2, 200);
    tableView->setColumnWidth(3, 90);
    tableView->setColumnWidth(4, 60);
    tableView->setColumnWidth(5, 60);
    tableView->setColumnWidth(6, 70);
    tableView->setColumnHidden(0, true);
    tableView->setMinimumHeight(300);
    mainLayout->addWidget(tableView);

    // Info double-clic
    QLabel *hint = new QLabel(
        "💡 Double-cliquez sur un article pour l'ajouter "
        "à la facture");
    hint->setStyleSheet(
        "font-size:10px;color:#718096;font-style:italic;");
    mainLayout->addWidget(hint);

    // Boutons action
    QHBoxLayout *btnLayout = new QHBoxLayout;
    addBtn = new QPushButton("➕ Ajouter");
    editBtn = new QPushButton("✏️ Modifier");
    deleteBtn = new QPushButton("🗑️ Supprimer");
    refreshBtn = new QPushButton("🔄 Actualiser");
    totalLabel = new QLabel;
    totalLabel->setStyleSheet(
        "font-weight:bold;color:#2B6CB0;font-size:11px;");

    addBtn->setStyleSheet(
        "background:#27AE60;color:white;");
    editBtn->setStyleSheet(
        "background:#3182CE;color:white;");
    deleteBtn->setStyleSheet(
        "background:#E53E3E;color:white;");
    refreshBtn->setStyleSheet(
        "background:#718096;color:white;");

    btnLayout->addWidget(addBtn);
    btnLayout->addWidget(editBtn);
    btnLayout->addWidget(deleteBtn);
    btnLayout->addWidget(refreshBtn);
    btnLayout->addStretch();
    btnLayout->addWidget(totalLabel);
    mainLayout->addLayout(btnLayout);

    // Connexions
    connect(addBtn,    &QPushButton::clicked,
            this, &ArticlesWidget::onAddArticle);
    connect(editBtn,   &QPushButton::clicked,
            this, &ArticlesWidget::onEditArticle);
    connect(deleteBtn, &QPushButton::clicked,
            this, &ArticlesWidget::onDeleteArticle);
    connect(refreshBtn,&QPushButton::clicked,
            this, &ArticlesWidget::refreshModel);
    connect(searchBtn, &QPushButton::clicked,
            this, &ArticlesWidget::onSearch);
    connect(searchEdit,&QLineEdit::returnPressed,
            this, &ArticlesWidget::onSearch);
    connect(resetBtn,  &QPushButton::clicked, [this](){
        searchEdit->clear();
        model->setFilter("");
        refreshModel();
    });
    connect(tableView, &QTableView::doubleClicked,
            this, &ArticlesWidget::onArticleDoubleClicked);
}

void ArticlesWidget::refreshModel()
{
    model->select();
    // Afficher total articles
    QSqlQuery q("SELECT COUNT(*) FROM articles");
    if (q.next())
        totalLabel->setText(
            QString("Total: %1 articles")
            .arg(q.value(0).toInt()));
}

void ArticlesWidget::onSearch()
{
    QString txt = searchEdit->text().trimmed();
    if (txt.isEmpty()) {
        model->setFilter("");
    } else {
        model->setFilter(
            QString("designation LIKE '%%1%' OR "
                    "reference LIKE '%%1%'").arg(txt));
    }
    model->select();
}

void ArticlesWidget::onAddArticle()
{
    ArticleEditDialog dlg(this);
    if (dlg.exec() == QDialog::Accepted)
        refreshModel();
}

void ArticlesWidget::onEditArticle()
{
    int row = tableView->currentIndex().row();
    if (row < 0) {
        QMessageBox::warning(this, "Sélection",
            "Veuillez sélectionner un article.");
        return;
    }
    int id = model->data(
        model->index(row, 0)).toInt();
    ArticleEditDialog dlg(id, this);
    if (dlg.exec() == QDialog::Accepted)
        refreshModel();
}

void ArticlesWidget::onDeleteArticle()
{
    int row = tableView->currentIndex().row();
    if (row < 0) {
        QMessageBox::warning(this, "Sélection",
            "Veuillez sélectionner un article.");
        return;
    }
    int id = model->data(
        model->index(row, 0)).toInt();
    QString nom = model->data(
        model->index(row, 2)).toString();

    auto reply = QMessageBox::question(this,
        "Confirmation",
        QString("Supprimer l'article \"%1\" ?").arg(nom),
        QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        QSqlQuery q;
        q.prepare("DELETE FROM articles WHERE id = ?");
        q.addBindValue(id);
        if (q.exec()) {
            refreshModel();
            QMessageBox::information(this, "Succès",
                "Article supprimé.");
        } else {
            QMessageBox::critical(this, "Erreur",
                q.lastError().text());
        }
    }
}

void ArticlesWidget::onArticleDoubleClicked(
    const QModelIndex &index)
{
    int row = index.row();
    int id = model->data(
        model->index(row, 0)).toInt();
    QString designation = model->data(
        model->index(row, 2)).toString();
    double prixHT = model->data(
        model->index(row, 3)).toDouble();
    double tva = model->data(
        model->index(row, 4)).toDouble();

    emit articleSelected(id, designation, prixHT, tva);
    QMessageBox::information(this, "Article sélectionné",
        QString("✅ \"%1\" ajouté à la facture\n"
                "Prix HT: %2 MAD | TVA: %3%%")
        .arg(designation)
        .arg(QString::number(prixHT, 'f', 2))
        .arg(tva));
}