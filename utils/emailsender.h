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

private:
    bool sendSmtp(const QString &to,
                  const QString &subject,
                  const QString &body);
    QString gmailUser;
    QString gmailPassword;
};

#endif
