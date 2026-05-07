#ifndef ARTICLEEDITDIALOG_H
#define ARTICLEEDITDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QTextEdit>
#include <QPushButton>

class ArticleEditDialog : public QDialog
{
    Q_OBJECT
public:
    explicit ArticleEditDialog(QWidget *parent = nullptr);
    explicit ArticleEditDialog(int articleId,
                                QWidget *parent = nullptr);

private slots:
    void onSave();

private:
    void setupUI();
    void loadArticle(int id);

    int         m_articleId;
    QLineEdit   *referenceEdit;
    QLineEdit   *designationEdit;
    QDoubleSpinBox *prixHTSpin;
    QDoubleSpinBox *tvaSpin;
    QSpinBox    *stockSpin;
    QLineEdit   *uniteEdit;
    QTextEdit   *descriptionEdit;
    QPushButton *saveBtn;
    QPushButton *cancelBtn;
};

#endif // ARTICLEEDITDIALOG_H