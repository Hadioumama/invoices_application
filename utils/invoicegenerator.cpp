#include "utils/invoicegenerator.h"
#include "database/database.h"
#include <QPrinter>
#include <QTextDocument>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QStandardPaths>
#include <QDir>
#include <QPageLayout>
#include <QPageSize>
#include <QFile>
#include <QFileInfo>
#include <QProcess>
#include <QTemporaryFile>

InvoiceGenerator::InvoiceGenerator() {}

QString InvoiceGenerator::getPdfOutputPath()
{
    QString path = QStandardPaths::writableLocation(
                       QStandardPaths::DocumentsLocation) + "/Factures";
    QDir().mkpath(path);
    return path;
}

QString InvoiceGenerator::getInvoiceFileName(const QString &numeroFacture)
{
    QString n = numeroFacture;
    return QString("Facture_%1.pdf").arg(n.replace("/","_").replace(" ","_"));
}

static QString imageToBase64Html(const QString &path, int maxW, int maxH)
{
    if (path.isEmpty() || !QFile::exists(path)) return "";
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly)) return "";
    QByteArray data = f.readAll();
    QString ext = QFileInfo(path).suffix().toLower();
    QString mime = (ext == "png") ? "image/png" : "image/jpeg";
    return QString("<img src='data:%1;base64,%2' width='%3' height='%4' style='max-width:%3px;max-height:%4px;'/>")
           .arg(mime, QString(data.toBase64()))
           .arg(maxW).arg(maxH);
}
bool InvoiceGenerator::generatePDF(int invoiceId,
                                    const QString &filePath,
                                    const InvoiceStyle &style)
{
    // Données facture
    QSqlQuery q;
    q.prepare("SELECT numero, type, client_nom, client_adresse, "
              "client_tel, client_email, date_creation, date_echeance, "
              "total_ht, total_tva, total_ttc, statut "
              "FROM factures WHERE id = ?");
    q.addBindValue(invoiceId);
    if (!q.exec() || !q.next()) {
        qDebug() << "Facture non trouvée:" << invoiceId;
        return false;
    }

    QString numero       = q.value(0).toString();
    QString type         = q.value(1).toString();
    QString clientNom    = q.value(2).toString();
    QString clientAddr   = q.value(3).toString();
    QString clientTel    = q.value(4).toString();
    QString clientEmail  = q.value(5).toString();
    QString dateCreation = q.value(6).toDate().toString("dd/MM/yyyy");
    QString dateEcheance = q.value(7).toDate().toString("dd/MM/yyyy");
    double  totalHT      = q.value(8).toDouble();
    double  totalTVA     = q.value(9).toDouble();
    double  totalTTC     = q.value(10).toDouble();
    QString statut       = q.value(11).toString();

    // Logo
    QString logoHtml = imageToBase64Html(style.logoPath, 180, 90);
    if (logoHtml.isEmpty())
        logoHtml = QString("<span style='font-size:28px;font-weight:900;color:white;letter-spacing:2px;'>LOGO HERE</span>");

    // Lignes articles
    QSqlQuery lq;
    lq.prepare("SELECT designation, quantite, prix_unitaire_ht, taux_tva "
               "FROM lignes_facture WHERE facture_id = ?");
    lq.addBindValue(invoiceId);

    QString rowsHtml;
    int rowNum = 1;
    if (lq.exec()) {
        while (lq.next()) {
            QString des  = lq.value(0).toString().toHtmlEscaped();
            int     qty  = lq.value(1).toInt();
            double  prix = lq.value(2).toDouble();
            Q_UNUSED(lq.value(3).toDouble());
            double  tot  = qty * prix;
            
            rowsHtml += QString(
                "<tr style='border-bottom:1px solid #e0e0e0;'>"
                "<td style='padding:10px 15px; font-size:12px; color:#333;'>%1. %2</td>"
                "<td style='padding:10px 15px; font-size:12px; color:#555; text-align:center;'>%3</td>"
                "<td style='padding:10px 15px; font-size:12px; color:#555; text-align:center;'>$%4</td>"
                "<td style='padding:10px 15px; font-size:12px; color:#555; text-align:center; font-weight:bold;'>$%5</td>"
                "</tr>"
            ).arg(rowNum).arg(des).arg(qty)
             .arg(QString::number(prix,'f',2))
             .arg(QString::number(tot,'f',2));
            
                      rowNum++;
        }
    }

    // ============================================
    // CALCUL HAUTEUR ÉLASTIQUE (AVANT QString html)
    // ============================================
   int elasticHeight = 80;   // Essayez 150, 200, 250, 300 selon besoin

    QString html;

    // ============================================
    // HTML ULTRA-SIMPLIFIÉ - 100% QTEXTDOCUMENT
    // ============================================
    html = QString(
        "<!DOCTYPE html>"
        "<html>"
        "<head><meta charset='UTF-8'></head>"
        "<body style='margin:0; padding:0; font-family:Arial,sans-serif;'>"

        // TABLE PRINCIPALE - CONTENEUR
        "<table width='600' align='center' cellpadding='0' cellspacing='0' style='background:white;'>"
        "<tr><td>"

        // HEADER BLEU
        "<table width='600' cellpadding='0' cellspacing='0' style='background:#4db8e8;'>"
        "<tr>"
        "<td style='padding:25px 40px; width:50%;'>"
        "<span style='font-size:14px; font-weight:bold; letter-spacing:2px; color:white;'>LOGO HERE</span>"
        "</td>"
        "<td style='padding:25px 40px; width:50%; text-align:right;'>"
        "<span style='font-size:28px; font-weight:bold; letter-spacing:3px; color:white;'>INVOICE</span>"
        "</td>"
        "</tr>"
        "</table>"

        // ESPACE
        "<table width='600' cellpadding='0' cellspacing='0'><tr><td height='25'></td></tr></table>"

        // INFOS CLIENT + FACTURE
        "<table width='600' cellpadding='0' cellspacing='0'>"
        "<tr>"
        "<td style='padding:0 40px; width:60%;'>"
        "<p style='color:#4db8e8; font-size:13px; font-weight:bold; margin:0 0 8px 0; text-transform:uppercase;'>Invoice to:</p>"
        "<p style='color:#666; font-size:12px; line-height:1.6; margin:0;'>"
        "<b>Name:</b> %1<br>%2<br>%3"
        "</p>"
        "</td>"
        "<td style='padding:0 40px; width:40%; text-align:right;'>"
        "<p style='margin:0; color:#666; font-size:12px;'>Invoice No: <b style='color:#333;'>%4</b></p>"
        "<p style='margin:0; color:#666; font-size:12px;'>Date: <b style='color:#333;'>%5</b></p>"
        "</td>"
        "</tr>"
        "</table>"

        // ESPACE
        "<table width='600' cellpadding='0' cellspacing='0'><tr><td height='25'></td></tr></table>"

        // TABLEAU ARTICLES
        "<table width='520' align='center' cellpadding='0' cellspacing='0'>"
        "<tr style='background:#4db8e8;'>"
        "<td style='padding:10px 15px; color:white; font-size:12px; font-weight:bold; text-transform:uppercase; width:50%;'>Description</td>"
        "<td style='padding:10px 15px; color:white; font-size:12px; font-weight:bold; text-transform:uppercase; text-align:center; width:16%;'>Qty</td>"
        "<td style='padding:10px 15px; color:white; font-size:12px; font-weight:bold; text-transform:uppercase; text-align:center; width:17%;'>Price</td>"
        "<td style='padding:10px 15px; color:white; font-size:12px; font-weight:bold; text-transform:uppercase; text-align:center; width:17%;'>Total</td>"
        "</tr>"
        "%6"
        "</table>"

        // ESPACE
        "<table width='600' cellpadding='0' cellspacing='0'><tr><td height='20'></td></tr></table>"

        // TOTALS SECTION
        "<table width='600' cellpadding='0' cellspacing='0'>"
        "<tr>"
        "<td style='padding:0 40px; width:50%;'>"
        "<p style='color:#4db8e8; font-size:11px; font-weight:bold; margin:0 0 8px 0; text-transform:uppercase;'>Bank Info:</p>"
        "<p style='font-size:10px; color:#666; line-height:1.5; margin:0;'>"
        "Bank Name: %7<br>"
        "Bank Account: %8<br>"
        "Code: %9"
        "</p>"
        "<p style='color:#4db8e8; font-size:11px; font-weight:bold; margin:15px 0 8px 0; text-transform:uppercase;'>Payment Info:</p>"
        "<p style='font-size:10px; color:#666; line-height:1.5; margin:0;'>"
        "Account: %10<br>"
        "A/C Name: %11<br>"
        "Bank Details: %12"
        "</p>"
        "</td>"
        "<td style='padding:0 40px; width:50%; text-align:right;'>"
        "<table width='260' cellpadding='0' cellspacing='0' style='background:#e8f4fc;' align='right'>"
        "<tr>"
        "<td style='padding:8px 15px; color:#666; font-size:12px; text-align:right;'>Sub Total:</td>"
        "<td style='padding:8px 15px; color:#333; font-size:12px; font-weight:bold; text-align:right;'>$%13</td>"
        "</tr>"
        "<tr>"
        "<td style='padding:8px 15px; color:#666; font-size:12px; text-align:right;'>Tax:</td>"
        "<td style='padding:8px 15px; color:#333; font-size:12px; font-weight:bold; text-align:right;'>%14</td>"
        "</tr>"
        "<tr>"
        "<td style='padding:8px 15px; color:#666; font-size:12px; text-align:right;'>Tax Rate:</td>"
        "<td style='padding:8px 15px; color:#333; font-size:12px; font-weight:bold; text-align:right;'>%15</td>"
        "</tr>"
        "<tr><td colspan='2' style='border-top:1px solid #4db8e8; height:8px;'></td></tr>"
        "<tr>"
        "<td style='padding:8px 15px; color:#4db8e8; font-size:14px; font-weight:bold; text-align:right;'>TOTAL:</td>"
        "<td style='padding:8px 15px; color:#4db8e8; font-size:14px; font-weight:bold; text-align:right;'>$%16</td>"
        "</tr>"
        "</table>"
        "</td>"
        "</tr>"
        "</table>"

        // ESPACE ÉLASTIQUE
        "<table width='600' cellpadding='0' cellspacing='0'><tr><td height='%21'></td></tr></table>"

        // TERMS & SIGNATURE
        "<table width='600' cellpadding='0' cellspacing='0'>"
        "<tr>"
        "<td style='padding:0 40px 30px 40px; width:70%;'>"
        "<p style='color:#4db8e8; font-size:11px; font-weight:bold; margin:0 0 8px 0; text-transform:uppercase;'>Terms & Conditions:</p>"
        "<p style='font-size:9px; color:#999; line-height:1.4; margin:0;'>"
        "%17"
        "</p>"
        "</td>"
        "<td style='padding:0 40px 30px 40px; width:30%; text-align:right; vertical-align:bottom;'>"
        "<p style='font-size:11px; color:#666; font-style:italic; margin:0;'>Authorized Sign</p>"
        "</td>"
        "</tr>"
        "</table>"

        // FOOTER BLEU
        "<table width='600' cellpadding='0' cellspacing='0' style='background:#4db8e8;'>"
        "<tr>"
        "<td style='padding:15px 40px; font-size:10px; color:white; text-align:left; width:33%;'>"
        "<span style='display:inline-block; width:20px; height:20px; background:rgba(255,255,255,0.3); border-radius:50%; text-align:center; line-height:20px; margin-right:8px;'>&#9742;</span>"
        "%18"
        "</td>"
        "<td style='padding:15px 40px; font-size:10px; color:white; text-align:center; width:33%;'>"
        "<span style='display:inline-block; width:20px; height:20px; background:rgba(255,255,255,0.3); border-radius:50%; text-align:center; line-height:20px; margin-right:8px;'>&#9993;</span>"
        "%19"
        "</td>"
        "<td style='padding:15px 40px; font-size:10px; color:white; text-align:right; width:33%;'>"
        "<span style='display:inline-block; width:20px; height:20px; background:rgba(255,255,255,0.3); border-radius:50%; text-align:center; line-height:20px; margin-right:8px;'>&#127760;</span>"
        "%20"
        "</td>"
        "</tr>"
        "</table>"

        "</td></tr>"
        "</table>"

        "</body></html>"
    ).arg(
        clientNom,              // %1
        clientAddr,             // %2
        clientEmail,            // %3
        numero,                 // %4
        dateCreation,           // %5
        rowsHtml,               // %6
        style.companyName.isEmpty() ? "Lorem Ipsum Bank" : style.companyName,
        style.companyPhone.isEmpty() ? "0123 4567 89" : style.companyPhone,
        style.companyICE.isEmpty() ? "LOREMIPS" : style.companyICE,
        style.companyPhone.isEmpty() ? "0123 4567 89" : style.companyPhone,
        style.companyName.isEmpty() ? "Lorem Ipsum" : style.companyName,
        "Add your details",
        QString::number(totalHT, 'f', 2),
        QString::number(totalTVA, 'f', 2),
        QString::number(totalTVA > 0 && totalHT > 0 ? (totalTVA/totalHT*100) : 0, 'f', 0) + "%",
        QString::number(totalTTC, 'f', 2),
        "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat.",
        style.companyPhone.isEmpty() ? "000 1234 5678" : style.companyPhone,
        style.companyEmail.isEmpty() ? "your.email@site.com" : style.companyEmail,
        style.companyWebsite.isEmpty() ? "www.website.com" : style.companyWebsite,
               QString::number(elasticHeight)  // %21
    );

    // ============================================
    // GÉNÉRATION PDF AVEC QTEXTDOCUMENT
    // ============================================
    QString outputPath = filePath.isEmpty() ?
        getPdfOutputPath() + "/" + getInvoiceFileName(numero) : filePath;

    QPrinter printer(QPrinter::HighResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(outputPath);
    printer.setPageSize(QPageSize(QPageSize::A4));
    printer.setPageMargins(QMarginsF(0, 0, 0, 0), QPageLayout::Millimeter);
    printer.setPageOrientation(QPageLayout::Portrait);

    QTextDocument doc;
    doc.setHtml(html);
    doc.setPageSize(QSizeF(printer.pageRect(QPrinter::Point).size()));
    doc.setDefaultStyleSheet("* { margin:0; padding:0; }");
    doc.print(&printer);

    qDebug() << "PDF généré:" << outputPath;
    return true;
}
bool InvoiceGenerator::printInvoice(int invoiceId, const InvoiceStyle &style)
{
    QString tmp = getPdfOutputPath() + "/print_temp.pdf";
    return generatePDF(invoiceId, tmp, style);
}

void InvoiceGenerator::drawInvoiceContent(QPainter &p, int id, const QPageSize &ps)
{
    Q_UNUSED(p)
    Q_UNUSED(id)
    Q_UNUSED(ps)
}

void InvoiceGenerator::drawHeader(QPainter &p, int id, int &y)
{
    Q_UNUSED(p)
    Q_UNUSED(id)
    Q_UNUSED(y)
}

void InvoiceGenerator::drawInvoiceDetails(QPainter &p, int id, int &y)
{
    Q_UNUSED(p)
    Q_UNUSED(id)
    Q_UNUSED(y)
}

void InvoiceGenerator::drawItemsTable(QPainter &p, int id, int &y)
{
    Q_UNUSED(p)
    Q_UNUSED(id)
    Q_UNUSED(y)
}

void InvoiceGenerator::drawTotals(QPainter &p, int id, int &y)
{
    Q_UNUSED(p)
    Q_UNUSED(id)
    Q_UNUSED(y)
}

void InvoiceGenerator::drawFooter(QPainter &p, const QPageSize &ps)
{
    Q_UNUSED(p)
    Q_UNUSED(ps)
}