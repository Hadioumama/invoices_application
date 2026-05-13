#ifndef REGISTERDIALOG_H
#define REGISTERDIALOG_H

#include <QDialog>
#include "models/client.h"

class QLineEdit;
class QComboBox;
class QPushButton;
class QLabel;
class QCheckBox;

class RegisterDialog : public QDialog
{
    Q_OBJECT

public:
    explicit RegisterDialog(QWidget *parent = nullptr);
    ~RegisterDialog();

    Client getClient() const;

signals:
    void registerSuccess(const Client &client);
    void goToLogin();

private slots:
    void on_enregistrerClicked();
    void togglePasswordVisibility(bool checked);

private:
    void setupUI();
    void applyStyles();
    bool validateInputs();
    QString hashPassword(const QString &password);

    QLabel *m_titleLabel;
    QLabel *m_subtitleLabel;
    QLineEdit *m_lineEditNom;
    QLineEdit *m_lineEditPrenom;
    QLineEdit *m_lineEditEmail;
    QLineEdit *m_lineEditTelephone;
    QLineEdit *m_lineEditAdresse;
    QLineEdit *m_lineEditMotDePasse;
    QLineEdit *m_lineEditConfirmation;
    QCheckBox *m_showPasswordCheck;
    QComboBox *m_comboBoxType;
    QWidget *m_entrepriseWidget;
    QLineEdit *m_lineEditNomEntreprise;
    QLineEdit *m_lineEditICE;
    QPushButton *m_pushButtonEnregistrer;
    QPushButton *m_pushButtonAnnuler;
    QLabel *m_statusLabel;
};

#endif // REGISTERDIALOG_H