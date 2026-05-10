#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>

class LoginDialog;
class RegisterDialog;
class AdminWindow;
class ClientWindow;

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    static MainWindow* instance();
    void showLogin();
    void showRegister();
    void showAdmin(int adminId);
    void showClient(int clientId);

private slots:
    void onLoginSuccess(int userId, const QString &role);
    void onRegisterRequested();
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

#endif // MAINWINDOW_H