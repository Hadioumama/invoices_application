#ifndef ARTICLESWIDGET_H
#define ARTICLESWIDGET_H

#include <QWidget>
#include <QTableView>
#include <QSqlTableModel>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>

class ArticlesWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ArticlesWidget(QWidget *parent = nullptr);

signals:
    void articleSelected(int id, const QString &designation,
                         double prixHT, double tva);

private slots:
    void onAddArticle();
    void onEditArticle();
    void onDeleteArticle();
    void onSearch();
    void refreshModel();

private:
    void setupUI();

    QTableView      *tableView;
    QSqlTableModel  *model;
    QLineEdit       *searchEdit;
    QPushButton     *addBtn;
    QPushButton     *editBtn;
    QPushButton     *deleteBtn;
    QPushButton     *refreshBtn;
    QLabel          *totalLabel;
};

#endif // ARTICLESWIDGET_H