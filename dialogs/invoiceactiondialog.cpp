#include "invoiceactiondialog.h"
#include "utils/invoicegenerator.h"
#include "utils/entreprise_config_manager.h"
#include "utils/emailsender.h"
#include "database/database.h"
#include "dialogs/paymentdialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QMessageBox>
#include <QProgressDialog>
#include <QApplication>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QDesktopServices>
#include <QUrl>

InvoiceActionDialog::InvoiceActionDialog(int invoiceId, QWidget *parent)
    : QDialog(parent), m_invoiceId(invoiceId)
{
    setupUI();

    QSqlQuery query;
    query.prepare(
        "SELECT c.email FROM factures f "
        "JOIN clients c ON f.client_id = c.id "
        "WHERE f.id = ?");
    query.addBindValue(invoiceId);
    if (query.exec() && query.next())
        emailEdit->setText(query.value(0).toString());
}

void InvoiceActionDialog::setupUI()
{
    setWindowTitle("Actions sur la Facture");
    setGeometry(200, 200, 500, 220);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(new QLabel("Sélectionnez une action:"));

    QHBoxLayout *buttonLayout = new QHBoxLayout;

    printBtn   = new QPushButton("🖨️ Imprimer");
    emailBtn   = new QPushButton("📧 Envoyer par Email");
    paymentBtn = new QPushButton("💳 Gérer les Paiements");
    paymentBtn->setStyleSheet(
        "background:#9B59B6; color:white; font-weight:bold;"
        "padding:8px 16px; border-radius:4px; border:none;");

    buttonLayout->addWidget(printBtn);
    buttonLayout->addWidget(emailBtn);
    buttonLayout->addWidget(paymentBtn);
    mainLayout->addLayout(buttonLayout);

    mainLayout->addWidget(new QLabel("Email pour envoi:"));
    emailEdit = new QLineEdit;
    emailEdit->setPlaceholderText("email@example.com");
    mainLayout->addWidget(emailEdit);

    QHBoxLayout *closeLayout = new QHBoxLayout;
    closeLayout->addStretch();
    closeBtn = new QPushButton("Fermer");
    closeLayout->addWidget(closeBtn);
    mainLayout->addLayout(closeLayout);

    connect(printBtn,   &QPushButton::clicked,
            this, &InvoiceActionDialog::onPrint);
    connect(emailBtn,   &QPushButton::clicked,
            this, &InvoiceActionDialog::onSendEmail);
    connect(paymentBtn, &QPushButton::clicked,
            this, &InvoiceActionDialog::onManagePayments);
    connect(closeBtn,   &QPushButton::clicked,
            this, &QDialog::accept);
}
void InvoiceActionDialog::onPrint()
{
    InvoiceStyle style;

    EntrepriseConfigManager *mgr = EntrepriseConfigManager::instance();
    if (mgr->isConfigured()) {
        EntrepriseConfig cfg = mgr->loadConfig();
        if (!cfg.nom.isEmpty())            style.companyName    = cfg.nom;
        if (!cfg.rib.isEmpty())            style.companyICE     = cfg.rib;
        if (!cfg.logoPath.isEmpty())       style.logoPath       = cfg.logoPath;
        if (!cfg.signaturePath.isEmpty())  style.signaturePath  = cfg.signaturePath;
        if (cfg.themeCouleur.isValid())    style.primaryColor   = cfg.themeCouleur.name();
    }

    InvoiceGenerator gen;
    QString tmpPath = InvoiceGenerator::getPdfOutputPath() +
                      "/print_temp.pdf";

    if (gen.generatePDF(m_invoiceId, tmpPath, style)) {
        QDesktopServices::openUrl(QUrl::fromLocalFile(tmpPath));
    } else {
        QMessageBox::critical(this, "Erreur",
                              "Impossible de générer le PDF pour impression");
    }
}

void InvoiceActionDialog::onSendEmail()
{
    QString email = emailEdit->text().trimmed();
    if (email.isEmpty()) {
        QMessageBox::warning(this, "Erreur", "Entrez une adresse email");
        return;
    }

    QString pdfPath = InvoiceGenerator::getPdfOutputPath()
                    + "/Facture_"
                    + QString::number(m_invoiceId) + ".pdf";

    InvoiceStyle style;
    InvoiceGenerator generator;
    if (!generator.generatePDF(m_invoiceId, pdfPath, style)) {
        QMessageBox::critical(this, "Erreur",
            "Impossible de générer le PDF");
        return;
    }

    QSqlQuery q;
    q.prepare("SELECT numero, client_nom FROM factures WHERE id = ?");
    q.addBindValue(m_invoiceId);
    QString numero = "Facture", clientNom = "";
    if (q.exec() && q.next()) {
        numero    = q.value(0).toString();
        clientNom = q.value(1).toString();
    }

    QProgressDialog progress("Envoi en cours...", "Annuler", 0, 0, this);
    progress.setWindowModality(Qt::WindowModal);
    progress.show();
    QApplication::processEvents();

    EmailSender sender;
    bool ok = sender.sendInvoiceEmail(email, clientNom, numero, pdfPath);
    progress.close();

    if (ok)
        QMessageBox::information(this, "Succès",
            QString("Facture %1 envoyée à %2").arg(numero, email));
    else
        QMessageBox::critical(this, "Erreur d'envoi",
            "Échec de l'envoi.\n\nVérifiez:\n"
            "• Votre connexion internet\n"
            "• Le mot de passe d'application Gmail\n"
            "• Que l'accès IMAP est activé dans Gmail");
}

void InvoiceActionDialog::onManagePayments()
{
    PaymentDialog dlg(m_invoiceId, this);
    dlg.exec();
}