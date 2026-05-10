#include "mainwindow.h"
#include "dialogs/logindialog.h"
#include "dialogs/registerdialog.h"
#include "views/adminwindow.h"
#include "views/clientwindow.h"
#include <QVBoxLayout>
#include <QDebug>

MainWindow *MainWindow::s_instance = nullptr;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      m_adminPage(nullptr),
      m_clientPage(nullptr)
{
    s_instance = this;
    setupUI();
}

MainWindow::~MainWindow() {}

MainWindow* MainWindow::instance()
{
    return s_instance;
}

void MainWindow::setupUI()
{
    setMinimumSize(1100, 700);
    resize(1200, 750);

    m_stack = new QStackedWidget(this);
    setCentralWidget(m_stack);

    // Page login
    m_loginPage = new LoginDialog(this);
    m_stack->addWidget(m_loginPage);

    // Page register
    m_registerPage = new RegisterDialog(this);
    m_stack->addWidget(m_registerPage);

    // Bouton retour
    m_backButton = new QPushButton("← Retour", this);
    m_backButton->setStyleSheet(
        "background:#718096;color:white;"
        "border-radius:4px;padding:6px 14px;"
        "border:none;font-weight:bold;");
    m_backButton->setFixedSize(100, 32);
    m_backButton->move(10, 10);
    m_backButton->hide();

    // Afficher login
    m_stack->setCurrentWidget(m_loginPage);

    // Connexions
    connect(m_loginPage, &LoginDialog::loginSuccess,
            this, &MainWindow::onLoginSuccess);
    connect(m_loginPage,
            &LoginDialog::createAccountRequested,
            this, &MainWindow::onRegisterRequested);
    connect(m_registerPage,
            &RegisterDialog::registerSuccess,
            this, &MainWindow::showLogin);
    connect(m_backButton, &QPushButton::clicked,
            this, &MainWindow::goBack);
}

void MainWindow::onLoginSuccess(int userId,
                                 const QString &role)
{
    qDebug() << "onLoginSuccess:" << userId << role;

    if (role == "admin") {
        showAdmin(userId);
    } else if (role == "client") {
        showClient(userId);
    } else {
        qDebug() << "Rôle inconnu:" << role;
    }
}

void MainWindow::showLogin()
{
    m_stack->setCurrentWidget(m_loginPage);
    m_backButton->hide();
}

void MainWindow::showRegister()
{
    m_stack->setCurrentWidget(m_registerPage);
    m_backButton->show();
}

void MainWindow::showAdmin(int adminId)
{
    Q_UNUSED(adminId)
    if (m_adminPage) {
        m_stack->removeWidget(m_adminPage);
        delete m_adminPage;
        m_adminPage = nullptr;
    }

 m_adminPage = new AdminWindow(adminId, this);
    m_stack->addWidget(m_adminPage);
    m_stack->setCurrentWidget(m_adminPage);
    m_backButton->hide();

    // Déconnexion depuis AdminWindow
    // (si tu as un signal logout dans AdminWindow)
}

void MainWindow::showClient(int clientId)
{
    if (m_clientPage) {
        m_stack->removeWidget(m_clientPage);
        delete m_clientPage;
        m_clientPage = nullptr;
    }

    m_clientPage = new ClientWindow(clientId, this);
    m_stack->addWidget(m_clientPage);
    m_stack->setCurrentWidget(m_clientPage);
    m_backButton->hide();
}

void MainWindow::onRegisterRequested()
{
    showRegister();
}

void MainWindow::goBack()
{
    showLogin();
}

void MainWindow::onLogout()
{
    showLogin();
}