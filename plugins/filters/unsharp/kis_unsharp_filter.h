/*
 * This file is part of Krita
 *
 * SPDX-FileCopyrightText: 2006 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_UNSHARP_FILTER_H
#define KIS_UNSHARP_FILTER_H

#include "filter/kis_filter.h"

class KisUnsharpFilter : public KisFilter
{
public:

    KisUnsharpFilter();

    void processImpl(KisPaintDeviceSP device,
                     const QRect& applyRect,
                     const KisFilterConfigurationSP config,
                     KoUpdater* progressUpdater
                     ) const override;

    static inline KoID id() {
        return KoID("unsharp", i18n("Unsharp Mask"));
    }

    KisConfigWidget * createConfigurationWidget(QWidget* parent, const KisPaintDeviceSP dev, bool useForMasks) const override;
    KisFilterConfigurationSP defaultConfiguration(KisResourcesInterfaceSP resourcesInterface) const override;

    QRect changedRect(const QRect & rect, const KisFilterConfigurationSP _config, int lod) const override;
    QRect neededRect(const QRect & rect, const KisFilterConfigurationSP _config, int lod) const override;

private:
    void processLightnessOnly(KisPaintDeviceSP device,
                              const QRect &rect,
                              quint8 threshold,
                              qreal weights[2],
                              qreal factor,
                              const QBitArray &channelFlags, KoUpdater *progressUpdater) const;

    void processRaw(KisPaintDeviceSP device,
                    const QRect &rect,
                    quint8 threshold,
                    qreal weights[2],
                    qreal factor,
                    const QBitArray &channelFlags, KoUpdater *progressUpdater) const;
};

#endif
