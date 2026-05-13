#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class QStackedWidget;
class LoginDialog;
class RegisterDialog;
class AdminWindow;
class ClientWindow;
class Client;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    static MainWindow* instance();

public slots:
    void showLogin();
    void showRegister();

private slots:
    void onLoginSuccess(int userId, const QString &role);
    void onRegisterSuccess(const Client &client);
    void showAdmin(int adminId);
    void showClient(int clientId);
    void onLogout();

private:
    void setupUI();

    QStackedWidget *m_stack;
    LoginDialog *m_loginPage;
    RegisterDialog *m_registerPage;
    AdminWindow *m_adminPage;
    ClientWindow *m_clientPage;

    static MainWindow *s_instance;
};

#endif