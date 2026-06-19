#include "mainwindow.h"
#include "dialogs/logindialog.h"
#include "dialogs/registerdialog.h"
#include "views/adminwindow.h"
#include "views/clientwindow.h"
#include <QStackedWidget>
#include <QDebug>
#include <QMessageBox>

MainWindow *MainWindow::s_instance = nullptr;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      m_stack(nullptr),
      m_loginPage(nullptr),
      m_registerPage(nullptr),
      m_adminPage(nullptr),
      m_clientPage(nullptr)
{
    s_instance = this;
    setupUI();
}

MainWindow::~MainWindow()
{
    s_instance = nullptr;
}

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
 m_stack->setStyleSheet("QStackedWidget { background: #F7FAFC; }");
    // Page login
    m_loginPage = new LoginDialog(this);
    m_loginPage->setWindowFlags(Qt::Widget);
    m_stack->addWidget(m_loginPage);
 m_loginPage->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    // Page register
    m_registerPage = new RegisterDialog(this);
    m_registerPage->setWindowFlags(Qt::Widget);
    m_stack->addWidget(m_registerPage);

    // Afficher login par défaut
    m_stack->setCurrentWidget(m_loginPage);

    // ===== CONNEXIONS LOGIN =====
    connect(m_loginPage, &LoginDialog::loginSuccess,
            this, &MainWindow::onLoginSuccess);
    connect(m_loginPage, &LoginDialog::createAccountRequested,
            this, &MainWindow::showRegister);

    // ===== CONNEXIONS REGISTER =====
    connect(m_registerPage, &RegisterDialog::registerSuccess,
            this, &MainWindow::onRegisterSuccess);
    connect(m_registerPage, &RegisterDialog::goToLogin,
            this, &MainWindow::showLogin);
}

void MainWindow::onLoginSuccess(int userId, const QString &role)
{
    qDebug() << ">>> onLoginSuccess - ID:" << userId << "Role brut:" << role;

    QString normalizedRole = role.toLower().trimmed();
    qDebug() << "Rôle normalisé:" << normalizedRole;

    if (normalizedRole == "admin") {
        qDebug() << "→ Redirection Admin";
        showAdmin(userId);
    } else if (normalizedRole == "client") {
        qDebug() << "→ Redirection Client";
        showClient(userId);
    } else {
        qDebug() << "!!! RÔLE INCONNU:" << role;
        QMessageBox::warning(this, "Erreur",
            "Rôle utilisateur inconnu : '" + role + "'\n"
            "Rôles attendus : 'admin' ou 'client'");
    }
}

void MainWindow::onRegisterSuccess(const Client &client)
{
    qDebug() << ">>> onRegisterSuccess - ID:" << client.id
             << "Email:" << client.email;
    // Après inscription, retour au login
    showLogin();
}

void MainWindow::showLogin()
{
    qDebug() << "→ showLogin()";
    m_stack->setCurrentWidget(m_loginPage);
    m_loginPage->clearFields();
}

void MainWindow::showRegister()
{
    qDebug() << "→ showRegister()";
    m_stack->setCurrentWidget(m_registerPage);
}

void MainWindow::showAdmin(int adminId)
{
    qDebug() << ">>> showAdmin() - ID:" << adminId;

    // Nettoyer ancienne page admin si existe
    if (m_adminPage) {
        m_stack->removeWidget(m_adminPage);
        delete m_adminPage;
        m_adminPage = nullptr;
    }

    // Créer nouvelle page admin
    m_adminPage = new AdminWindow(adminId, this);

    if (!m_adminPage) {
        qDebug() << "ERREUR CRITIQUE: AdminWindow non créé !";
        QMessageBox::critical(this, "Erreur", "Impossible de créer la fenêtre admin");
        return;
    }

    m_stack->addWidget(m_adminPage);

    // ✅ CONNECTER DÉCONNEXION
    connect(m_adminPage, &AdminWindow::logoutRequested,
            this, &MainWindow::onLogout);

    m_stack->setCurrentWidget(m_adminPage);
    qDebug() << "AdminWindow affichée avec succès";
}

void MainWindow::showClient(int clientId)
{
    qDebug() << ">>> showClient() - ID:" << clientId;

    // Nettoyer ancienne page client si existe
    if (m_clientPage) {
        m_stack->removeWidget(m_clientPage);
        delete m_clientPage;
        m_clientPage = nullptr;
    }

    // Créer nouvelle page client
    m_clientPage = new ClientWindow(clientId, this);

    if (!m_clientPage) {
        qDebug() << "ERREUR CRITIQUE: ClientWindow non créé !";
        QMessageBox::critical(this, "Erreur", "Impossible de créer la fenêtre client");
        return;
    }

    m_stack->addWidget(m_clientPage);

    // ✅ CONNECTER DÉCONNEXION
    connect(m_clientPage, &ClientWindow::logoutRequested,
            this, &MainWindow::onLogout);

    m_stack->setCurrentWidget(m_clientPage);
    qDebug() << "ClientWindow affichée avec succès";
}

void MainWindow::onLogout()
{
    qDebug() << ">>> onLogout()";

    // Nettoyer admin
    if (m_adminPage) {
        m_stack->removeWidget(m_adminPage);
        delete m_adminPage;
        m_adminPage = nullptr;
    }

    // Nettoyer client
    if (m_clientPage) {
        m_stack->removeWidget(m_clientPage);
        delete m_clientPage;
        m_clientPage = nullptr;
    }

    showLogin();
}