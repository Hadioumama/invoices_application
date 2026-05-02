#include "emailsender.h"
#include <QSslSocket>
#include <QByteArray>
#include <QDebug>
#include <QFile>
#include <QDateTime>
#include <QFileInfo>

EmailSender::EmailSender(QObject *parent) : QObject(parent)
{
    gmailUser     = "hadi.oumama@gmail.com";
    gmailPassword = "lohyyfqczkiktyec";
}

bool EmailSender::sendWelcomeEmail(const QString &to, const QString &name)
{
    QString subject = "Bienvenue sur FacturationApp";
    QString body = "Bonjour " + name + ",\n\n"
                   "Votre compte a été créé avec succès.\n\n"
                   "Cordialement,\nL'équipe FacturationApp";
    return sendSmtp(to, subject, body);
}

bool EmailSender::sendInvoiceEmail(const QString &to,
                                    const QString &clientName,
                                    const QString &invoiceNumber,
                                    const QString &pdfFilePath)
{
    QString subject = QString("Facture %1").arg(invoiceNumber);
    QString body = QString(
        "Bonjour %1,\n\n"
        "Veuillez trouver ci-joint votre facture N° %2.\n\n"
        "Merci pour votre confiance.\n\n"
        "Cordialement,\nFacturationApp"
    ).arg(clientName, invoiceNumber);

    return sendSmtpWithAttachment(to, subject, body, pdfFilePath);
}

bool EmailSender::sendEmailWithAttachment(const QString &to,
                                           const QString &subject,
                                           const QString &body,
                                           const QString &attachmentPath)
{
    return sendSmtpWithAttachment(to, subject, body, attachmentPath);
}

bool EmailSender::sendSmtp(const QString &to,
                            const QString &subject,
                            const QString &body)
{
    QString cleanTo   = to.trimmed();
    QString cleanFrom = gmailUser.trimmed();
    QString cleanPass = gmailPassword.trimmed().remove(" ");

    QSslSocket socket;
    socket.setPeerVerifyMode(QSslSocket::VerifyNone);
    socket.connectToHostEncrypted("smtp.gmail.com", 465);

    if (!socket.waitForEncrypted(10000)) {
        qDebug() << "SSL Error:" << socket.errorString();
        return false;
    }

    auto recv = [&]() -> QByteArray {
        socket.waitForReadyRead(8000);
        QByteArray r = socket.readAll();
        qDebug() << "<<" << r.trimmed();
        return r;
    };
    auto send = [&](const QByteArray &line) {
        qDebug() << ">>" << line.trimmed();
        socket.write(line);
        socket.waitForBytesWritten(5000);
    };

    recv();
    send("EHLO localhost\r\n");
    recv();

    QByteArray auth;
    auth.append(char(0));
    auth.append(cleanFrom.toUtf8());
    auth.append(char(0));
    auth.append(cleanPass.toUtf8());

    send("AUTH PLAIN " + auth.toBase64() + "\r\n");
    QByteArray authResp = recv();
    if (!authResp.contains("235")) {
        qDebug() << "Auth failed!";
        socket.disconnectFromHost();
        return false;
    }

    send("MAIL FROM:<" + cleanFrom.toUtf8() + ">\r\n");
    recv();
    send("RCPT TO:<" + cleanTo.toUtf8() + ">\r\n");
    recv();
    send("DATA\r\n");
    recv();

    QString msg =
        "From: FacturationApp <" + cleanFrom + ">\r\n"
        "To: " + cleanTo + "\r\n"
        "Subject: " + subject + "\r\n"
        "Content-Type: text/plain; charset=UTF-8\r\n"
        "\r\n" + body + "\r\n.\r\n";

    send(msg.toUtf8());
    QByteArray resp = recv();
    if (!resp.contains("250")) {
        socket.disconnectFromHost();
        return false;
    }

    send("QUIT\r\n");
    recv();
    socket.disconnectFromHost();
    return true;
}

bool EmailSender::sendSmtpWithAttachment(const QString &to,
                                          const QString &subject,
                                          const QString &body,
                                          const QString &attachmentPath)
{
    QString cleanTo   = to.trimmed();
    QString cleanFrom = gmailUser.trimmed();
    QString cleanPass = gmailPassword.trimmed().remove(" ");

    QSslSocket socket;
    socket.setPeerVerifyMode(QSslSocket::VerifyNone);
    socket.connectToHostEncrypted("smtp.gmail.com", 465);

    if (!socket.waitForEncrypted(10000)) {
        qDebug() << "SSL Error:" << socket.errorString();
        return false;
    }

    auto recv = [&]() -> QByteArray {
        socket.waitForReadyRead(8000);
        QByteArray r = socket.readAll();
        qDebug() << "<<" << r.trimmed();
        return r;
    };
    auto send = [&](const QByteArray &line) {
        qDebug() << ">>" << line.trimmed();
        socket.write(line);
        socket.waitForBytesWritten(5000);
    };

    recv();
    send("EHLO localhost\r\n");
    recv();

    QByteArray auth;
    auth.append(char(0));
    auth.append(cleanFrom.toUtf8());
    auth.append(char(0));
    auth.append(cleanPass.toUtf8());

    send("AUTH PLAIN " + auth.toBase64() + "\r\n");
    QByteArray authResp = recv();
    if (!authResp.contains("235")) {
        qDebug() << "Auth failed:" << authResp;
        socket.disconnectFromHost();
        return false;
    }

    send("MAIL FROM:<" + cleanFrom.toUtf8() + ">\r\n");
    recv();
    send("RCPT TO:<" + cleanTo.toUtf8() + ">\r\n");
    recv();
    send("DATA\r\n");
    recv();

    // Boundary
    QString boundary = "==Boundary_" +
        QString::number(QDateTime::currentMSecsSinceEpoch());

    // En-têtes
    QString headers =
        "From: FacturationApp <" + cleanFrom + ">\r\n"
        "To: " + cleanTo + "\r\n"
        "Subject: " + subject + "\r\n"
        "MIME-Version: 1.0\r\n"
        "Content-Type: multipart/mixed; boundary=\"" + boundary + "\"\r\n"
        "\r\n";

    // Corps texte
    QString textPart =
        "--" + boundary + "\r\n"
        "Content-Type: text/plain; charset=UTF-8\r\n"
        "Content-Transfer-Encoding: 8bit\r\n"
        "\r\n" + body + "\r\n";

    // Pièce jointe PDF
    QString attachPart;
    QFile f(attachmentPath);
    if (!attachmentPath.isEmpty() && f.open(QIODevice::ReadOnly)) {
        QByteArray pdfData = f.readAll();
        f.close();
        QString fileName = QFileInfo(attachmentPath).fileName();
        attachPart =
            "--" + boundary + "\r\n"
            "Content-Type: application/pdf; name=\"" + fileName + "\"\r\n"
            "Content-Transfer-Encoding: base64\r\n"
            "Content-Disposition: attachment; filename=\"" + fileName + "\"\r\n"
            "\r\n" +
            QString(pdfData.toBase64()) + "\r\n";
    }

    QString endPart = "--" + boundary + "--\r\n";
    QString fullMsg = headers + textPart + attachPart + endPart + ".\r\n";

    send(fullMsg.toUtf8());
    QByteArray resp = recv();
    if (!resp.contains("250")) {
        qDebug() << "Send failed:" << resp;
        socket.disconnectFromHost();
        return false;
    }

    send("QUIT\r\n");
    recv();
    socket.disconnectFromHost();
    qDebug() << "Email envoyé avec succès!";
    return true;
}

QString EmailSender::encodeBase64(const QByteArray &data)
{
    return data.toBase64();
}

QString EmailSender::getMimeType(const QString &filePath)
{
    if (filePath.endsWith(".pdf")) return "application/pdf";
    return "application/octet-stream";
}