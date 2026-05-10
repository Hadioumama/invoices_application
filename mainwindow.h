#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>
#include <QPushButton>

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
    void goBack();

private slots:
    void onLoginSuccess(int userId, const QString &role);  // ✅ const QString &
    void onRegisterRequested();
    void onLogout();

private:
    void setupUI();
    void updateBackButtonVisibility();

    QStackedWidget *m_stack;
    QPushButton *m_backButton;
    LoginDialog *m_loginPage;
    RegisterDialog *m_registerPage;
    AdminWindow *m_adminPage;
    ClientWindow *m_clientPage;
    
    static MainWindow *s_instance;
};

#endif // MAINWINDOW_H