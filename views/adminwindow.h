#ifndef ADMINWINDOW_H
#define ADMINWINDOW_H

#include <QMainWindow>
#include <QSqlTableModel>

class QTableView;
class QPushButton;

class AdminWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit AdminWindow(QWidget *parent = nullptr);
    ~AdminWindow();

private slots:
    void onAddClient();
    void onEditClient();
    void onDeleteClient();
    void refreshModel();

private:
    void setupUI();
    QSqlTableModel *clientModel;
    QTableView *clientView;
    QPushButton *addButton, *editButton, *deleteButton, *refreshButton;
};

#endif