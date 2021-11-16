/*
 * KDE. Krita Project.
 *
 * SPDX-FileCopyrightText: 2020 Deif Lou <ginoba@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <generator/kis_generator.h>
#include <generator/kis_generator_registry.h>
#include <KoResourceLoadResult.h>

#include "KisHalftoneFilterConfiguration.h"

KisHalftoneFilterConfiguration::KisHalftoneFilterConfiguration(const QString & name,
                                                               qint32 version,
                                                               KisResourcesInterfaceSP resourcesInterface)
    : KisFilterConfiguration(name, version, resourcesInterface)
{}

KisHalftoneFilterConfiguration::KisHalftoneFilterConfiguration(const KisHalftoneFilterConfiguration &rhs)
    : KisFilterConfiguration(rhs)
{
    QHashIterator<QString, KisFilterConfigurationSP> it(rhs.m_generatorConfigurationsCache);
    while (it.hasNext()) {
        it.next();
        m_generatorConfigurationsCache[it.key()] = it.value()->clone();
    }
}

KisHalftoneFilterConfiguration::~KisHalftoneFilterConfiguration()
{}

KisFilterConfigurationSP KisHalftoneFilterConfiguration::clone() const
{
    return new KisHalftoneFilterConfiguration(*this);
}

void KisHalftoneFilterConfiguration::setResourcesInterface(KisResourcesInterfaceSP resourcesInterface)
{
    KisFilterConfiguration::setResourcesInterface(resourcesInterface);

    if (mode() == HalftoneMode_IndependentChannels) {
        const QString prefix = colorModelId() + "_channel";
        for (int i = 0; i < 4; ++i) {
            const QString fullPrefix = prefix + QString::number(i) + "_";
            KisFilterConfigurationSP generatorConfig = generatorConfiguration(fullPrefix);
            if (generatorConfig) {
                m_generatorConfigurationsCache[fullPrefix]->setResourcesInterface(resourcesInterface);
            }
        }
    } else {
        const QString prefix = mode() + "_";
        KisFilterConfigurationSP generatorConfig = generatorConfiguration(prefix);
        if (generatorConfig) {
            m_generatorConfigurationsCache[prefix]->setResourcesInterface(resourcesInterface);
        }
    }
}

QList<KoResourceLoadResult> KisHalftoneFilterConfiguration::linkedResources(KisResourcesInterfaceSP globalResourcesInterface) const
{
    QList<KoResourceLoadResult> resourcesList;

    if (mode() == HalftoneMode_IndependentChannels) {
        const QString prefix = colorModelId() + "_channel";
        for (int i = 0; i < 4; ++i) {
            const QString fullPrefix = prefix + QString::number(i) + "_";
            KisFilterConfigurationSP generatorConfig = generatorConfiguration(fullPrefix);
            if (generatorConfig) {
                resourcesList += generatorConfig->linkedResources(globalResourcesInterface);
            }
        }
    } else {
        const QString prefix = mode() + "_";
        KisFilterConfigurationSP generatorConfig = generatorConfiguration(prefix);
        if (generatorConfig) {
            resourcesList += generatorConfig->linkedResources(globalResourcesInterface);
        }
    }

    return resourcesList;
}

QList<KoResourceLoadResult> KisHalftoneFilterConfiguration::embeddedResources(KisResourcesInterfaceSP globalResourcesInterface) const
{
    QList<KoResourceLoadResult> resourcesList;

    if (mode() == HalftoneMode_IndependentChannels) {
        const QString prefix = colorModelId() + "_channel";
        for (int i = 0; i < 4; ++i) {
            const QString fullPrefix = prefix + QString::number(i) + "_";
            KisFilterConfigurationSP generatorConfig = generatorConfiguration(fullPrefix);
            if (generatorConfig) {
                resourcesList += generatorConfig->embeddedResources(globalResourcesInterface);
            }
        }
    } else {
        const QString prefix = mode() + "_";
        KisFilterConfigurationSP generatorConfig = generatorConfiguration(prefix);
        if (generatorConfig) {
            resourcesList += generatorConfig->embeddedResources(globalResourcesInterface);
        }
    }

    return resourcesList;
}

QString KisHalftoneFilterConfiguration::colorModelId() const
{
    return getString("color_model_id", "");
}

QString KisHalftoneFilterConfiguration::mode() const
{
    return getString("mode", "");
}

QString KisHalftoneFilterConfiguration::generatorId(const QString &prefix) const
{
    return getString(prefix + "generator", "");
}

KisFilterConfigurationSP KisHalftoneFilterConfiguration::generatorConfiguration(const QString &prefix) const
{
    if (m_generatorConfigurationsCache.contains(prefix)) {
        return m_generatorConfigurationsCache[prefix];
    } else {
        QStringList generatorIds = KisGeneratorRegistry::instance()->keys();
        QString generatorId = this->generatorId(prefix);
        if (generatorIds.indexOf(generatorId) != -1) {
            QString fullGeneratorId = prefix + "generator_" + generatorId;
            KisGeneratorSP generator = KisGeneratorRegistry::instance()->get(generatorId);
            KisFilterConfigurationSP generatorConfig = generator->defaultConfiguration(resourcesInterface());
            getPrefixedProperties(fullGeneratorId + "_", generatorConfig);
            m_generatorConfigurationsCache[prefix] = generatorConfig;
            return generatorConfig;
        }
    }
    return nullptr;
}

qreal KisHalftoneFilterConfiguration::hardness(const QString &prefix) const
{
    return getDouble(prefix + "hardness", defaultHardness());
}

bool KisHalftoneFilterConfiguration::invert(const QString &prefix) const
{
    return getBool(prefix + "invert", defaultInvert());
}

KoColor KisHalftoneFilterConfiguration::foregroundColor(const QString &prefix) const
{
    return getColor(prefix + "foreground_color", defaultForegroundColor());
}

int KisHalftoneFilterConfiguration::foregroundOpacity(const QString &prefix) const
{
    return getInt(prefix + "foreground_opacity", defaultForegroundOpacity());
}

KoColor KisHalftoneFilterConfiguration::backgroundColor(const QString &prefix) const
{
    return getColor(prefix + "background_color", defaultBackgroundColor());
}

int KisHalftoneFilterConfiguration::backgroundOpacity(const QString &prefix) const
{
    return getInt(prefix + "background_opacity", defaultForegroundOpacity());
}

void KisHalftoneFilterConfiguration::setColorModelId(const QString &newColorModelId)
{
    setProperty("color_model_id", newColorModelId);
}

void KisHalftoneFilterConfiguration::setMode(const QString &newMode)
{
    setProperty("mode", newMode);
}

void KisHalftoneFilterConfiguration::setGeneratorId(const QString &prefix, const QString &id)
{
    setProperty(prefix + "generator", id);
}

void KisHalftoneFilterConfiguration::setGeneratorConfiguration(const QString &prefix, KisFilterConfigurationSP config)
{
    if (!config) {
        return;
    }

    QString generatorId = this->generatorId(prefix);
    QString fullGeneratorId = prefix + "generator_" + generatorId;
    setPrefixedProperties(fullGeneratorId + "_", config);
    m_generatorConfigurationsCache[prefix] = config;
}

void KisHalftoneFilterConfiguration::setHardness(const QString & prefix, qreal newHardness)
{
    setProperty(prefix + "hardness", newHardness);
}

void KisHalftoneFilterConfiguration::setInvert(const QString & prefix, bool newInvert)
{
    setProperty(prefix + "invert", newInvert);
}

void KisHalftoneFilterConfiguration::setForegroundColor(const QString & prefix, const KoColor & newForegroundColor)
{
    QVariant v;
    v.setValue(newForegroundColor);
    setProperty(prefix + "foreground_color", v);
}

void KisHalftoneFilterConfiguration::setForegroundOpacity(const QString & prefix, int newForegroundOpacity)
{
    setProperty(prefix + "foreground_opacity", newForegroundOpacity);
}
void KisHalftoneFilterConfiguration::setBackgroundColor(const QString & prefix, const KoColor & newBackgroundColor)
{
    QVariant v;
    v.setValue(newBackgroundColor);
    setProperty(prefix + "background_color", v);
}

void KisHalftoneFilterConfiguration::setBackgroundOpacity(const QString & prefix, int newBackgroundOpacity)
{
    setProperty(prefix + "background_opacity", newBackgroundOpacity);
}
