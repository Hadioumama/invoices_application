#include "articleeditdialog.h"
#include <QVBoxLayout>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QMessageBox>

#include <QSqlQuery>
#include <QSqlError>

ArticleEditDialog::ArticleEditDialog(QWidget *parent)
    : QDialog(parent), m_articleId(-1)
{
    setupUI();
}

ArticleEditDialog::ArticleEditDialog(int articleId,
                                      QWidget *parent)
    : QDialog(parent), m_articleId(articleId)
{
    setupUI();
    loadArticle(articleId);
}

void ArticleEditDialog::setupUI()
{
   

    setWindowTitle(m_articleId == -1 ?
        "Ajouter un Article" : "Modifier l'Article");
    setMinimumWidth(420);

    setStyleSheet(
        "QDialog { background:#F7FAFC; }" 
        "QGroupBox {"
        "  font-weight:bold;border:1px solid #CBD5E0;"
        "  border-radius:6px;margin-top:8px;"
        "  padding-top:8px;color:#2B6CB0;"
        "  background:#F7FAFC;"  
        "}"
        "QLineEdit, QDoubleSpinBox, QSpinBox, QTextEdit {"
        "  border:1px solid #CBD5E0;border-radius:4px;"
        "  padding:5px;min-height:28px;"
          "  background:white;color:#1A202C;" 
        "}"
        "QLineEdit:focus, QDoubleSpinBox:focus {"
        "  border:1px solid #3182CE;"
        "}"
           "QLabel { color:#2D3748; }"   
    );

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(10);
    mainLayout->setContentsMargins(14, 14, 14, 14);

    QGroupBox *group = new QGroupBox("Informations Article");
    QFormLayout *form = new QFormLayout(group);
    form->setSpacing(8);
    form->setLabelAlignment(Qt::AlignRight);

    referenceEdit = new QLineEdit;
    referenceEdit->setPlaceholderText("Ex: ART-001");

    designationEdit = new QLineEdit;
    designationEdit->setPlaceholderText(
        "Nom du produit/service...");

    prixHTSpin = new QDoubleSpinBox;
    prixHTSpin->setMaximum(999999);
    prixHTSpin->setDecimals(2);
    prixHTSpin->setSuffix(" MAD");
    prixHTSpin->setMinimum(0);

    tvaSpin = new QDoubleSpinBox;
    tvaSpin->setMaximum(100);
    tvaSpin->setDecimals(1);
    tvaSpin->setValue(20.0);
    tvaSpin->setSuffix(" %");

    stockSpin = new QSpinBox;
    stockSpin->setMaximum(999999);
    stockSpin->setMinimum(0);

    uniteEdit = new QLineEdit;
    uniteEdit->setPlaceholderText("unité, kg, heure...");
    uniteEdit->setText("unité");

    descriptionEdit = new QTextEdit;
    descriptionEdit->setMaximumHeight(80);
    descriptionEdit->setPlaceholderText(
        "Description optionnelle...");

    form->addRow("Référence :*", referenceEdit);
    form->addRow("Désignation :*", designationEdit);
    form->addRow("Prix HT :*", prixHTSpin);
    form->addRow("TVA :", tvaSpin);
    form->addRow("Stock :", stockSpin);
    form->addRow("Unité :", uniteEdit);
    form->addRow("Description :", descriptionEdit);
    mainLayout->addWidget(group);

    QHBoxLayout *btnRow = new QHBoxLayout;
    cancelBtn = new QPushButton("Annuler");
    saveBtn = new QPushButton("💾 Enregistrer");
    saveBtn->setFixedHeight(34);
    cancelBtn->setFixedHeight(34);
    saveBtn->setStyleSheet(
        "background:#27AE60;color:white;font-weight:bold;"
        "border-radius:4px;padding:0 16px;");
    cancelBtn->setStyleSheet(
        "border:1px solid #CBD5E0;border-radius:4px;"
        "padding:0 16px;background:white;");
    btnRow->addStretch();
    btnRow->addWidget(cancelBtn);
    btnRow->addWidget(saveBtn);
    mainLayout->addLayout(btnRow);

    connect(saveBtn,   &QPushButton::clicked,
            this, &ArticleEditDialog::onSave);
    connect(cancelBtn, &QPushButton::clicked,
            this, &QDialog::reject);
}

void ArticleEditDialog::loadArticle(int id)
{
    QSqlQuery q;
    q.prepare("SELECT reference, designation, prix_ht, "
              "taux_tva, stock, unite, description "
              "FROM articles WHERE id = ?");
    q.addBindValue(id);
    if (!q.exec() || !q.next()) return;

    referenceEdit->setText(q.value(0).toString());
    designationEdit->setText(q.value(1).toString());
    prixHTSpin->setValue(q.value(2).toDouble());
    tvaSpin->setValue(q.value(3).toDouble());
    stockSpin->setValue(q.value(4).toInt());
    uniteEdit->setText(q.value(5).toString());
    descriptionEdit->setText(q.value(6).toString());
}
void ArticleEditDialog::onSave()
{
    QString reference = referenceEdit->text().trimmed();
    QString designation = designationEdit->text().trimmed();
    
    if (reference.isEmpty() || designation.isEmpty()) {
        QMessageBox::warning(this, "Erreur", "Référence et désignation sont obligatoires");
        return;
    }

    QSqlQuery q;
    
    if (m_articleId > 0) {
        q.prepare("UPDATE articles SET "
                  "reference=?, designation=?, prix_ht=?, taux_tva=?, "
                  "stock=?, unite=?, description=? "
                  "WHERE id=?");
        q.addBindValue(reference);
        q.addBindValue(designation);
        q.addBindValue(prixHTSpin->value());
        q.addBindValue(tvaSpin->value());
        q.addBindValue(stockSpin->value());
        q.addBindValue(uniteEdit->text().trimmed());
        q.addBindValue(descriptionEdit->toPlainText().trimmed());
        q.addBindValue(m_articleId);
    } else {
        q.prepare("INSERT INTO articles "
                  "(reference, designation, prix_ht, taux_tva, stock, unite, description) "
                  "VALUES (?,?,?,?,?,?,?)");
        q.addBindValue(reference);
        q.addBindValue(designation);
        q.addBindValue(prixHTSpin->value());
        q.addBindValue(tvaSpin->value());
        q.addBindValue(stockSpin->value());
        q.addBindValue(uniteEdit->text().trimmed());
        q.addBindValue(descriptionEdit->toPlainText().trimmed());
    }

    if (!q.exec()) {
        QMessageBox::critical(this, "Erreur SQL", q.lastError().text());
        return;
    }

    accept();
}