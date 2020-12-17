/*
 * This file is part of Krita
 *
 * SPDX-FileCopyrightText: Michael Thaler <michael.thaler@physik.tu-muenchen.de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef _KIS_RAINDROPS_FILTER_H_
#define _KIS_RAINDROPS_FILTER_H_

#include "filter/kis_filter.h"
#include "kis_config_widget.h"
#include "kis_paint_device.h"

class KisRainDropsFilter : public KisFilter
{
public:
    KisRainDropsFilter();
public:

    void processImpl(KisPaintDeviceSP device,
                     const QRect& applyRect,
                     const KisFilterConfigurationSP config,
                     KoUpdater* progressUpdater) const override;
    static inline KoID id() {
        return KoID("raindrops", i18n("Raindrops"));
    }

    KisFilterConfigurationSP defaultConfiguration(KisResourcesInterfaceSP resourcesInterface) const override;
public:
    KisConfigWidget * createConfigurationWidget(QWidget* parent, const KisPaintDeviceSP dev, bool useForMasks) const override;
private:
    bool** CreateBoolArray(uint Columns, uint Rows) const;
    void   FreeBoolArray(bool** lpbArray, uint Columns) const;
    uchar  LimitValues(int ColorValue) const;
};

#endif
