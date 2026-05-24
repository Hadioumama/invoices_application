#ifndef CLIENTWINDOW_H
#define CLIENTWINDOW_H

#include <QWidget>
#include <QStackedWidget>
#include <QTableView>
#include <QSqlQueryModel>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTextEdit>
#include <QComboBox>
#include <QDateEdit>
#include <QFrame>

class ClientWindow : public QWidget
{
    Q_OBJECT
public:
    explicit ClientWindow(int clientId,
                          QWidget *parent = nullptr);
signals:
    void logoutRequested();

private slots:
    void refreshDashboard();
    void onDownloadPDF();
    void onViewDetails();
    void onFilterFactures();
    void onResetFilter();
    void refreshPaiements();
    void onSaveProfil();
    void onChangePassword();
    void onSendMessage();
    void onLogout();
    void onNavigateTo(const QString &page);

private:
    void setupUI();
    QWidget* buildSidebar();
    QWidget* buildDashboardPage();
    QWidget* buildFacturesPage();
    QWidget* buildPaiementsPage();
    QWidget* buildProfilPage();
    QWidget* buildContactPage();
    void loadClientInfo();

    int     m_clientId;
    QString m_clientNom;
    QString m_clientEmail;

    QStackedWidget *m_pageStack;

    // Dashboard
    QLabel *statTotalFactures;
    QLabel *statMontantDu;
    QLabel *statDerniereFacture;
    QLabel *statStatutCompte;
    QLabel *statTotalPaye;
    QLabel *statFacturesMois;
    QTableView    *recentTable;
    QSqlQueryModel *recentModel;

    // Factures
    QTableView    *facturesTable;
    QSqlQueryModel *facturesModel;
    QComboBox *filterStatut;
    QDateEdit *filterDateDebut;
    QDateEdit *filterDateFin;
    QLineEdit *searchFacture;

    // Paiements
    QTableView    *paiementsTable;
    QSqlQueryModel *paiementsModel;
    QLabel *totalPayeLabel;
    QLabel *totalDuLabel;
    QLabel *resteLabel;

    // Profil
    QLineEdit *nomEdit;
    QLineEdit *prenomEdit;
    QLineEdit *emailEdit;
    QLineEdit *telEdit;
    QLineEdit *adresseEdit;
    QLineEdit *oldPasswordEdit;
    QLineEdit *newPasswordEdit;
    QLineEdit *confirmPasswordEdit;

    // Contact
    QLineEdit *sujetEdit;
    QTextEdit *messageEdit;
};

#endif