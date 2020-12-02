/*
 * This file is part of the KDE project
 *
 * Copyright (c) 2016 Spencer Brown <sbrown655@gmail.com>
 * Copyright (c) 2020 Deif Lou <ginoba@gmail.com>
 * 
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_GRADIENT_MAP_FILTER_CONFIGURATION_H
#define KIS_GRADIENT_MAP_FILTER_CONFIGURATION_H

#include <kis_filter_configuration.h>
#include <KisGradientConversion.h>
#include <KoResourceServerProvider.h>
#include <KoColorSpaceRegistry.h>
#include <KoAbstractGradient.h>
#include <KoStopGradient.h>

class KisGradientMapFilterConfiguration;
typedef KisPinnedSharedPtr<KisGradientMapFilterConfiguration> KisGradientMapFilterConfigurationSP;

class KisGradientMapFilterConfiguration : public KisFilterConfiguration
{
public:
    enum ColorMode {
        ColorMode_Blend,
        ColorMode_Nearest,
        ColorMode_Dither
    };

    KisGradientMapFilterConfiguration();
    KisGradientMapFilterConfiguration(qint32 version);
    
    static inline QString defaultName()
    {
        return "gradientmap";
    }
    
    static constexpr qint32 defaultVersion()
    {
        return 2;
    }

    static inline KoAbstractGradientSP defaultGradient()
    {
        KoAbstractGradientSP gradient;
        KoAbstractGradient *resourceGradient = KoResourceServerProvider::instance()->gradientServer()->resources().first();
        if (resourceGradient) {
            gradient = KoAbstractGradientSP(resourceGradient->clone());
        } else {
            KoStopGradient *stopGradient = new KoStopGradient;
            stopGradient->setStops(
                QList<KoGradientStop>()
                << KoGradientStop(0.0, KoColor(Qt::black, KoColorSpaceRegistry::instance()->rgb8(0)), FOREGROUNDSTOP)
                << KoGradientStop(1.0, KoColor(Qt::white, KoColorSpaceRegistry::instance()->rgb8(0)), BACKGROUNDSTOP)
            );
            gradient = KoAbstractGradientSP(static_cast<KoAbstractGradient*>(stopGradient));
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

    KoAbstractGradientSP gradient() const;
    int colorMode() const;

    void setGradient(KoAbstractGradientSP newGradient);
    void setColorMode(int newColorMode);
    void setDefaults();

};

#endif
