/*
 * This file is part of the KDE project
 *
 * SPDX-FileCopyrightText: 2016 Spencer Brown <sbrown655@gmail.com>
 * SPDX-FileCopyrightText: 2020 Deif Lou <ginoba@gmail.com>
 * 
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_GRADIENT_MAP_FILTER_CONFIGURATION_H
#define KIS_GRADIENT_MAP_FILTER_CONFIGURATION_H

#include <kis_filter_configuration.h>
#include <KisGradientConversion.h>
#include <KisResourcesInterface.h>
#include <KoColorSpaceRegistry.h>
#include <KoAbstractGradient.h>
#include <KoStopGradient.h>


class KisGradientMapFilterConfiguration;
typedef KisPinnedSharedPtr<KisGradientMapFilterConfiguration> KisGradientMapFilterConfigurationSP;

class KisGradientMapFilterConfiguration : public KisFilterConfiguration
{
public:
    enum ColorMode
    {
        ColorMode_Blend,
        ColorMode_Nearest,
        ColorMode_Dither
    };

    KisGradientMapFilterConfiguration(KisResourcesInterfaceSP resourcesInterface);
    KisGradientMapFilterConfiguration(qint32 version, KisResourcesInterfaceSP resourcesInterface);
    KisGradientMapFilterConfiguration(const KisGradientMapFilterConfiguration &rhs);
    
    virtual KisFilterConfigurationSP clone() const override;

    QList<KoResourceSP> linkedResources(KisResourcesInterfaceSP globalResourcesInterface) const override;
    QList<KoResourceSP> embeddedResources(KisResourcesInterfaceSP globalResourcesInterface) const override;
    
    static inline QString defaultName()
    {
        return "gradientmap";
    }
    
    static constexpr qint32 defaultVersion()
    {
        return 2;
    }

    static inline KoAbstractGradientSP defaultGradient(KisResourcesInterfaceSP resourcesInterface)
    {
        KoAbstractGradientSP gradient;
        KoAbstractGradientSP resourceGradient = resourcesInterface->source<KoAbstractGradient>(ResourceType::Gradients).fallbackResource();
        if (resourceGradient) {
            gradient = resourceGradient->clone().dynamicCast<KoAbstractGradient>();
        } else {
            KoStopGradientSP stopGradient(new KoStopGradient);
            stopGradient->setStops(
                QList<KoGradientStop>()
                << KoGradientStop(0.0, KoColor(Qt::black, KoColorSpaceRegistry::instance()->rgb8(0)), FOREGROUNDSTOP)
                << KoGradientStop(1.0, KoColor(Qt::white, KoColorSpaceRegistry::instance()->rgb8(0)), BACKGROUNDSTOP)
            );
            gradient = stopGradient.staticCast<KoAbstractGradient>();
        }
        if (gradient) {
            gradient->setName(i18nc("Default gradient name for the gradient generator", "Unnamed"));
            gradient->setValid(true);
        }
        return gradient;
    }

    static constexpr int defaultColorMode()
    {
        return ColorMode_Blend;
    }

    KoAbstractGradientSP gradient(KoAbstractGradientSP fallbackGradient = nullptr) const;
    int colorMode() const;

    void setGradient(KoAbstractGradientSP newGradient);
    void setColorMode(int newColorMode);
    void setDefaults();
};

#endif
