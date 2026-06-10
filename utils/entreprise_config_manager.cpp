#include "entreprise_config_manager.h"
#include <QSettings>

EntrepriseConfigManager* EntrepriseConfigManager::m_instance = nullptr;

EntrepriseConfigManager* EntrepriseConfigManager::instance()
{
    if (!m_instance) m_instance = new EntrepriseConfigManager;
    return m_instance;
}

EntrepriseConfigManager::EntrepriseConfigManager(QObject *parent) : QObject(parent) {}

EntrepriseConfig EntrepriseConfigManager::loadConfig()
{
    QSettings s("MonApp", "FacturPro");
    EntrepriseConfig cfg;
    cfg.nom = s.value("entreprise/nom").toString();
    cfg.rib = s.value("entreprise/rib").toString();
    cfg.logoPath = s.value("entreprise/logoPath").toString();
    cfg.signaturePath = s.value("entreprise/signaturePath").toString();
    cfg.themeCouleur = QColor(s.value("entreprise/theme", "#2563EB").toString());
    cfg.configured = s.value("entreprise/configured", false).toBool();
    return cfg;
}

void EntrepriseConfigManager::saveConfig(const EntrepriseConfig &cfg)
{
    QSettings s("MonApp", "FacturPro");
    s.setValue("entreprise/nom", cfg.nom);
    s.setValue("entreprise/rib", cfg.rib);
    s.setValue("entreprise/logoPath", cfg.logoPath);
    s.setValue("entreprise/signaturePath", cfg.signaturePath);
    s.setValue("entreprise/theme", cfg.themeCouleur.name());
    s.setValue("entreprise/configured", cfg.isValid());  // ← CORRIGÉ : utilise isValid()
    emit configChanged(cfg);
}

bool EntrepriseConfigManager::isConfigured() const
{
    QSettings s("MonApp", "FacturPro");
    // Vérifie à la fois le flag ET la validité des données
    bool flag = s.value("entreprise/configured", false).toBool();
    QString nom = s.value("entreprise/nom").toString();
    QString rib = s.value("entreprise/rib").toString();
    return flag && !nom.isEmpty() && !rib.isEmpty();  // ← CORRIGÉ
}

QString EntrepriseConfigManager::getLogoPath() const
{
    return QSettings("MonApp", "FacturPro").value("entreprise/logoPath").toString();
}

QString EntrepriseConfigManager::getSignaturePath() const
{
    return QSettings("MonApp", "FacturPro").value("entreprise/signaturePath").toString();
}

QColor EntrepriseConfigManager::getThemeColor() const
{
    return QColor(QSettings("MonApp", "FacturPro").value("entreprise/theme", "#2563EB").toString());
}

QString EntrepriseConfigManager::getNomBanque() const
{
    return QSettings("MonApp", "FacturPro").value("entreprise/nom").toString();
}

QString EntrepriseConfigManager::getRib() const
{
    return QSettings("MonApp", "FacturPro").value("entreprise/rib").toString();
}