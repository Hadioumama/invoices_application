#ifndef EMAILSENDER_H
#define EMAILSENDER_H

#include <QObject>
#include <QSslSocket>

class EmailSender : public QObject
{
    Q_OBJECT
public:
    explicit EmailSender(QObject *parent = nullptr);
    bool sendWelcomeEmail(const QString &to, const QString &name);
     // Envoyer facture par email avec PDF en pièce jointe
    bool sendInvoiceEmail(const QString &to, 
                         const QString &clientName,
                         const QString &invoiceNumber,
                         const QString &pdfFilePath);
    
    // Envoyer email générique avec pièce jointe
    bool sendEmailWithAttachment(const QString &to,
                                const QString &subject,
                                const QString &body,
                                const QString &attachmentPath);
private:
 bool sendSmtpWithAttachment(const QString &to,
                               const QString &subject,
                               const QString &body,
                               const QString &attachmentPath);
    bool sendSmtp(const QString &to,
                  const QString &subject,
                  const QString &body);
     QString encodeBase64(const QByteArray &data);
    QString getMimeType(const QString &filePath);
    QString gmailUser;
    QString gmailPassword;
};

#endif
