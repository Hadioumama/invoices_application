#ifndef CLIENT_EDIT_DIALOG_H
#define CLIENT_EDIT_DIALOG_H

#include <QDialog>

class QLineEdit;
class QComboBox;
class QPushButton;

class ClientEditDialog : public QDialog
{
    Q_OBJECT
public:
    explicit ClientEditDialog(QWidget *parent = nullptr);
    void setClientId(int id); // charge le client pour modification

private slots:
    void on_typeChanged(const QString &type);
    void onSave();

private:
    void loadClient(int id);
    void saveClient();

    QLineEdit *lineEditNom, *lineEditPrenom, *lineEditEmail;
    QLineEdit *lineEditAdresse, *lineEditTelephone;
    QLineEdit *lineEditMotDePasse;
    QComboBox *comboBoxType;
    QLineEdit *lineEditNomEntreprise, *lineEditICE;
    QPushButton *saveButton, *cancelButton;

    int currentId;
};

#endif