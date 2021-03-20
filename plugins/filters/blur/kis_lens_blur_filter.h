/*
 * This file is part of Krita
 *
 * SPDX-FileCopyrightText: 2010 Edward Apap <schumifer@hotmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */


#ifndef KIS_LENS_BLUR_FILTER_H
#define KIS_LENS_BLUR_FILTER_H

#include "filter/kis_filter.h"
#include "ui_wdg_lens_blur.h"

#include <Eigen/Core>

class KisLensBlurFilter : public KisFilter
{
public:
    KisLensBlurFilter();
public:

    void processImpl(KisPaintDeviceSP src,
                     const QRect& size,
                     const KisFilterConfigurationSP config,
                     KoUpdater* progressUpdater
                     ) const override;
    static inline KoID id() {
        return KoID("lens blur", i18n("Lens Blur"));
    }

    KisFilterConfigurationSP defaultConfiguration(KisResourcesInterfaceSP resourcesInterface) const override;

    static QSize getKernelHalfSize(const KisFilterConfigurationSP config, int lod);

    QRect neededRect(const QRect & rect, const KisFilterConfigurationSP _config, int lod) const override;
    QRect changedRect(const QRect & rect, const KisFilterConfigurationSP _config, int lod) const override;

public:
    KisConfigWidget * createConfigurationWidget(QWidget* parent, const KisPaintDeviceSP dev, bool useForMasks) const override;

private:
    static QPolygonF getIrisPolygon(const KisFilterConfigurationSP config, int lod);
};

#endif
