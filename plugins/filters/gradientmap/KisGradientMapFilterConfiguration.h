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

class KisGradientMapFilterConfiguration;
typedef KisPinnedSharedPtr<KisGradientMapFilterConfiguration> KisGradientMapFilterConfigurationSP;

class KisGradientMapFilterConfiguration : public KisFilterConfiguration
{
public:
    KisGradientMapFilterConfiguration(const QString & name, qint32 version, KisResourcesInterfaceSP resourcesInterface);
    KisGradientMapFilterConfiguration(const KisGradientMapFilterConfiguration &rhs);
    
    virtual KisFilterConfigurationSP clone() const override;

    KoStopGradientSP gradient() const;

    QList<KoResourceSP> linkedResources(KisResourcesInterfaceSP globalResourcesInterface) const override;
    QList<KoResourceSP> embeddedResources(KisResourcesInterfaceSP globalResourcesInterface) const override;

private:
    KoStopGradientSP gradientImpl(KisResourcesInterfaceSP resourcesInterface) const;
};

#endif
