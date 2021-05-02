/*
 * KDE. Krita Project.
 *
 * SPDX-FileCopyrightText: 2020 Deif Lou <ginoba@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <generator/kis_generator.h>
#include <generator/kis_generator_registry.h>

#include "KisHalftoneFilterConfiguration.h"

KisHalftoneFilterConfiguration::KisHalftoneFilterConfiguration(const QString & name,
                                                               qint32 version,
                                                               KisResourcesInterfaceSP resourcesInterface)
    : KisFilterConfiguration(name, version, resourcesInterface)
{}

KisHalftoneFilterConfiguration::KisHalftoneFilterConfiguration(const KisHalftoneFilterConfiguration &rhs)
    : KisFilterConfiguration(rhs)
{}

KisHalftoneFilterConfiguration::~KisHalftoneFilterConfiguration()
{}

KisFilterConfigurationSP KisHalftoneFilterConfiguration::clone() const
{
    return new KisHalftoneFilterConfiguration(*this);
}

QList<KoResourceSP> KisHalftoneFilterConfiguration::linkedResources(KisResourcesInterfaceSP globalResourcesInterface) const
{
    QList<KoResourceSP> resourcesList;

    if (mode() == HalftoneMode_IndependentChannels) {
        QString prefix = colorModelId() + "_channel";
        for (int i = 0; i < 4; ++i) {
            QString fullPrefix = prefix + QString::number(i) + "_";
            KisFilterConfigurationSP generatorConfig = generatorConfiguration(fullPrefix);
            if (generatorConfig) {
                resourcesList += generatorConfig->linkedResources(globalResourcesInterface);
            }
        }
    } else {
        QString prefix = mode() + "_";
        KisFilterConfigurationSP generatorConfig = generatorConfiguration(prefix);
        resourcesList += generatorConfig->linkedResources(globalResourcesInterface);
    }

    return resourcesList;
}

QList<KoResourceSP> KisHalftoneFilterConfiguration::embeddedResources(KisResourcesInterfaceSP globalResourcesInterface) const
{
    QList<KoResourceSP> resourcesList;

    if (mode() == HalftoneMode_IndependentChannels) {
        QString prefix = colorModelId() + "_channel";
        for (int i = 0; i < 4; ++i) {
            QString fullPrefix = prefix + QString::number(i) + "_";
            KisFilterConfigurationSP generatorConfig = generatorConfiguration(fullPrefix);
            if (generatorConfig) {
                resourcesList += generatorConfig->embeddedResources(globalResourcesInterface);
            }
        }
    } else {
        QString prefix = mode() + "_";
        KisFilterConfigurationSP generatorConfig = generatorConfiguration(prefix);
        resourcesList += generatorConfig->embeddedResources(globalResourcesInterface);
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
    QStringList generatorIds = KisGeneratorRegistry::instance()->keys();
    QString generatorId = this->generatorId(prefix);
    if (generatorIds.indexOf(generatorId) != -1) {
        QString fullGeneratorId = prefix + "generator_" + generatorId;
        KisGeneratorSP generator = KisGeneratorRegistry::instance()->get(generatorId);
        KisFilterConfigurationSP generatorConfig = generator->defaultConfiguration(resourcesInterface());
        getPrefixedProperties(fullGeneratorId + "_", generatorConfig);
        return generatorConfig;
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
