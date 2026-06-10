#ifndef ENTREPRISE_CONFIG_MANAGER_H
#define ENTREPRISE_CONFIG_MANAGER_H

#include <QObject>
#include <QSettings>
#include "models/entreprise_config.h"

class EntrepriseConfigManager : public QObject {
    Q_OBJECT
public:
    static EntrepriseConfigManager* instance();
    
    EntrepriseConfig loadConfig();
    void saveConfig(const EntrepriseConfig &cfg);
    bool isConfigured() const;
    
    QString getLogoPath() const;
    QString getSignaturePath() const;
    QColor getThemeColor() const;
    QString getNomBanque() const;   // ← CORRIGÉ
    QString getRib() const;
    
signals:
    void configChanged(const EntrepriseConfig &cfg);
    
private:
    explicit EntrepriseConfigManager(QObject *parent = nullptr);
    static EntrepriseConfigManager *m_instance;
};

#endif