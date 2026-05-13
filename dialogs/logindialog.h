#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QString>
#include <QTimer>

class LoginDialog : public QDialog
{
    Q_OBJECT
public:
    explicit LoginDialog(QWidget *parent = nullptr);
    
    static QString hashPassword(const QString &password); 
    QString getEmail() const;
    QString getPassword() const;
    void clearFields();

signals:
    void loginSuccess(int userId, const QString &role);
    void createAccountRequested();
   
private slots:
    void onLogin();
    void onCreateAccountClicked();
    void onForgotPassword();
    void togglePasswordVisibility(bool checked);

private:
    void setupUI();
    void applyStyles();
    
    bool isValidEmail(const QString &email);
    bool isGmailEmail(const QString &email);
    
    QString generateVerificationCode();
    bool sendVerificationCode(const QString &email, const QString &code);
    void showVerificationDialog(const QString &email);
    void showPasswordResetDialog(const QString &email);

    // Widgets
    QLineEdit *emailEdit;
    QLineEdit *passwordEdit;
    QPushButton *loginButton;
    QPushButton *createButton;
    QPushButton *forgotButton;
    QPushButton *togglePwdButton;

    QTimer *m_countdownTimer;
    int m_remainingSeconds;
    int m_attemptCount;
    QString m_pendingEmail;
    QString m_verificationCode;
};

#endif // LOGINDIALOG_H