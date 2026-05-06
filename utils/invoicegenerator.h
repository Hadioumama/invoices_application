#ifndef INVOICEGENERATOR_H
#define INVOICEGENERATOR_H

#include <QString>
#include <QPainter>
#include <QPageSize>

struct InvoiceStyle {
    QString primaryColor   = "#0099CC";
    QString secondaryColor = "#FFFFFF";
    QString logoPath       = "";
    QString signaturePath  = "";
    QString companyName    = "VOTRE ENTREPRISE";
    QString companyAddress = "123 Rue Al Hoceima, Maroc";
    QString companyPhone   = "+212 5XX XXX XXX";
    QString companyEmail   = "contact@entreprise.com";
    QString companyWebsite = "www.entreprise.com";
    QString companyICE     = "000000000000000";
    QString terms;
    
};

class InvoiceGenerator
{
public:
    InvoiceGenerator();
    bool generatePDF(int invoiceId,
                     const QString &filePath = "",
                     const InvoiceStyle &style = InvoiceStyle());
    bool printInvoice(int invoiceId,
                      const InvoiceStyle &style = InvoiceStyle());
    static QString getPdfOutputPath();
    static QString getInvoiceFileName(const QString &numeroFacture);

private:
    void drawInvoiceContent(QPainter &painter, int invoiceId,
                            const QPageSize &pageSize);
    void drawHeader(QPainter &painter, int invoiceId, int &currentY);
    void drawInvoiceDetails(QPainter &painter, int invoiceId, int &currentY);
    void drawItemsTable(QPainter &painter, int invoiceId, int &currentY);
    void drawTotals(QPainter &painter, int invoiceId, int &currentY);
    void drawFooter(QPainter &painter, const QPageSize &pageSize);
};

#endif