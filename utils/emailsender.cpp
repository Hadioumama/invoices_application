#include "emailsender.h"
#include <QSslSocket>
#include <QByteArray>
#include <QDebug>

EmailSender::EmailSender(QObject *parent) : QObject(parent)
{
    gmailUser     = "hadi.oumama@gmail.com";  // ← ton email
    gmailPassword = "lohyyfqczkiktyec";        // ← ton mot de passe app Gmail
}

bool EmailSender::sendWelcomeEmail(const QString &to, const QString &name)
{
    QString subject = "Bienvenue sur FacturationApp";
    QString body = "Bonjour " + name + ",\n\n"
                   "Nous vous confirmons la création réussie de votre compte sur FacturationApp.\n\n"
                   "Vous pouvez dès à présent vous connecter et profiter de nos services de facturation.\n\n"
                   "Pour des raisons de sécurité, nous vous recommandons de ne jamais partager vos informations de connexion.\n\n"
                   "Si vous n’êtes pas à l’origine de cette inscription, veuillez contacter notre support immédiatement.\n\n"
                   "Cordialement,\n"
                   "L’équipe FacturationApp";
    return sendSmtp(to.trimmed(), subject, body);
}
bool EmailSender::sendSmtp(const QString &to,
                           const QString &subject,
                           const QString &body)
{
    QString cleanTo   = to.trimmed();
    QString cleanFrom = gmailUser.trimmed();

    QSslSocket socket;
    socket.connectToHostEncrypted("smtp.gmail.com", 465);

    if (!socket.waitForEncrypted(5000)) {
        qDebug() << "Erreur SSL:" << socket.errorString();
        return false;
    }
    qDebug() << "Connecte a Gmail SMTP";

    auto readLine = [&]() {
        socket.waitForReadyRead(5000);
        QByteArray resp = socket.readAll();
        qDebug() << "<<" << resp.trimmed();
        return resp;
    };

    auto sendLine = [&](const QByteArray &line) {
        qDebug() << ">>" << line.trimmed();
        socket.write(line);
        socket.waitForBytesWritten(3000);
    };

    readLine();

    sendLine("EHLO localhost\r\n");
    readLine();

    QString cleanPassword = gmailPassword.remove(" ");
    QByteArray auth;
    auth.append(char(0));
    auth.append(cleanFrom.toUtf8());
    auth.append(char(0));
    auth.append(cleanPassword.toUtf8());

    sendLine("AUTH PLAIN " + auth.toBase64() + "\r\n");
    QByteArray authResp = readLine();
    if (!authResp.contains("235")) {
        qDebug() << "Echec authentification Gmail !";
        return false;
    }

    sendLine("MAIL FROM:<" + cleanFrom.toUtf8() + ">\r\n");
    readLine();

    sendLine("RCPT TO:<" + cleanTo.toUtf8() + ">\r\n");
    readLine();

    sendLine("DATA\r\n");
    readLine();

    QString message =
        "From: FacturationApp <" + cleanFrom + ">\r\n"
                                               "To: " + cleanTo + "\r\n"
                    "Subject: " + subject + "\r\n"
                    "Content-Type: text/plain; charset=UTF-8\r\n"
                    "\r\n" + body + "\r\n.\r\n";

    sendLine(message.toUtf8());
    QByteArray dataResp = readLine();
    if (!dataResp.contains("250")) {
        qDebug() << "Erreur envoi message";
        return false;
    }

    sendLine("QUIT\r\n");
    readLine();

    qDebug() << "Email envoye avec succes !";
    return true;
}