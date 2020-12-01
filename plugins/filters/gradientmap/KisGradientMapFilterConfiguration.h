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

using KoStopGradientSP = QSharedPointer<KoStopGradient>;

class KisGradientMapFilterConfiguration;
typedef KisPinnedSharedPtr<KisGradientMapFilterConfiguration> KisGradientMapFilterConfigurationSP;

class KisGradientMapFilterConfiguration : public KisFilterConfiguration
{
public:
    KisGradientMapFilterConfiguration(const QString & name, qint32 version);
    
    KoStopGradientSP gradient() const;

private:
    KoStopGradientSP gradientImpl() const;
};

#endif
