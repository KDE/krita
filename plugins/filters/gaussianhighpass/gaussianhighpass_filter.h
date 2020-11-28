/*
 * This file is part of Krita
 *
 * SPDX-FileCopyrightText: 2019 Miguel Lopez <reptillia39@live.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_GAUSSIANHIGHPASS_FILTER_H
#define KIS_GAUSSIANHIGHPASS_FILTER_H

#include "filter/kis_filter.h"
#include "kis_cached_paint_device.h"


class KisGaussianHighPassFilter : public KisFilter
{
public:

    KisGaussianHighPassFilter();

    void processImpl(KisPaintDeviceSP device,
                     const QRect& applyRect,
                     const KisFilterConfigurationSP config,
                     KoUpdater *
                     ) const override;

    static inline KoID id() {
        return KoID("gaussianhighpass", i18n("Gaussian High Pass"));
    }

    KisConfigWidget * createConfigurationWidget(QWidget* parent, const KisPaintDeviceSP dev, bool useForMasks) const override;
    KisFilterConfigurationSP defaultConfiguration(KisResourcesInterfaceSP resourcesInterface) const override;

    QRect changedRect(const QRect & rect, const KisFilterConfigurationSP _config, int lod) const override;
    QRect neededRect(const QRect & rect, const KisFilterConfigurationSP _config, int lod) const override;

private:
    mutable KisCachedPaintDevice m_cachedPaintDevice;
};

#endif
