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
    void registerSuccess();

private slots:
    void on_typeChanged(const QString &type);
    void on_enregistrerClicked();
    void togglePasswordVisibility(bool checked);

private:
    void setupUI();
    void applyStyles();
    bool validateInputs();
    QString hashPassword(const QString &password);

    // Widgets
    QLineEdit *lineEditNom;
    QLineEdit *lineEditPrenom;
    QLineEdit *lineEditEmail;
    QLineEdit *lineEditAdresse;
    QLineEdit *lineEditTelephone;
    QLineEdit *lineEditMotDePasse;
    QLineEdit *lineEditConfirmation;
    QComboBox *comboBoxType;
    QLineEdit *lineEditNomEntreprise;
    QLineEdit *lineEditICE;
    QPushButton *pushButtonEnregistrer;
    QPushButton *pushButtonAnnuler;
    QCheckBox *showPasswordCheck;
    QLabel *statusLabel;
};

#endif