/*
 * This file is part of Krita
 *
 * SPDX-FileCopyrightText: 2016 Spencer Brown <sbrown655@gmail.com>
 * SPDX-FileCopyrightText: 2020 Deif Lou <ginoba@gmail.com>
 * 
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_GRADIENT_MAP_FILTER_H
#define KIS_GRADIENT_MAP_FILTER_H

#include <QObject>

#include <filter/kis_filter.h>
#include <kis_filter_configuration.h>

class KisConfigWidget;

class KisGradientMapFilter : public KisFilter
{
public:
    KisGradientMapFilter();

    static inline KoID id() {
        return KoID("gradientmap", i18n("Gradient Map"));
    }

    void processImpl(KisPaintDeviceSP device,
                     const QRect& applyRect,
                     const KisFilterConfigurationSP config,
                     KoUpdater *progressUpdater) const override;

    template <typename ColorModeStrategy>
    void processImpl(KisPaintDeviceSP device,
                     const QRect& applyRect,
                     const KisFilterConfigurationSP config,
                     KoUpdater *progressUpdater,
                     const ColorModeStrategy &colorModeStrategy) const;

    KisFilterConfigurationSP factoryConfiguration(KisResourcesInterfaceSP resourcesInterface) const override;
    KisFilterConfigurationSP defaultConfiguration(KisResourcesInterfaceSP resourcesInterface) const override;
    KisConfigWidget* createConfigurationWidget(QWidget* parent, const KisPaintDeviceSP dev, bool useForMasks) const override;
};

#endif
