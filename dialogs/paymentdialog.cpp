#include "dialogs/paymentdialog.h"
#include "database/database.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QDoubleSpinBox>
#include <QComboBox>
#include <QDateEdit>
#include <QTextEdit>
#include <QPushButton>
#include <QTableWidget>
#include <QHeaderView>
#include <QMessageBox>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QFrame>
#include <QScrollArea>

PaymentDialog::PaymentDialog(int invoiceId, QWidget *parent)
    : QDialog(parent), m_invoiceId(invoiceId)
{
    setWindowTitle("Gestion des Paiements");
    setMinimumSize(750, 700);
    resize(800, 750);

    // Style global cohérent avec InvoiceEditDialog
    setStyleSheet(
        "QDialog { background: #F7FAFC; }"
        "QGroupBox {"
        "  font-weight: bold;"
        "  font-size: 12px;"
        "  border: 1px solid #CBD5E0;"
        "  border-radius: 6px;"
        "  margin-top: 8px;"
        "  padding-top: 8px;"
        "  padding-bottom: 12px;"
        "  padding-left: 16px;"
        "  padding-right: 16px;"
        "  color: #2B6CB0;"
        "  background: white;"
        "}"
        "QGroupBox::title {"
        "  subcontrol-origin: margin;"
        "  left: 10px;"
        "  padding: 0 6px;"
        "}"
        "QLabel {"
        "  font-size: 12px;"
        "  color: #4A5568;"
        "}"
        "QLineEdit, QDoubleSpinBox, QDateEdit, QComboBox, QTextEdit {"
        "  border: 1px solid #CBD5E0;"
        "  border-radius: 4px;"
        "  padding: 5px 8px;"
        "  background: white;"
        "  min-height: 28px;"
        "  font-size: 13px;"
        "}"
        "QLineEdit:focus, QDoubleSpinBox:focus, QDateEdit:focus, QComboBox:focus, QTextEdit:focus {"
        "  border: 1px solid #3182CE;"
        "}"
        "QDoubleSpinBox::up-button, QDoubleSpinBox::down-button {"
        "  width: 20px;"
        "  border-left: 1px solid #CBD5E0;"
        "  background: #F7FAFC;"
        "}"
        "QComboBox::drop-down {"
        "  border-left: 1px solid #CBD5E0;"
        "  width: 28px;"
        "}"
       "QComboBox::down-arrow {"
"  image: none;"
"  border-left: 5px solid transparent;"
"  border-right: 5px solid transparent;"
"  border-top: 7px solid #4A5568;"
"  width: 0px;"
"  height: 0px;"
"}"
        "QComboBox QAbstractItemView {"
        "  border: 1px solid #CBD5E0;"
        "  background: white;"
        "  selection-background-color: #BEE3F8;"
        "}"
    );

    setupUI();
    loadInvoiceInfo();
    refreshPaymentsList();
}

void PaymentDialog::setupUI()
{
    // ============================================
    // CONFIGURATION DU DIALOG
    // ============================================
    this->setWindowTitle("Gestion des Paiements");
    this->setMinimumSize(600, 500);  // ← Taille minimale pour éviter l'écrasement
    this->resize(650, 600);          // ← Taille par défaut raisonnable

    QScrollArea *scroll = new QScrollArea(this);
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    // ← IMPORTANT: Politique de redimensionnement pour que le scrollArea prenne tout l'espace
    scroll->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    QWidget *container = new QWidget;
    // ← IMPORTANT: Définir une hauteur minimale pour le container pour forcer le scroll
    // si le contenu dépasse, mais pas avant
    container->setMinimumWidth(560);
    
    QVBoxLayout *mainLayout = new QVBoxLayout(container);
    mainLayout->setSpacing(10);
    mainLayout->setContentsMargins(16, 16, 16, 16);

    // ============================================
    // TITRE (inchangé)
    // ============================================
    QLabel *titleLabel = new QLabel("💳 Gestion des Paiements");
    titleLabel->setStyleSheet(
        "font-size:18px; font-weight:bold; color:#1B2A3B; "
        "padding-bottom:8px; border-bottom:2px solid #27AE60;"
        "background: transparent;"
    );
    mainLayout->addWidget(titleLabel);

    // ============================================
    // INFOS FACTURE (inchangé)
    // ============================================
    QGroupBox *infoGroup = new QGroupBox("📋 Informations Facture");
    QFormLayout *infoForm = new QFormLayout(infoGroup);
    infoForm->setSpacing(8);
    infoForm->setLabelAlignment(Qt::AlignRight | Qt::AlignVCenter);
    infoForm->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);

    factureNumLabel = new QLabel;
    totalTTCLabel = new QLabel;
    dejaPayeLabel = new QLabel;
    resteLabel = new QLabel;
    statutLabel = new QLabel;

    factureNumLabel->setStyleSheet("font-weight: bold; font-size: 13px; color: #2D3748;");
    totalTTCLabel->setStyleSheet("font-weight: bold; font-size: 13px; color: #2B6CB0;");
    dejaPayeLabel->setStyleSheet("font-weight: bold; font-size: 13px; color: #27AE60;");
    resteLabel->setStyleSheet("font-weight: bold; font-size: 14px; color: #E74C3C;");
    statutLabel->setStyleSheet("font-weight: bold; font-size: 13px; color: #2D3748;");

    infoForm->addRow("Numéro :", factureNumLabel);
    infoForm->addRow("Total TTC :", totalTTCLabel);
    infoForm->addRow("Déjà payé :", dejaPayeLabel);
    infoForm->addRow("Reste à payer :", resteLabel);
    infoForm->addRow("Statut :", statutLabel);

    mainLayout->addWidget(infoGroup);

    // ============================================
    // NOUVEAU PAIEMENT (inchangé)
    // ============================================
    QGroupBox *newPaymentGroup = new QGroupBox("💵 Nouveau Paiement");
    QVBoxLayout *paymentMainLayout = new QVBoxLayout(newPaymentGroup);
    paymentMainLayout->setSpacing(12);
    paymentMainLayout->setContentsMargins(16, 16, 16, 16);

    // --- Ligne 1 : Montant | Date | Méthode ---
    QHBoxLayout *fieldsRow1 = new QHBoxLayout;
    fieldsRow1->setSpacing(12);

    // Montant
    QVBoxLayout *montantLayout = new QVBoxLayout;
    montantLayout->setSpacing(4);
    QLabel *montantLabel = new QLabel("Montant *");
    montantLabel->setStyleSheet("font-weight:bold; color:#2D3748; font-size:12px;");
    montantSpinBox = new QDoubleSpinBox;
    montantSpinBox->setMaximum(999999.99);
    montantSpinBox->setDecimals(2);
    montantSpinBox->setMinimum(0.01);
    montantSpinBox->setSuffix(" MAD");
    montantSpinBox->setFixedHeight(32);
    montantLayout->addWidget(montantLabel);
    montantLayout->addWidget(montantSpinBox);
    fieldsRow1->addLayout(montantLayout, 1);

    // Date
    QVBoxLayout *dateLayout = new QVBoxLayout;
    dateLayout->setSpacing(4);
    QLabel *dateLabel = new QLabel("Date *");
    dateLabel->setStyleSheet("font-weight:bold; color:#2D3748; font-size:12px;");
    dateEdit = new QDateEdit(QDate::currentDate());
    dateEdit->setCalendarPopup(true);
    dateEdit->setDisplayFormat("dd/MM/yyyy");
    dateEdit->setFixedHeight(32);
    dateLayout->addWidget(dateLabel);
    dateLayout->addWidget(dateEdit);
    fieldsRow1->addLayout(dateLayout, 1);

    // Méthode
    QVBoxLayout *methodeLayout = new QVBoxLayout;
    methodeLayout->setSpacing(4);
    QLabel *methodeLabel = new QLabel("Méthode");
    methodeLabel->setStyleSheet("font-weight:bold; color:#2D3748; font-size:12px;");
    methodeCombo = new QComboBox;
    methodeCombo->addItems({"Espèce", "Virement", "Chèque", "Carte", "Autre"});
    methodeCombo->setFixedHeight(32);
    methodeLayout->addWidget(methodeLabel);
    methodeLayout->addWidget(methodeCombo);
    fieldsRow1->addLayout(methodeLayout, 1);
    paymentMainLayout->addLayout(fieldsRow1);

    // --- Notes ---
    QVBoxLayout *notesLayout = new QVBoxLayout;
    notesLayout->setSpacing(4);
    QLabel *notesLabel = new QLabel("Notes");
    notesLabel->setStyleSheet("font-weight:bold; color:#2D3748; font-size:12px;");
    notesEdit = new QTextEdit;
    notesEdit->setMaximumHeight(80);
    notesEdit->setMinimumHeight(60);
    notesEdit->setPlaceholderText("Notes optionnelles concernant ce paiement...");
    notesLayout->addWidget(notesLabel);
    notesLayout->addWidget(notesEdit);
    paymentMainLayout->addLayout(notesLayout);

    // --- BOUTONS (alignés à droite) ---
    QHBoxLayout *btnLayout = new QHBoxLayout;
    btnLayout->addStretch();

    cancelPaymentBtn = new QPushButton("❌ Annuler");
    cancelPaymentBtn->setFixedHeight(34);
    cancelPaymentBtn->setStyleSheet(
        "QPushButton {"
        "  background: #E53E3E;"
        "  color: white;"
        "  font-weight: bold;"
        "  padding: 0 16px;"
        "  border-radius: 4px;"
        "  border: none;"
        "  font-size: 12px;"
        "}"
        "QPushButton:hover { background: #C53030; }"
        "QPushButton:disabled { background: #FC8181; }"
    );
    cancelPaymentBtn->setCursor(Qt::PointingHandCursor);

    addBtn = new QPushButton("➕ Enregistrer");
    addBtn->setFixedHeight(34);
    addBtn->setStyleSheet(
        "QPushButton {"
        "  background: #27AE60;"
        "  color: white;"
        "  font-weight: bold;"
        "  padding: 0 16px;"
        "  border-radius: 4px;"
        "  border: none;"
        "  font-size: 12px;"
        "}"
        "QPushButton:hover { background: #219A52; }"
        "QPushButton:disabled { background: #A0AEC0; }"
    );
    addBtn->setCursor(Qt::PointingHandCursor);

    btnLayout->addWidget(cancelPaymentBtn);
    btnLayout->addSpacing(8);
    btnLayout->addWidget(addBtn);
    paymentMainLayout->addLayout(btnLayout);

    mainLayout->addWidget(newPaymentGroup);

    // ============================================
    // HISTORIQUE (inchangé)
    // ============================================
    QGroupBox *historyGroup = new QGroupBox("📜 Historique des Paiements");
    QVBoxLayout *historyLayout = new QVBoxLayout(historyGroup);
    historyLayout->setContentsMargins(12, 12, 12, 12);
    historyLayout->setSpacing(8);

    paymentsTable = new QTableWidget(0, 4);
    paymentsTable->setHorizontalHeaderLabels({"Date", "Montant", "Méthode", "Notes"});
    paymentsTable->horizontalHeader()->setStretchLastSection(true);
    paymentsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    paymentsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    paymentsTable->setSelectionMode(QAbstractItemView::SingleSelection);
    paymentsTable->setColumnWidth(0, 110);
    paymentsTable->setColumnWidth(1, 110);
    paymentsTable->setColumnWidth(2, 110);
    paymentsTable->setMinimumHeight(140);
    paymentsTable->setMaximumHeight(220);
    paymentsTable->setAlternatingRowColors(true);
    paymentsTable->verticalHeader()->setVisible(false);

    paymentsTable->setStyleSheet(
        "QTableWidget {"
        "  border: 1px solid #CBD5E0;"
        "  gridline-color: #EDF2F7;"
        "  background: white;"
        "  font-size: 12px;"
        "}"
        "QTableWidget::item {"
        "  padding: 6px 6px;"
        "}"
        "QHeaderView::section {"
        "  background: #2B6CB0;"
        "  color: white;"
        "  font-weight: bold;"
        "  font-size: 11px;"
        "  padding: 8px 4px;"
        "  border: none;"
        "  border-right: 1px solid #2C5282;"
        "}"
        "QHeaderView::section:last {"
        "  border-right: none;"
        "}"
        "QTableWidget::item:selected {"
        "  background: #BEE3F8;"
        "  color: #1A202C;"
        "}"
        "QTableWidget::item:alternate {"
        "  background: #F7FAFC;"
        "}"
    );

    historyLayout->addWidget(paymentsTable);
    mainLayout->addWidget(historyGroup);

    // ============================================
    // BOUTON FERMER - CORRIGÉ
    // ============================================
    QHBoxLayout *closeLayout = new QHBoxLayout;
    closeLayout->addStretch();

    closeBtn = new QPushButton("Fermer");
    closeBtn->setFixedHeight(34);
    closeBtn->setStyleSheet(
        "QPushButton {"
        "  background: #718096;"
        "  color: white;"
        "  padding: 0 20px;"
        "  border-radius: 4px;"
        "  border: none;"
        "  font-size: 12px;"
        "}"
        "QPushButton:hover { background: #4A5568; }"
    );
    closeBtn->setCursor(Qt::PointingHandCursor);
    // ← IMPORTANT: Forcer une hauteur minimale pour le widget bouton lui-même
    closeBtn->setMinimumHeight(34);

    closeLayout->addWidget(closeBtn);
    mainLayout->addLayout(closeLayout);

    // ============================================
    // AJOUT D'UN ESPACE EXTENSIBLE AVANT LE BOUTON FERMER
    // ← CECI EST LA CLÉ : ça pousse le bouton vers le bas mais visible
    // ============================================
    // Déjà fait implicitement par le VBoxLayout, mais on peut forcer:
    // mainLayout->addStretch(1);  // ← Décommentez si le bouton remonte trop

    // Scroll
    scroll->setWidget(container);
    
    // ============================================
    // LAYOUT DU DIALOG - CORRIGÉ
    // ============================================
    QVBoxLayout *dialogLayout = new QVBoxLayout(this);
    dialogLayout->setContentsMargins(0, 0, 0, 0);
    dialogLayout->setSpacing(0);  // ← Pas d'espace entre scrollArea et bords
    dialogLayout->addWidget(scroll);

    // ============================================
    // CONNEXIONS (inchangées)
    // ============================================
    connect(addBtn, &QPushButton::clicked, this, &PaymentDialog::onAddPayment);
    connect(cancelPaymentBtn, &QPushButton::clicked, this, &PaymentDialog::onCancelPayment);
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::accept);
    connect(montantSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &PaymentDialog::updateResteDisplay);
}
void PaymentDialog::loadInvoiceInfo()
{
    QSqlQuery q;
    q.prepare("SELECT numero, total_ttc, statut FROM factures WHERE id = ?");
    q.addBindValue(m_invoiceId);

    if (q.exec() && q.next()) {
        factureNumLabel->setText(q.value(0).toString());
        m_totalTTC = q.value(1).toDouble();
        totalTTCLabel->setText(QString::number(m_totalTTC, 'f', 2) + " MAD");
        statutLabel->setText(q.value(2).toString());
    } else {
        QMessageBox::critical(this, "Erreur", "Facture introuvable");
        reject();
        return;
    }

    m_totalPaid = getTotalPaid();
    updateResteDisplay();
    updateButtonsState();
}

double PaymentDialog::getTotalPaid() const
{
    QSqlQuery q;
    q.prepare("SELECT COALESCE(SUM(montant), 0) FROM paiements WHERE facture_id = ?");
    q.addBindValue(m_invoiceId);
    if (q.exec() && q.next()) {
        return q.value(0).toDouble();
    }
    return 0.0;
}

void PaymentDialog::updateResteDisplay()
{
    double nouveauPaiement = montantSpinBox->value();
    double reste = m_totalTTC - m_totalPaid - nouveauPaiement;
    reste = qRound(reste * 100.0) / 100.0;

    dejaPayeLabel->setText(QString::number(m_totalPaid, 'f', 2) + " MAD");

    if (reste < -0.01) {
        resteLabel->setText("⚠️ Excédent: " + QString::number(-reste, 'f', 2) + " MAD");
        resteLabel->setStyleSheet("font-weight:bold; font-size:14px; color:#F39C12;");
        addBtn->setEnabled(false);
        addBtn->setText("❌ Montant excède le reste");
    } else if (reste <= 0.01) {
        resteLabel->setText("✅ Solde: 0.00 MAD (Facture soldée)");
        resteLabel->setStyleSheet("font-weight:bold; font-size:14px; color:#27AE60;");
        addBtn->setEnabled(true);
        addBtn->setText("✅ Enregistrer et solder");
    } else {
        resteLabel->setText("Reste: " + QString::number(reste, 'f', 2) + " MAD");
        resteLabel->setStyleSheet("font-weight:bold; font-size:14px; color:#E74C3C;");
        addBtn->setEnabled(true);
        addBtn->setText("➕ Enregistrer");
    }
}

void PaymentDialog::updateButtonsState()
{
    cancelPaymentBtn->setEnabled(m_totalPaid > 0);
}

void PaymentDialog::refreshPaymentsList()
{
    paymentsTable->setRowCount(0);

    QSqlQuery q;
    q.prepare("SELECT id, date_paiement, montant, methode, notes "
              "FROM paiements WHERE facture_id = ? ORDER BY date_paiement DESC, id DESC");
    q.addBindValue(m_invoiceId);

    if (!q.exec()) {
        qDebug() << "Erreur chargement paiements:" << q.lastError().text();
        return;
    }

    while (q.next()) {
        int row = paymentsTable->rowCount();
        paymentsTable->insertRow(row);

        paymentsTable->setItem(row, 0, new QTableWidgetItem(q.value(1).toDate().toString("dd/MM/yyyy")));
        paymentsTable->setItem(row, 1, new QTableWidgetItem(QString::number(q.value(2).toDouble(), 'f', 2) + " MAD"));
        paymentsTable->setItem(row, 2, new QTableWidgetItem(q.value(3).toString()));
        paymentsTable->setItem(row, 3, new QTableWidgetItem(q.value(4).toString()));

        paymentsTable->setRowHeight(row, 30);
    }

    updateButtonsState();
}
void PaymentDialog::onAddPayment()
{
    double montant = montantSpinBox->value();

    if (montant <= 0) {
        QMessageBox::warning(this, "Erreur", "Le montant doit être supérieur à 0");
        return;
    }

    double reste = qRound((m_totalTTC - m_totalPaid) * 100.0) / 100.0;

    if (montant > reste + 0.01) {
        QMessageBox::warning(this, "Erreur",
            QString("Le montant (%1) dépasse le reste à payer (%2)")
            .arg(montant, 0, 'f', 2).arg(reste, 0, 'f', 2));
        return;
    }

    // ── Insérer le paiement ──────────────────────────────────────────────────
    QSqlQuery q;
    q.prepare("INSERT INTO paiements (facture_id, montant, date_paiement, methode, notes) "
              "VALUES (?, ?, ?, ?, ?)");
    q.addBindValue(m_invoiceId);
    q.addBindValue(montant);
    q.addBindValue(dateEdit->date().toString("yyyy-MM-dd"));
    q.addBindValue(methodeCombo->currentText());
    q.addBindValue(notesEdit->toPlainText().trimmed());

    if (!q.exec()) {
        QMessageBox::critical(this, "Erreur",
            "Échec de l'enregistrement:\n" + q.lastError().text());
        return;
    }

    // ── Recalculer le total payé depuis la DB ────────────────────────────────
    double nouveauTotalPaye = getTotalPaid();
    double nouveauReste = qRound((m_totalTTC - nouveauTotalPaye) * 100.0) / 100.0;

    // ── Mettre à jour le statut manuellement (sans dépendre du trigger) ──────
    QString nouveauStatut;
    if (nouveauReste <= 0.01) {
        nouveauStatut = "Payée";
    } else if (nouveauTotalPaye > 0) {
        nouveauStatut = "Partiellement payée";
    } else {
        nouveauStatut = "Envoyée";
    }

    QSqlQuery updateStatut;
    updateStatut.prepare("UPDATE factures SET statut = ?, "
                         "montant_paye = ?, reste_a_payer = ? "
                         "WHERE id = ?");
    updateStatut.addBindValue(nouveauStatut);
    updateStatut.addBindValue(nouveauTotalPaye);
    updateStatut.addBindValue(qMax(0.0, nouveauReste));
    updateStatut.addBindValue(m_invoiceId);

    if (!updateStatut.exec()) {
        qDebug() << "Erreur UPDATE statut:" << updateStatut.lastError().text();
    }

    QMessageBox::information(this, "Succès",
        QString("Paiement de %1 MAD enregistré.\nStatut: %2")
        .arg(montant, 0, 'f', 2).arg(nouveauStatut));

    montantSpinBox->setValue(0.01);
    notesEdit->clear();
    m_totalPaid = nouveauTotalPaye;
    updateResteDisplay();
    refreshPaymentsList();

    // ── Rafraîchir le label statut ───────────────────────────────────────────
    statutLabel->setText(nouveauStatut);
}
void PaymentDialog::onCancelPayment()
{
    QSqlQuery q;
    q.prepare("SELECT id, montant, date_paiement FROM paiements "
              "WHERE facture_id = ? ORDER BY id DESC LIMIT 1");
    q.addBindValue(m_invoiceId);

    if (!q.exec() || !q.next()) {
        QMessageBox::warning(this, "Information", "Aucun paiement à annuler");
        return;
    }

    int paymentId = q.value(0).toInt();
    double montant = q.value(1).toDouble();
    QString date = q.value(2).toDate().toString("dd/MM/yyyy");

    QMessageBox::StandardButton reply = QMessageBox::question(this, "Confirmation",
        QString("Annuler le dernier paiement ?\n\n"
                "Montant: %1 MAD\n"
                "Date: %2\n\n"
                "Le statut de la facture sera recalculé.")
        .arg(montant, 0, 'f', 2).arg(date),
        QMessageBox::Yes | QMessageBox::No);

    if (reply != QMessageBox::Yes) {
        return;
    }

    QSqlQuery del;
    del.prepare("DELETE FROM paiements WHERE id = ?");
    del.addBindValue(paymentId);

    if (!del.exec()) {
        QMessageBox::critical(this, "Erreur", 
            "Échec de la suppression:\n" + del.lastError().text());
        return;
    }
// ── Recalculer après suppression ─────────────────────────────────────────
    double nouveauTotalPaye = getTotalPaid();
    double nouveauReste = qRound((m_totalTTC - nouveauTotalPaye) * 100.0) / 100.0;

    QString nouveauStatut;
    if (nouveauTotalPaye <= 0) {
        nouveauStatut = "Envoyée";
    } else if (nouveauReste <= 0.01) {
        nouveauStatut = "Payée";
    } else {
        nouveauStatut = "Partiellement payée";
    }

    QSqlQuery updateStatut;
    updateStatut.prepare("UPDATE factures SET statut = ?, "
                         "montant_paye = ?, reste_a_payer = ? "
                         "WHERE id = ?");
    updateStatut.addBindValue(nouveauStatut);
    updateStatut.addBindValue(nouveauTotalPaye);
    updateStatut.addBindValue(qMax(0.0, nouveauReste));
    updateStatut.addBindValue(m_invoiceId);
    updateStatut.exec();

    QMessageBox::information(this, "Succès",
        "Paiement annulé.\nStatut mis à jour: " + nouveauStatut);

    m_totalPaid = nouveauTotalPaye;
    updateResteDisplay();
    refreshPaymentsList();
    statutLabel->setText(nouveauStatut);
    QMessageBox::information(this, "Succès", 
        "Paiement annulé.\nLe statut a été mis à jour automatiquement.");

    m_totalPaid = getTotalPaid();
    updateResteDisplay();
    refreshPaymentsList();

    QSqlQuery sq;
    sq.prepare("SELECT statut FROM factures WHERE id = ?");
    sq.addBindValue(m_invoiceId);
    if (sq.exec() && sq.next()) {
        statutLabel->setText(sq.value(0).toString());
    }
}