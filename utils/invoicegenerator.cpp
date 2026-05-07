#include "utils/invoicegenerator.h"
#include "database/database.h"
#include <QPrinter>
#include <QPicture>
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

    QImage img(path);
    int w = img.width();
    int h = img.height();
    
    if (w > maxW || h > maxH) {
        qreal ratio = qMin((qreal)maxW / w, (qreal)maxH / h);
        w = qRound(w * ratio);
        h = qRound(h * ratio);
    }

    return QString("<img src='data:%1;base64,%2' width='%3' height='%4'/>")
           .arg(mime, QString(data.toBase64()))
           .arg(w).arg(h);
}

bool InvoiceGenerator::generatePDF(int invoiceId,
                                    const QString &filePath,
                                    const InvoiceStyle &style)
{
    // Données facture
        QSqlQuery q;
    q.prepare("SELECT numero, type, client_nom, client_adresse, "
              "client_tel, client_email, date_creation, date_echeance, "
              "total_ht, total_tva, total_ttc, statut, "
              "logo_path, signature_path "
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
    
    // Récupérer les chemins depuis la base de données
    QString dbLogoPath      = q.value(12).toString();
    QString dbSignaturePath = q.value(13).toString();

    // Fusionner avec le style passé en paramètre (priorité au style personnalisé, sinon DB)
    InvoiceStyle finalStyle = style;
    
    if (finalStyle.logoPath.isEmpty() && !dbLogoPath.isEmpty()) {
        finalStyle.logoPath = dbLogoPath;
    }
    if (finalStyle.signaturePath.isEmpty() && !dbSignaturePath.isEmpty()) {
        finalStyle.signaturePath = dbSignaturePath;
    }

    QString primaryColor = finalStyle.primaryColor.isEmpty() ? "#4db8e8" : finalStyle.primaryColor;
    qDebug() << "Logo:" << finalStyle.logoPath << "Signature:" << finalStyle.signaturePath;
    qDebug() << "primaryColor:" << style.primaryColor << "→ utilisée:" << primaryColor;

       QString logoHtml = imageToBase64Html(finalStyle.logoPath, 130, 65);
    if (logoHtml.isEmpty())
        logoHtml = QString("<span style='font-size:16px;font-weight:900;color:white;letter-spacing:2px;'>LOGO HERE</span>");

    // Signature en base64 si fournie
    QString signatureHtml = imageToBase64Html(finalStyle.signaturePath, 90, 45);
    if (!signatureHtml.isEmpty()) {
        signatureHtml = QString("<div style='text-align:right;margin-bottom:2px;'>%1</div>").arg(signatureHtml);
    }

    // Lignes articles
    QSqlQuery lq;
    lq.prepare("SELECT designation, quantite, prix_unitaire_ht, taux_tva "
               "FROM lignes_facture WHERE facture_id = ?");
    lq.addBindValue(invoiceId);

    QString rowsHtml;
    int rowNum = 1;
    int nombreLignes = 0;
    if (lq.exec()) {
        while (lq.next()) {
            nombreLignes++;
            QString des  = lq.value(0).toString().toHtmlEscaped();
            int     qty  = lq.value(1).toInt();
            double  prix = lq.value(2).toDouble();
            Q_UNUSED(lq.value(3).toDouble());
            double  tot  = qty * prix;
            
            rowsHtml += QString(
                "<tr style='border-bottom:1px solid #d0d0d0;'>"
                "<td style='padding:7px 10px; font-size:10px; color:#333; border-right:1px solid #e0e0e0;'>%1. %2</td>"
                "<td style='padding:7px 10px; font-size:10px; color:#555; text-align:center; border-right:1px solid #e0e0e0;'>%3</td>"
                "<td style='padding:7px 10px; font-size:10px; color:#555; text-align:center; border-right:1px solid #e0e0e0;'>$%4</td>"
                "<td style='padding:7px 10px; font-size:10px; color:#555; text-align:center; font-weight:bold;'>$%5</td>"
                "</tr>"
            ).arg(rowNum).arg(des).arg(qty)
             .arg(QString::number(prix,'f',2))
             .arg(QString::number(tot,'f',2));
            
            rowNum++;
        }
    }

    // ============================================
    // LIGNES VIDES AVEC BORDURES GRIS CLAIR
    // ============================================
    int lignesVides = 11 - nombreLignes;
    if (lignesVides < 6) lignesVides = 6;
    
    for (int i = 0; i < lignesVides; i++) {
        rowsHtml += QString(
            "<tr style='border-bottom:1px solid #d0d0d0;'>"
            "<td style='padding:7px 10px; font-size:10px; color:#ccc; border-right:1px solid #e0e0e0;'>-</td>"
            "<td style='padding:7px 10px; font-size:10px; color:#ccc; text-align:center; border-right:1px solid #e0e0e0;'>-</td>"
            "<td style='padding:7px 10px; font-size:10px; color:#ccc; text-align:center; border-right:1px solid #e0e0e0;'>-</td>"
            "<td style='padding:7px 10px; font-size:10px; color:#ccc; text-align:center;'>-</td>"
            "</tr>"
        );
    }

    // ============================================
    // HTML COMPLET - BORDURES DE TABLEAU + DÉCALAGE BAS
    // ============================================
    QString html = QString(
        "<!DOCTYPE html>"
        "<html>"
        "<head><meta charset='UTF-8'></head>"
        "<body style='margin:0; padding:0; font-family:Arial,Helvetica,sans-serif; background:white; line-height:1.3;'>"

        "<table width='100%' cellpadding='0' cellspacing='0' style='background:white; width:100%;'>"

        // HEADER
        "<tr>"
        "<td style='background-color:%22; padding:12px 30px;'>"
        "<table width='100%' cellpadding='0' cellspacing='0'>"
        "<tr>"
        "<td style='width:50%;'>%21</td>"
        "<td style='width:50%; text-align:right;'>"
        "<span style='font-size:20px; font-weight:bold; letter-spacing:2px; color:white;'>INVOICE</span>"
        "</td>"
        "</tr>"
        "</table>"
        "</td>"
        "</tr>"

        "<tr><td height='10'></td></tr>"

        // INFOS CLIENT
        "<tr>"
        "<td style='padding:0 30px;'>"
        "<table width='100%' cellpadding='0' cellspacing='0'>"
        "<tr>"
        "<td style='width:60%;'>"
        "<p style='color:%22; font-size:10px; font-weight:bold; margin:0 0 3px 0; text-transform:uppercase;'>Invoice to:</p>"
        "<p style='color:#666; font-size:9px; line-height:1.4; margin:0;'>"
        "<b>Name:</b> %1<br>%2<br>%3"
        "</p>"
        "</td>"
        "<td style='width:40%; text-align:right;'>"
        "<p style='margin:0; color:#666; font-size:9px;'>Invoice No: <b style='color:#333;'>%4</b></p>"
        "<p style='margin:0; color:#666; font-size:9px;'>Date: <b style='color:#333;'>%5</b></p>"
        "</td>"
        "</tr>"
        "</table>"
        "</td>"
        "</tr>"

        "<tr><td height='12'></td></tr>"

        // TABLEAU ARTICLES AVEC BORDURES COMPLÈTES
        "<tr>"
        "<td style='padding:0 30px;'>"
        "<table width='100%' cellpadding='0' cellspacing='0' style='border:1px solid #d0d0d0; border-collapse:collapse;'>"
        "<tr style='background-color:%22;'>"
        "<td style='padding:7px 10px; color:white; font-size:10px; font-weight:bold; text-transform:uppercase; width:50%; border-right:1px solid rgba(255,255,255,0.3);'>Description</td>"
        "<td style='padding:7px 10px; color:white; font-size:10px; font-weight:bold; text-transform:uppercase; text-align:center; width:16%; border-right:1px solid rgba(255,255,255,0.3);'>Qty</td>"
        "<td style='padding:7px 10px; color:white; font-size:10px; font-weight:bold; text-transform:uppercase; text-align:center; width:17%; border-right:1px solid rgba(255,255,255,0.3);'>Price</td>"
        "<td style='padding:7px 10px; color:white; font-size:10px; font-weight:bold; text-transform:uppercase; text-align:center; width:17%;'>Total</td>"
        "</tr>"
        "%6"
        "</table>"
        "</td>"
        "</tr>"

        "<tr><td height='18'></td></tr>"

        // TOTALS SECTION
        "<tr>"
        "<td style='padding:0 30px;'>"
        "<table width='100%' cellpadding='0' cellspacing='0'>"
        "<tr>"
        "<td style='width:50%; vertical-align:top;'>"
        "<p style='color:%22; font-size:9px; font-weight:bold; margin:0 0 3px 0; text-transform:uppercase;'>Bank Info:</p>"
        "<p style='font-size:8px; color:#666; line-height:1.3; margin:0;'>"
        "Bank Name: %7<br>"
        "Bank Account: %8<br>"
        "Code: %9"
        "</p>"
        "<p style='color:%22; font-size:9px; font-weight:bold; margin:6px 0 3px 0; text-transform:uppercase;'>Payment Info:</p>"
        "<p style='font-size:8px; color:#666; line-height:1.3; margin:0;'>"
        "Account: %10<br>"
        "A/C Name: %11<br>"
        "Bank Details: %12"
        "</p>"
        "</td>"
        "<td style='width:50%; text-align:right; vertical-align:top;'>"
        "<table width='220' cellpadding='0' cellspacing='0' style='background:#e8f4fc;' align='right'>"
        "<tr>"
        "<td style='padding:4px 10px; color:#666; font-size:9px; text-align:right;'>Sub Total:</td>"
        "<td style='padding:4px 10px; color:#333; font-size:9px; font-weight:bold; text-align:right;'>$%13</td>"
        "</tr>"
        "<tr>"
        "<td style='padding:4px 10px; color:#666; font-size:9px; text-align:right;'>Tax:</td>"
        "<td style='padding:4px 10px; color:#333; font-size:9px; font-weight:bold; text-align:right;'>%14</td>"
        "</tr>"
        "<tr>"
        "<td style='padding:4px 10px; color:#666; font-size:9px; text-align:right;'>Tax Rate:</td>"
        "<td style='padding:4px 10px; color:#333; font-size:9px; font-weight:bold; text-align:right;'>%15</td>"
        "</tr>"
        "<tr><td colspan='2' style='border-top:2px solid %22; height:3px;'></td></tr>"
        "<tr>"
        "<td style='padding:5px 10px; color:%22; font-size:11px; font-weight:bold; text-align:right;'>TOTAL:</td>"
        "<td style='padding:5px 10px; color:%22; font-size:11px; font-weight:bold; text-align:right;'>$%16</td>"
        "</tr>"
        "</table>"
        "</td>"
        "</tr>"
        "</table>"
        "</td>"
        "</tr>"

        "<tr><td height='22'></td></tr>"

        // TERMS & SIGNATURE
        "<tr>"
        "<td style='padding:0 30px 18px 30px;'>"
        "<table width='100%' cellpadding='0' cellspacing='0'>"
        "<tr>"
        "<td style='width:60%; vertical-align:bottom;'>"
        "<p style='color:%22; font-size:9px; font-weight:bold; margin:0 0 3px 0; text-transform:uppercase;'>Terms & Conditions:</p>"
        "<p style='font-size:7px; color:#999; line-height:1.3; margin:0;'>"
        "%17"
        "</p>"
        "</td>"
        "<td style='width:40%; text-align:right; vertical-align:bottom;'>"
        "%23"
        "</td>"
        "</tr>"
        "</table>"
        "</td>"
        "</tr>"

        // FOOTER
        "<tr>"
  "<td style='background-color:%22; padding:14px 30px;'>"
        "<table width='100%' cellpadding='0' cellspacing='0'>"
        "<tr>"
        "<td style='width:33%; font-size:8px; color:white; text-align:left;'>"
        "<span style='display:inline-block; width:14px; height:14px; background:rgba(255,255,255,0.3); border-radius:50%; text-align:center; line-height:14px; margin-right:5px; font-size:9px;'>&#9742;</span>"
        "%18"
        "</td>"
        "<td style='width:33%; font-size:8px; color:white; text-align:center;'>"
        "<span style='display:inline-block; width:14px; height:14px; background:rgba(255,255,255,0.3); border-radius:50%; text-align:center; line-height:14px; margin-right:5px; font-size:9px;'>&#9993;</span>"
        "%19"
        "</td>"
        "<td style='width:33%; font-size:8px; color:white; text-align:right;'>"
        "<span style='display:inline-block; width:14px; height:14px; background:rgba(255,255,255,0.3); border-radius:50%; text-align:center; line-height:14px; margin-right:5px; font-size:9px;'>&#127760;</span>"
        "%20"
        "</td>"
        "</tr>"
        "</table>"
        "</td>"
        "</tr>"

        "</table>"

        "</body></html>"
        ).arg(
        clientNom,
        clientAddr,
        clientEmail,
        numero,
        dateCreation,
        rowsHtml,
        finalStyle.companyName.isEmpty() ? "Lorem Ipsum Bank" : finalStyle.companyName,      // ← style → finalStyle
        finalStyle.companyPhone.isEmpty() ? "0123 4567 89" : finalStyle.companyPhone,        // ← style → finalStyle
        finalStyle.companyICE.isEmpty() ? "LOREMIPS" : finalStyle.companyICE,                // ← style → finalStyle
        finalStyle.companyPhone.isEmpty() ? "0123 4567 89" : finalStyle.companyPhone,        // ← style → finalStyle
        finalStyle.companyName.isEmpty() ? "Lorem Ipsum" : finalStyle.companyName,           // ← style → finalStyle
        "Add your details",
        QString::number(totalHT, 'f', 2),
        QString::number(totalTVA, 'f', 2),
        QString::number(totalTVA > 0 && totalHT > 0 ? (totalTVA/totalHT*100) : 0, 'f', 0) + "%",
        QString::number(totalTTC, 'f', 2),
        "Lorem ipsum dolor sit amet, consectetur adipiscing elit.",
        finalStyle.companyPhone.isEmpty() ? "000 1234 5678" : finalStyle.companyPhone,       // ← style → finalStyle
        finalStyle.companyEmail.isEmpty() ? "your.email@site.com" : finalStyle.companyEmail, // ← style → finalStyle
        finalStyle.companyWebsite.isEmpty() ? "www.website.com" : finalStyle.companyWebsite, // ← style → finalStyle
        logoHtml,
        primaryColor,
        signatureHtml
    );

    // Génération PDF - UNE SEULE PAGE A4
    QString outputPath = filePath.isEmpty() ?
        getPdfOutputPath() + "/" + getInvoiceFileName(numero) : filePath;

    QPrinter printer(QPrinter::HighResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(outputPath);
    printer.setPageSize(QPageSize(QPageSize::A4));
    printer.setPageMargins(QMarginsF(10, 10, 10, 10), QPageLayout::Millimeter);
    printer.setPageOrientation(QPageLayout::Portrait);

    QTextDocument doc;
    doc.setHtml(html);
    
    QRectF contentRect = printer.pageRect(QPrinter::Point);
    QSizeF pageSize(contentRect.width(), contentRect.height());
    doc.setPageSize(pageSize);
    doc.setDefaultStyleSheet("* { margin:0; padding:0; }");
    
    doc.print(&printer);

    qDebug() << "PDF généré:" << outputPath;
    return true;
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