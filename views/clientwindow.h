#ifndef CLIENTWINDOW_H
#define CLIENTWINDOW_H

#include <QMainWindow>

class ClientWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit ClientWindow(const QString &email, QWidget *parent = nullptr);
};

#endif