#include "mainwindow.h"
#include "dialogs/logindialog.h"
#include "dialogs/registerdialog.h"
#include "views/adminwindow.h"
#include "views/clientwindow.h"
#include <QVBoxLayout>
#include <QDebug>
#include <QMessageBox>

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
    setStyleSheet("background: #F7FAFC;");

    m_stack = new QStackedWidget(this);
    setCentralWidget(m_stack);

    // Page login
    m_loginPage = new LoginDialog(this);
    m_loginPage->setWindowFlags(Qt::Widget);
    m_stack->addWidget(m_loginPage);

    // Page register
    m_registerPage = new RegisterDialog(this);
    m_registerPage->setWindowFlags(Qt::Widget);
    m_stack->addWidget(m_registerPage);

    // Afficher login
    m_stack->setCurrentWidget(m_loginPage);

    // Connexions
    connect(m_loginPage, &LoginDialog::loginSuccess,
            this, &MainWindow::onLoginSuccess);
    connect(m_loginPage, &LoginDialog::createAccountRequested,
            this, &MainWindow::onRegisterRequested);
    connect(m_registerPage, &RegisterDialog::registerSuccess,
            this, &MainWindow::showLogin);
}

void MainWindow::onLoginSuccess(int userId, const QString &role)
{
    qDebug() << ">>> onLoginSuccess:" << userId << role;
    
    QString normalizedRole = role.toLower().trimmed();
    qDebug() << "Rôle normalisé:" << normalizedRole;

    if (normalizedRole == "admin") {
        qDebug() << "Ouverture AdminWindow...";
        showAdmin(userId);
    } else if (normalizedRole == "client") {
        qDebug() << "Ouverture ClientWindow...";
        showClient(userId);
    } else {
        qDebug() << "!!! RÔLE INCONNU:" << role;
        QMessageBox::warning(this, "Erreur", 
            "Rôle utilisateur inconnu : '" + role + "'\n"
            "Rôles attendus : 'admin' ou 'client'");
    }
}

void MainWindow::showLogin()
{
    m_stack->setCurrentWidget(m_loginPage);
    m_loginPage->clearFields(); // Nettoyer les champs
}

void MainWindow::showRegister()
{
    m_stack->setCurrentWidget(m_registerPage);
}

void MainWindow::showAdmin(int adminId)
{
    if (m_adminPage) {
        m_stack->removeWidget(m_adminPage);
        delete m_adminPage;
        m_adminPage = nullptr;
    }

    m_adminPage = new AdminWindow(adminId, this);
    m_stack->addWidget(m_adminPage);
    m_stack->setCurrentWidget(m_adminPage);

    connect(m_adminPage, &AdminWindow::logoutRequested, [this]() {
        qDebug() << ">>> logoutRequested reçu - retour à login";
        showLogin();
    });

}

void MainWindow::showClient(int clientId)
{
    qDebug() << "showClient appelé avec ID:" << clientId;
    
    if (m_clientPage) {
        m_stack->removeWidget(m_clientPage);
        delete m_clientPage;
        m_clientPage = nullptr;
    }

    m_clientPage = new ClientWindow(clientId, this);
    m_stack->addWidget(m_clientPage);
    m_stack->setCurrentWidget(m_clientPage);
}

void MainWindow::onRegisterRequested()
{
    showRegister();
}

