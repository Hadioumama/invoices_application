#include "invoicegenerator.h"
#include "invoicesenderdialog.h"
#include "utils/emailsender.h"
#include "database/database.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QCheckBox>
#include <QMessageBox>
#include <QSqlQuery>
#include <QDebug>
#include <QApplication>


InvoiceSenderDialog::InvoiceSenderDialog(int invoiceId, const QString &clientEmail,
                                         const QString &invoiceNumber, QWidget *parent)
    : QDialog(parent), m_invoiceId(invoiceId), m_clientEmail(clientEmail),
      m_invoiceNumber(invoiceNumber), m_wasSent(false)
{
    setupUI();
    emailEdit->setText(clientEmail);
}

void InvoiceSenderDialog::setupUI()
{
    setWindowTitle("Envoyer la Facture par Email");
    setGeometry(200, 200, 500, 300);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // Info
    mainLayout->addWidget(new QLabel(QString("Facture: <b>%1</b>").arg(m_invoiceNumber)));

    // Email field
    mainLayout->addWidget(new QLabel("Email du destinataire:"));
    emailEdit = new QLineEdit;
    emailEdit->setText(m_clientEmail);
    mainLayout->addWidget(emailEdit);

    // Confirmation checkbox
    confirmCheckBox = new QCheckBox("Je confirme l'envoi de cette facture");
    mainLayout->addWidget(confirmCheckBox);

    // Status label
    statusLabel = new QLabel("");
    statusLabel->setStyleSheet("color: blue;");
    mainLayout->addWidget(statusLabel);

    mainLayout->addSpacing(20);

    // Buttons
    QHBoxLayout *buttonLayout = new QHBoxLayout;
    sendBtn = new QPushButton("📧 Envoyer");
    cancelBtn = new QPushButton("Annuler");
    buttonLayout->addStretch();
    buttonLayout->addWidget(sendBtn);
    buttonLayout->addWidget(cancelBtn);
    mainLayout->addLayout(buttonLayout);

    connect(sendBtn, &QPushButton::clicked, this, &InvoiceSenderDialog::onSendEmail);
    connect(cancelBtn, &QPushButton::clicked, this, &InvoiceSenderDialog::onCancel);
}

void InvoiceSenderDialog::onSendEmail()
{
    if (!confirmCheckBox->isChecked())
    {
        QMessageBox::warning(this, "Confirmation", "Veuillez confirmer l'envoi");
        return;
    }

    QString email = emailEdit->text().trimmed();
    if (email.isEmpty())
    {
        QMessageBox::warning(this, "Erreur", "Entrez une adresse email");
        return;
    }

    statusLabel->setText("Envoi en cours...");
    QApplication::processEvents();

    if (sendEmail())
    {
        statusLabel->setText("✓ Email envoyé avec succès");
        QMessageBox::information(this, "Succès", "Facture envoyée au client");
        m_wasSent = true;
        accept();
    }
    else
    {
        statusLabel->setText("✗ Erreur lors de l'envoi");
        QMessageBox::critical(this, "Erreur", "Impossible d'envoyer l'email");
    }
}

bool InvoiceSenderDialog::sendEmail()
{
    // Récupérer infos client complémentaires
    QSqlQuery query;
    query.prepare("SELECT c.nom, c.prenom FROM factures f "
                  "JOIN clients c ON f.client_id = c.id WHERE f.id = ?");
    query.addBindValue(m_invoiceId);

    if (!query.exec() || !query.next())
    {
        return false;
    }

    QString clientNom = query.value(0).toString();
    QString clientPrenom = query.value(1).toString();
    QString clientFullName = clientNom + " " + clientPrenom;

    // Générer PDF
    QString pdfPath = InvoiceGenerator::getPdfOutputPath() + "/" +
                      InvoiceGenerator::getInvoiceFileName(m_invoiceNumber);

    InvoiceGenerator pdfGenerator;
    if (!pdfGenerator.generatePDF(m_invoiceId, pdfPath))
    {
        qDebug() << "Erreur génération PDF";
        return false;
    }

    // Envoyer email
    EmailSender emailSender;
    return emailSender.sendInvoiceEmail(emailEdit->text(), clientFullName, m_invoiceNumber, pdfPath);
}

void InvoiceSenderDialog::onCancel()
{
    reject();
}
