#ifndef CLIENTWINDOW_H
#define CLIENTWINDOW_H

#include <QMainWindow>
#include <QTabWidget>
#include <QTableView>
#include <QSqlQueryModel>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTextEdit>
#include <QComboBox>
#include <QDateEdit>

class ClientWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit ClientWindow(int clientId, QWidget *parent = nullptr);
signals:
    void logoutRequested();
private slots:
    // Dashboard
    void refreshDashboard();
    // Factures
    void onDownloadPDF();
    void onViewDetails();
    void onFilterFactures();
    void onResetFilter();
    // Paiements
    void refreshPaiements();
    // Profil
    void onSaveProfil();
    void onChangePassword();
    // Contact
    void onSendMessage();
    // Déconnexion
    void onLogout();

private:
    void setupUI();
    void setupDashboard();
    void setupFactures();
    void setupPaiements();
    void setupProfil();
    void setupContact();
    void loadClientInfo();

    int m_clientId;
    QString m_clientNom;
    QString m_clientEmail;

    QTabWidget *tabWidget;

    // ── Dashboard
    QLabel *statTotalFactures;
    QLabel *statMontantDu;
    QLabel *statDerniereFacture;
    QLabel *statStatutCompte;
    QLabel *statTotalPaye;
    QLabel *statFacturesMois;
    QTableView *recentTable;
    QSqlQueryModel *recentModel;

    // ── Factures
    QTableView *facturesTable;
    QSqlQueryModel *facturesModel;
    QComboBox *filterStatut;
    QDateEdit *filterDateDebut;
    QDateEdit *filterDateFin;
    QLineEdit *searchFacture;

    // ── Paiements
    QTableView *paiementsTable;
    QSqlQueryModel *paiementsModel;
    QLabel *totalPayeLabel;
    QLabel *totalDuLabel;
    QLabel *resteLabel;

    // ── Profil
    QLineEdit *nomEdit;
    QLineEdit *prenomEdit;
    QLineEdit *emailEdit;
    QLineEdit *telEdit;
    QLineEdit *adresseEdit;
    QLineEdit *oldPasswordEdit;
    QLineEdit *newPasswordEdit;
    QLineEdit *confirmPasswordEdit;

    // ── Contact
    QLineEdit *sujetEdit;
    QTextEdit *messageEdit;
};

#endif