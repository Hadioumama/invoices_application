#include "invoiceactiondialog.h"
#include <QColorDialog>
#include "utils/invoicegenerator.h"
#include "utils/emailsender.h"
#include "database/database.h"
#include "dialogs/paymentdialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QDir>
#include <QGroupBox>
#include <QFormLayout>
#include <QLineEdit>
#include <QProgressDialog>
#include <QApplication>
#include "utils/emailsender.h"
#include <QPushButton>
#include <QMessageBox>
#include <QFileDialog>
#include <QPrinter>
#include <QPrintDialog>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QDesktopServices>
#include <QUrl>

InvoiceActionDialog::InvoiceActionDialog(int invoiceId, QWidget *parent)
    : QDialog(parent), m_invoiceId(invoiceId)
{
    setupUI();

    // Load client email
    QSqlQuery query;
    query.prepare("SELECT c.email FROM factures f JOIN clients c ON f.client_id = c.id WHERE f.id = ?");
    query.addBindValue(invoiceId);
    
    if (query.exec() && query.next()) {
        emailEdit->setText(query.value(0).toString());
    }
}

void InvoiceActionDialog::setupUI()
{
    setWindowTitle("Actions sur la Facture");
    setGeometry(200, 200, 500, 250);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    mainLayout->addWidget(new QLabel("Sélectionnez une action:"));

    // Action buttons
    QHBoxLayout *buttonLayout = new QHBoxLayout;

    pdfBtn = new QPushButton("📄 Exporter PDF");
    printBtn = new QPushButton("🖨️ Imprimer");
    emailBtn = new QPushButton("📧 Envoyer par Email");
    paymentBtn = new QPushButton("💳 Gérer les Paiements");
paymentBtn->setStyleSheet(
    "background:#9B59B6; color:white; font-weight:bold; "
    "padding:8px 16px; border-radius:4px; border:none;"
);

    connect(pdfBtn, &QPushButton::clicked, this, &InvoiceActionDialog::onExportPDF);
    connect(printBtn, &QPushButton::clicked, this, &InvoiceActionDialog::onPrint);
    connect(emailBtn, &QPushButton::clicked, this, &InvoiceActionDialog::onSendEmail);

    buttonLayout->addWidget(pdfBtn);
    buttonLayout->addWidget(printBtn);
    buttonLayout->addWidget(emailBtn);
    buttonLayout->addWidget(paymentBtn);

    mainLayout->addLayout(buttonLayout);
    
    // Email section
    mainLayout->addWidget(new QLabel("Email pour envoi:"));
    emailEdit = new QLineEdit;
    emailEdit->setPlaceholderText("email@example.com");
    mainLayout->addWidget(emailEdit);

    // Close button
    QHBoxLayout *closeLayout = new QHBoxLayout;
    closeLayout->addStretch();
    closeBtn = new QPushButton("Fermer");
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::accept);
    closeLayout->addWidget(closeBtn);
    mainLayout->addLayout(closeLayout);
    connect(paymentBtn, &QPushButton::clicked, this, &InvoiceActionDialog::onManagePayments);
}

void InvoiceActionDialog::onExportPDF()
{
    InvoiceStyle style;

    // ===== CHOIX FORMAT =====
    QMessageBox msgBox(this);
    msgBox.setWindowTitle("Export PDF - Facture");
    msgBox.setIcon(QMessageBox::Question);
    msgBox.setText(
        "<b>Comment souhaitez-vous générer votre facture ?</b><br><br>"
        "<small style='color:#666;'>"
        "Le format standard convient aux particuliers.<br>"
        "La personnalisation est recommandée pour les entreprises."
        "</small>"
    );
    QPushButton *standardBtn = msgBox.addButton(
        "📄 Format Standard", QMessageBox::AcceptRole);
    QPushButton *persoBtn = msgBox.addButton(
        "🏢 Personnaliser (Entreprise)", QMessageBox::ActionRole);
    QPushButton *cancelBtn = msgBox.addButton(
        "❌ Annuler", QMessageBox::RejectRole);
    Q_UNUSED(standardBtn)
    msgBox.exec();

    // Annulé
    if (msgBox.clickedButton() == cancelBtn) return;

    // ===== PERSONNALISATION ENTREPRISE =====
    if (msgBox.clickedButton() == persoBtn) {

        QDialog dlg(this);
        dlg.setWindowTitle("🏢 Personnalisation Entreprise");
        dlg.setMinimumWidth(520);

        QVBoxLayout *mainLayout = new QVBoxLayout(&dlg);

        // Info lisibilité
        QLabel *infoLabel = new QLabel(
            "<b>💡 Conseils pour une meilleure lisibilité:</b><br>"
            "• Logo: format PNG transparent, min 200x80px<br>"
            "• Signature: fond blanc ou transparent, min 150x60px<br>"
            "• Choisissez une couleur qui contraste bien avec le blanc"
        );
        infoLabel->setWordWrap(true);
        infoLabel->setStyleSheet(
            "background:#EBF5FB;border:1px solid #AED6F1;"
            "border-radius:6px;padding:10px;font-size:11px;"
            "color:#1A5276;margin-bottom:6px;"
        );
        mainLayout->addWidget(infoLabel);

        // Formulaire
        QGroupBox *infoGroup = new QGroupBox("Informations Entreprise");
        QFormLayout *form = new QFormLayout(infoGroup);
        form->setSpacing(8);

        QLineEdit *nameE = new QLineEdit(style.companyName);
        QLineEdit *addrE = new QLineEdit(style.companyAddress);
        QLineEdit *telE  = new QLineEdit(style.companyPhone);
        QLineEdit *mailE = new QLineEdit(style.companyEmail);
        QLineEdit *webE  = new QLineEdit(style.companyWebsite);
        QLineEdit *iceE  = new QLineEdit(style.companyICE);

        nameE->setPlaceholderText("Ex: ACME Corporation");
        addrE->setPlaceholderText("Ex: 123 Rue Al Hoceima, Maroc");
        telE->setPlaceholderText("Ex: +212 5XX XXX XXX");
        mailE->setPlaceholderText("Ex: contact@entreprise.com");
        webE->setPlaceholderText("Ex: www.entreprise.com");
        iceE->setPlaceholderText("Ex: 000000000000000");

        form->addRow("Nom entreprise:*", nameE);
        form->addRow("Adresse:", addrE);
        form->addRow("Téléphone:", telE);
        form->addRow("Email:", mailE);
        form->addRow("Site web:", webE);
        form->addRow("ICE:", iceE);
        mainLayout->addWidget(infoGroup);

        // Groupe personnalisation visuelle
        QGroupBox *visualGroup = new QGroupBox("Personnalisation Visuelle");
        QFormLayout *visualForm = new QFormLayout(visualGroup);
        visualForm->setSpacing(8);

        // Couleur principale
        QString selColor = style.primaryColor;
        QPushButton *colorBtn = new QPushButton(
            "🎨  Couleur actuelle: " + selColor);
        colorBtn->setStyleSheet(
            QString("background:%1;color:white;font-weight:bold;"
                    "padding:7px;border-radius:4px;border:none;")
            .arg(selColor));
        connect(colorBtn, &QPushButton::clicked, [&](){
            QColor c = QColorDialog::getColor(
                QColor(selColor), &dlg,
                "Choisir la couleur principale");
            if (c.isValid()) {
                selColor = c.name();
                colorBtn->setText("🎨  Couleur: " + selColor);
                colorBtn->setStyleSheet(
                    QString("background:%1;color:white;"
                            "font-weight:bold;padding:7px;"
                            "border-radius:4px;border:none;")
                    .arg(selColor));
            }
        });
        visualForm->addRow("Couleur:", colorBtn);

        // Logo
        QLineEdit *logoE = new QLineEdit;
        logoE->setPlaceholderText("Aucun logo sélectionné...");
        logoE->setReadOnly(true);
        QPushButton *logoB = new QPushButton("📁 Parcourir");
        logoB->setFixedWidth(100);
        logoB->setStyleSheet(
            "background:#3498DB;color:white;padding:5px;"
            "border-radius:4px;border:none;");
        connect(logoB, &QPushButton::clicked, [&](){
            QString p = QFileDialog::getOpenFileName(
                &dlg,
                "Choisir le logo de l'entreprise",
                QDir::homePath(),
                "Images (*.png *.jpg *.jpeg)");
            if (!p.isEmpty()) logoE->setText(p);
        });
        QHBoxLayout *lRow = new QHBoxLayout;
        lRow->addWidget(logoE);
        lRow->addWidget(logoB);
        lRow->setSpacing(6);
        visualForm->addRow("Logo entreprise:", lRow);

        // Signature
        QLineEdit *signE = new QLineEdit;
        signE->setPlaceholderText("Aucune signature sélectionnée...");
        signE->setReadOnly(true);
        QPushButton *signB = new QPushButton("📁 Parcourir");
        signB->setFixedWidth(100);
        signB->setStyleSheet(
            "background:#3498DB;color:white;padding:5px;"
            "border-radius:4px;border:none;");
        connect(signB, &QPushButton::clicked, [&](){
            QString p = QFileDialog::getOpenFileName(
                &dlg,
                "Choisir la signature",
                QDir::homePath(),
                "Images (*.png *.jpg *.jpeg)");
            if (!p.isEmpty()) signE->setText(p);
        });
        QHBoxLayout *sRow = new QHBoxLayout;
        sRow->addWidget(signE);
        sRow->addWidget(signB);
        sRow->setSpacing(6);
        visualForm->addRow("Signature:", sRow);

        mainLayout->addWidget(visualGroup);

        // Boutons
        QHBoxLayout *btnRow = new QHBoxLayout;
        QPushButton *okB  = new QPushButton("✅ Appliquer et générer PDF");
        QPushButton *canB = new QPushButton("Annuler");
        okB->setFixedHeight(36);
        canB->setFixedHeight(36);
        okB->setStyleSheet(
            "background:#27AE60;color:white;font-weight:bold;"
            "padding:8px 16px;border-radius:4px;border:none;");
        canB->setStyleSheet(
            "background:#E74C3C;color:white;padding:8px 16px;"
            "border-radius:4px;border:none;");
        btnRow->addStretch();
        btnRow->addWidget(canB);
        btnRow->addWidget(okB);
        mainLayout->addLayout(btnRow);

        connect(okB,  &QPushButton::clicked, &dlg, &QDialog::accept);
        connect(canB, &QPushButton::clicked, &dlg, &QDialog::reject);

        if (dlg.exec() == QDialog::Accepted) {
            style.companyName    = nameE->text().trimmed();
            style.companyAddress = addrE->text().trimmed();
            style.companyPhone   = telE->text().trimmed();
            style.companyEmail   = mailE->text().trimmed();
            style.companyWebsite = webE->text().trimmed();
            style.companyICE     = iceE->text().trimmed();
            style.primaryColor   = selColor;
            style.logoPath       = logoE->text();
            style.signaturePath  = signE->text();
        } else {
            return;
        }
    }

    // ===== CHOISIR DESTINATION PDF =====
    QString defaultName = "Facture_" + QString::number(m_invoiceId) + ".pdf";
    QString fileName = QFileDialog::getSaveFileName(
        this,
        "Enregistrer la facture PDF",
        InvoiceGenerator::getPdfOutputPath() + "/" + defaultName,
        "Fichiers PDF (*.pdf)");

    if (fileName.isEmpty()) return;

    // ===== GÉNÉRER PDF =====
    InvoiceGenerator gen;
    if (gen.generatePDF(m_invoiceId, fileName, style)) {
        QMessageBox::information(this, "✅ Succès",
            "La facture PDF a été générée avec succès:\n\n" + fileName +
            "\n\nLe fichier va s'ouvrir automatiquement.");
        QDesktopServices::openUrl(QUrl::fromLocalFile(fileName));
    } else {
        QMessageBox::critical(this, "❌ Erreur",
            "Impossible de générer le PDF.\n\n"
            "Vérifiez que:\n"
            "• La facture contient au moins un article\n"
            "• Le dossier de destination est accessible");
    }
}
void InvoiceActionDialog::onPrint()
{
    InvoiceStyle style;
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

    // Générer PDF d'abord
    QString pdfPath = InvoiceGenerator::getPdfOutputPath() +
                      "/Facture_" + QString::number(m_invoiceId) + ".pdf";

    InvoiceStyle style;
    InvoiceGenerator generator;
    if (!generator.generatePDF(m_invoiceId, pdfPath, style)) {
        QMessageBox::critical(this, "Erreur",
                              "Impossible de générer le PDF");
        return;
    }

    // Récupérer infos
    QSqlQuery q;
    q.prepare("SELECT numero, client_nom FROM factures WHERE id = ?");
    q.addBindValue(m_invoiceId);
    QString numero = "Facture";
    QString clientNom = "";
    if (q.exec() && q.next()) {
        numero    = q.value(0).toString();
        clientNom = q.value(1).toString();
    }

    // Afficher progression
    QProgressDialog progress("Envoi en cours...", "Annuler", 0, 0, this);
    progress.setWindowModality(Qt::WindowModal);
    progress.show();
    QApplication::processEvents();

    EmailSender sender;
    bool ok = sender.sendInvoiceEmail(email, clientNom, numero, pdfPath);

    progress.close();

    if (ok) {
        QMessageBox::information(this, "Succès",
            QString("Facture %1 envoyée à %2").arg(numero, email));
    } else {
        QMessageBox::critical(this, "Erreur d'envoi",
            "Échec de l'envoi.\n\n"
            "Vérifiez:\n"
            "• Votre connexion internet\n"
            "• Le mot de passe d'application Gmail\n"
            "• Que l'accès IMAP est activé dans Gmail");
    }
}
void InvoiceActionDialog::onManagePayments()
{
    PaymentDialog dlg(m_invoiceId, this);
    dlg.exec();
}