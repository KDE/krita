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

    static inline KoStopGradientSP defaultGradient()
    {
        KoStopGradientSP gradient =
            KoStopGradientSP(
                KisGradientConversion::toStopGradient(
                    KoResourceServerProvider::instance()->gradientServer()->resources().first()
                )
            );
        gradient->setName(i18nc("Default gradient name for the gradient generator", "Unnamed"));
        gradient->setValid(true);
        return gradient;
    }

    static constexpr int defaultColorMode()
    {
        return ColorMode_Blend;
    }

    KoStopGradientSP gradient() const;
    int colorMode() const;

    void setGradient(KoStopGradientSP newGradient);
    void setColorMode(int newColorMode);
    void setDefaults();

};

#endif
