#include "utils/invoicegenerator.h"
#include "database/database.h"
#include <QPrinter>
#include <QScrollArea>
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

static QString imageToBase64Html(const QString &path,
                                  int maxW, int maxH)
{
    if (path.isEmpty() || !QFile::exists(path)) return "";
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly)) return "";
    QByteArray data = f.readAll();
    QString ext = QFileInfo(path).suffix().toLower();
    QString mime = (ext == "png") ? "image/png" : "image/jpeg";
    return QString("<img src='data:%1;base64,%2' width='%3' height='%4'"
                   " style='max-width:%3px;max-height:%4px;'/>")
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

    QString pc = style.primaryColor;

    // Logo
 QString logoHtml = imageToBase64Html(style.logoPath, 180, 90);
if (logoHtml.isEmpty())
    logoHtml = QString(
        "<span style='font-size:28px;font-weight:900;color:white;"
        "letter-spacing:2px;'>LOGO</span>");
    if (logoHtml.isEmpty())
        logoHtml = QString("<span style='font-size:22px;font-weight:900;"
                           "color:white;letter-spacing:2px;'>LOGO</span>");

    // Signature
    QString signHtml = imageToBase64Html(style.signaturePath, 130, 45);

    // Lignes articles
    QSqlQuery lq;
    lq.prepare("SELECT designation, quantite, prix_unitaire_ht, taux_tva "
               "FROM lignes_facture WHERE facture_id = ?");
    lq.addBindValue(invoiceId);

    QString rowsHtml;
    bool alt = false;
    if (lq.exec()) {
        while (lq.next()) {
            QString des  = lq.value(0).toString().toHtmlEscaped();
            int     qty  = lq.value(1).toInt();
            double  prix = lq.value(2).toDouble();
            Q_UNUSED(lq.value(3).toDouble());
            double  tot  = qty * prix;
            QString bg   = alt ? "#F5F8FF" : "#FFFFFF";
          rowsHtml += QString(
    "<tr style='background:%1;'>"
    "<td style='padding:7px 10px;border-bottom:1px solid #E8EDF2;"
    "color:#333;'>%2</td>"
    "<td style='padding:7px 10px;border-bottom:1px solid #E8EDF2;"
    "text-align:center;color:#333;'>%3</td>"
    "<td style='padding:7px 10px;border-bottom:1px solid #E8EDF2;"
    "text-align:right;color:#333;'>$%4</td>"
    "<td style='padding:7px 10px;border-bottom:1px solid #E8EDF2;"
    "text-align:right;font-weight:bold;color:#333;'>$%5</td>"
    "</tr>"
).arg(bg, des)
 .arg(qty)
 .arg(QString::number(prix,'f',2))
 .arg(QString::number(tot,'f',2));
            alt = !alt;
        }
    }

    // Statut couleur
    QString sBg="#FFF3CD", sCol="#856404";
    if (statut=="Payée")   { sBg="#D4EDDA"; sCol="#155724"; }
    if (statut=="Envoyée") { sBg="#CCE5FF"; sCol="#004085"; }
    if (statut=="Annulée") { sBg="#F8D7DA"; sCol="#721C24"; }

    // HTML complet — tableaux imbriqués compatibles Qt
QString html;
html = "<!DOCTYPE html><html><head><meta charset='UTF-8'></head>"
       "<body style='margin:0;padding:0;font-family:Arial,sans-serif;"
       "font-size:10px;color:#333;background:white;'>";

// ── HEADER FONCÉ ──────────────────────────────────────────
html += QString(
    "<table width='600' cellpadding='0' cellspacing='0' border='0' "
    "align='center' style='background:#1B2A3B;'>"
    "<tr>"
    "<td style='padding:16px 20px;vertical-align:middle;width:55%%;'>"
    "<table cellpadding='0' cellspacing='6' border='0'><tr>"
    "<td style='vertical-align:middle;'>%1</td>"
    "<td style='vertical-align:middle;'>"
    "<span style='font-size:22px;font-weight:900;color:white;"
    "letter-spacing:2px;text-transform:uppercase;'>%2</span>"
    "</td>"
    "</tr></table>"
    "</td>"
    "<td style='padding:16px 20px;vertical-align:middle;"
    "text-align:right;width:45%%;'>"
    "<span style='font-size:10px;color:rgba(255,255,255,0.6);'>"
    "%3</span>"
    "</td>"
    "</tr></table>"
).arg(logoHtml,
      style.companyName.toHtmlEscaped(),
      style.companyWebsite.toHtmlEscaped());

// ── INFOS ENTREPRISE + INVOICE ────────────────────────────
html += QString(
    "<table width='600' cellpadding='0' cellspacing='0' border='0' "
    "align='center' style='margin:16px 0 8px 0;'>"
    "<tr>"
    "<td width='320' style='padding:0 20px;vertical-align:top;'>"
    "<p style='font-size:15px;font-weight:bold;color:#1B2A3B;"
    "margin:0 0 4px;'>&lt;Company Name&gt;</p>"
    "<p style='font-size:9px;color:#666;margin:0;line-height:1.9;'>"
    "%1<br>%2, %3<br>%4</p>"
    "</td>"
    "<td width='280' style='padding:0 20px;vertical-align:top;"
    "text-align:right;'>"
    "<p style='font-size:26px;font-weight:900;color:#1B2A3B;"
    "margin:0;letter-spacing:1px;'>INVOICE</p>"
    "<table cellpadding='3' cellspacing='0' border='0' "
    "width='100%%' align='right' style='margin-top:4px;'>"
    "<tr>"
    "<td style='color:#888;font-size:9px;text-align:left;width:60%%;'>"
    "Invoice No:</td>"
    "<td style='font-size:9px;text-align:right;color:#333;'>"
    "%5</td>"
    "</tr>"
    "<tr>"
    "<td style='color:#888;font-size:9px;text-align:left;'>"
    "Invoice Date:</td>"
    "<td style='font-size:9px;text-align:right;color:%6;"
    "font-weight:bold;'>%7</td>"
    "</tr>"
    "<tr>"
    "<td style='color:#888;font-size:9px;text-align:left;'>"
    "Due Date:</td>"
    "<td style='font-size:9px;text-align:right;color:#E53E3E;"
    "font-weight:bold;'>%8</td>"
    "</tr>"
    "</table>"
    "</td>"
    "</tr></table>"
).arg(style.companyAddress.toHtmlEscaped(),
      style.companyEmail.toHtmlEscaped(),
      style.companyWebsite.toHtmlEscaped(),
      style.companyPhone.toHtmlEscaped(),
      numero, pc,
      dateCreation, dateEcheance);

// ── BILL TO + LOCATION + STATUT ───────────────────────────
html += QString(
    "<table width='600' cellpadding='0' cellspacing='0' border='0' "
    "align='center' style='margin:10px 0;'>"
    "<tr>"
    // BILL TO
    "<td width='200' style='padding:0 20px;vertical-align:top;'>"
    "<p style='font-size:8px;font-weight:bold;color:#888;"
    "text-transform:uppercase;letter-spacing:1px;margin:0 0 5px;'>"
    "BILL TO</p>"
    "<p style='font-size:10px;font-weight:bold;color:#1B2A3B;"
    "margin:0 0 3px;'>%1</p>"
    "<p style='font-size:9px;color:#666;margin:0;line-height:1.7;'>"
    "%2<br>%3<br>%4</p>"
    "</td>"
    // LOCATION
    "<td width='200' style='padding:0 20px;vertical-align:top;'>"
    "<p style='font-size:8px;font-weight:bold;color:#888;"
    "text-transform:uppercase;letter-spacing:1px;margin:0 0 5px;'>"
    "LOCATION</p>"
    "<p style='font-size:9px;color:#666;margin:0;line-height:1.7;'>"
    "&lt;Name&gt;<br>&lt;Address&gt;<br>&lt;Phone&gt;</p>"
    "</td>"
    // STATUT
    "<td width='200' style='padding:0 20px;vertical-align:top;"
    "text-align:right;'>"
    "<span style='background:%5;color:%6;padding:3px 10px;"
    "border-radius:6px;font-size:8px;font-weight:bold;'>%7</span>"
    "</td>"
    "</tr></table>"
).arg(clientNom.toHtmlEscaped(),
      clientAddr.toHtmlEscaped(),
      clientTel.toHtmlEscaped(),
      clientEmail.toHtmlEscaped(),
      sBg, sCol, statut);

// ── SÉPARATEUR ────────────────────────────────────────────
html += "<table width='600' cellpadding='0' cellspacing='0' border='0' "
        "align='center' style='margin:10px 0 0 0;'>"
        "<tr><td height='1' bgcolor='#E2E8F0'></td></tr></table>";

// ── TABLEAU ARTICLES ──────────────────────────────────────
html += QString(
    "<table width='600' cellpadding='0' cellspacing='0' border='0' "
    "align='center' style='border-collapse:collapse;'>"
    "<thead>"
    "<tr style='background:%1;'>"
    "<th style='padding:9px 14px;color:white;font-size:9px;"
    "font-weight:bold;text-align:left;text-transform:uppercase;"
    "letter-spacing:0.8px;width:270px;'>DESCRIPTION</th>"
    "<th style='padding:9px 14px;color:white;font-size:9px;"
    "font-weight:bold;text-align:center;text-transform:uppercase;"
    "letter-spacing:0.8px;width:55px;'>QTY</th>"
    "<th style='padding:9px 14px;color:white;font-size:9px;"
    "font-weight:bold;text-align:right;text-transform:uppercase;"
    "letter-spacing:0.8px;width:140px;'>UNIT PRICE</th>"
    "<th style='padding:9px 14px;color:white;font-size:9px;"
    "font-weight:bold;text-align:right;text-transform:uppercase;"
    "letter-spacing:0.8px;width:120px;'>TOTAL</th>"
    "</tr>"
    "</thead>"
    "<tbody>%2</tbody>"
    // Lignes vides
    "<tr><td colspan='4' height='8' "
    "style='border-bottom:1px solid #EDF2F7;'></td></tr>"
    "<tr><td colspan='4' height='8' "
    "style='border-bottom:1px solid #EDF2F7;'></td></tr>"
    "<tr><td colspan='4' height='8' "
    "style='border-bottom:1px solid #EDF2F7;'></td></tr>"
    "</table>"
).arg(pc, rowsHtml);

// ── MERCI ─────────────────────────────────────────────────
html += QString(
    "<table width='600' cellpadding='0' cellspacing='0' border='0' "
    "align='center' style='margin:12px 0 6px 0;'>"
    "<tr><td style='padding:0 20px;'>"
    "<span style='font-size:11px;font-weight:bold;color:%1;font-style:italic;'>"
    "Thank you for your business!</span>"
    "</td></tr></table>"
).arg(pc);

// ── TOTAUX + TERMS ────────────────────────────────────────
html += QString(
    "<table width='600' cellpadding='0' cellspacing='0' border='0' "
    "align='center'>"
    "<tr>"
    // Totaux DROITE
    "<td width='280' style='padding:0 0 0 0;vertical-align:top;'>"
    "</td>"
    "<td width='320' style='padding:0 20px 0 0;vertical-align:top;'>"
    "<table width='100%%' cellpadding='0' cellspacing='0' border='0'>"
    "<tr style='border-bottom:1px solid #EDF2F7;'>"
    "<td style='color:#666;font-size:9px;padding:5px 8px;"
    "text-align:left;'>SUBTOTAL</td>"
    "<td style='text-align:right;color:#333;font-size:9px;"
    "padding:5px 8px;'>$ %1</td></tr>"
    "<tr style='border-bottom:1px solid #EDF2F7;'>"
    "<td style='color:#666;font-size:9px;padding:5px 8px;'>DISCOUNT</td>"
    "<td style='text-align:right;color:#333;font-size:9px;"
    "padding:5px 8px;'>$ 0.00</td></tr>"
    "<tr style='border-bottom:1px solid #EDF2F7;'>"
    "<td style='color:#666;font-size:9px;padding:5px 8px;'>"
    "SUBTOTAL LESS DISCOUNT</td>"
    "<td style='text-align:right;color:#333;font-size:9px;"
    "padding:5px 8px;'>$ %2</td></tr>"
    "<tr style='border-bottom:1px solid #EDF2F7;'>"
    "<td style='color:#666;font-size:9px;padding:5px 8px;'>TAX RATE</td>"
    "<td style='text-align:right;color:#333;font-size:9px;"
    "padding:5px 8px;'>20%%</td></tr>"
    "<tr style='border-bottom:1px solid #EDF2F7;'>"
    "<td style='color:#666;font-size:9px;padding:5px 8px;'>TOTAL TAX</td>"
    "<td style='text-align:right;color:#333;font-size:9px;"
    "padding:5px 8px;'>$ %3</td></tr>"
    "<tr style='background:%4;'>"
    "<td style='color:white;font-size:12px;font-weight:bold;"
    "padding:11px 8px;'>Balance Due</td>"
    "<td style='color:white;font-size:14px;font-weight:bold;"
    "text-align:right;padding:11px 8px;'>$ %5</td>"
    "</tr></table>"
    "</td>"
    "</tr></table>"
).arg(QString::number(totalHT,  'f', 2),
      QString::number(totalHT,  'f', 2),
      QString::number(totalTVA, 'f', 2),
      pc,
      QString::number(totalTTC, 'f', 2));

// ── TERMS ─────────────────────────────────────────────────
html += QString(
    "<table width='600' cellpadding='0' cellspacing='0' border='0' "
    "align='center' style='margin:14px 0 0 0;'>"
    "<tr><td style='padding:0 20px;'>"
    "<p style='font-size:9px;font-weight:bold;color:#333;margin:0 0 4px;'>"
    "Terms &amp; Instructions</p>"
    "<p style='font-size:8px;color:#888;margin:0;line-height:1.7;'>"
    "&lt;Add payment instructions here, e.g. bank, paypal...&gt;<br>"
    "&lt;Add terms here, e.g. warranty, returns policy...&gt;<br>"
    "Contact: %1</p>"
    "</td></tr></table>"
).arg(style.companyEmail.toHtmlEscaped());

// ── FOOTER FONCÉ ──────────────────────────────────────────
html += QString(
    "<table width='100%' cellpadding='0' cellspacing='0' border='0' "
    "align='center' style='background:#1B2A3B;margin-top:16px;'>"
    "<tr>"
    "<td width='50%' style='padding:14px 40px;vertical-align:middle;'>"
    "<p style='font-size:9px;color:rgb(255, 255, 255);"
    "margin:0 0 3px;'>&#128222; %1</p>"
    "<p style='font-size:9px;color:rgb(255, 255, 255);"
    "margin:0 0 3px;'>&#127760; %2</p>"
    "<p style='font-size:9px;color:rgb(255, 255, 255);"
    "margin:0 0 3px;'>&#9993; %3</p>"
    "<p style='font-size:9px;color:rgb(255, 255, 255);"
    "margin:0;'>&#128205; %4</p>"
    "</td>"
    "<td width='50%' style='padding:14px 40px;vertical-align:middle;"
    "text-align:right;'>"
    "%5"
    "<br>"
    "<span style='display:inline-block;"
    "border-top:1px dashed rgb(255, 255, 255);"
    "width:160px;padding-top:5px;'>"
    "<span style='font-size:9px;color:rgb(255, 255, 255);'>"
    "Authorized Signature</span>"
    "</span>"
    "</td>"
    "</tr></table>"
).arg(style.companyPhone.toHtmlEscaped(),
      style.companyWebsite.toHtmlEscaped(),
      style.companyEmail.toHtmlEscaped(),
      style.companyAddress.toHtmlEscaped(),
      signHtml);

html += "</body></html>";

    // Générer PDF
    QString outputPath = filePath.isEmpty() ?
        getPdfOutputPath() + "/" + getInvoiceFileName(numero) : filePath;

    QPrinter printer(QPrinter::HighResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(outputPath);
    printer.setPageSize(QPageSize(QPageSize::A4));
  printer.setPageMargins(
    QMarginsF(10, 10, 10, 10), QPageLayout::Millimeter);
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

void InvoiceGenerator::drawInvoiceContent(QPainter &p,int id,const QPageSize &ps)
{Q_UNUSED(p)Q_UNUSED(id)Q_UNUSED(ps)}
void InvoiceGenerator::drawHeader(QPainter &p,int id,int &y)
{Q_UNUSED(p)Q_UNUSED(id)Q_UNUSED(y)}
void InvoiceGenerator::drawInvoiceDetails(QPainter &p,int id,int &y)
{Q_UNUSED(p)Q_UNUSED(id)Q_UNUSED(y)}
void InvoiceGenerator::drawItemsTable(QPainter &p,int id,int &y)
{Q_UNUSED(p)Q_UNUSED(id)Q_UNUSED(y)}
void InvoiceGenerator::drawTotals(QPainter &p,int id,int &y)
{Q_UNUSED(p)Q_UNUSED(id)Q_UNUSED(y)}
void InvoiceGenerator::drawFooter(QPainter &p,const QPageSize &ps)
{Q_UNUSED(p)Q_UNUSED(ps)}