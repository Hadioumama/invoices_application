#ifndef ENTREPRISE_CONFIG_WIDGET_H
#define ENTREPRISE_CONFIG_WIDGET_H

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include "models/entreprise_config.h"

class EntrepriseConfigWidget : public QWidget
{
    Q_OBJECT
public:
    explicit EntrepriseConfigWidget(QWidget *parent = nullptr);
    EntrepriseConfig getConfig() const;
    void setConfig(const EntrepriseConfig &cfg);

signals:
    void configSaved(const EntrepriseConfig &cfg);
    void backToDashboard();

private slots:
    void pickLogo();
    void pickSignature();
    void pickColor();
    void saveConfig();

private:
    void setupUI();
    void styleFileBtn(QPushButton *btn);

    QLineEdit *nomEdit;
    QLineEdit *ribEdit;
    QLabel *logoPreview, *signaturePreview, *colorPreview;
    QPushButton *logoBtn, *signatureBtn, *colorBtn, *saveBtn, *cancelBtn;

    QString currentLogoPath;
    QString currentSignaturePath;
    QColor currentThemeColor;
};

#endif