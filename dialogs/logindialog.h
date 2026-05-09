#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QPushButton>

class LoginDialog : public QDialog
{
    Q_OBJECT
public:
    explicit LoginDialog(QWidget *parent = nullptr);

    QString getEmail() const;
    QString getPassword() const;

signals:
    void createAccountRequested();

private slots:
    void onLogin();               // ✅ AJOUTER CECI
    void onCreateAccountClicked();

private:
    QLineEdit *emailEdit;
    QLineEdit *passwordEdit;
    QPushButton *loginButton;
    QPushButton *createButton;
};

#endif