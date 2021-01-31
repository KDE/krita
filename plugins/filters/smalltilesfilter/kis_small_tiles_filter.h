/*
 * This file is part of the KDE project
 *
 * SPDX-FileCopyrightText: Michael Thaler <michael.thaler@physik.tu-muenchen.de>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef _KIS_SMALL_TILES_FILTER_H_
#define _KIS_SMALL_TILES_FILTER_H_

#include "kis_paint_device.h"
#include "filter/kis_filter.h"
#include "kis_config_widget.h"

class KisSmallTilesFilter : public KisFilter
{

public:
    KisSmallTilesFilter();

public:
    void processImpl(KisPaintDeviceSP device,
                     const QRect& applyRect,
                     const KisFilterConfigurationSP config,
                     KoUpdater* progressUpdater
                     ) const override;
    static inline KoID id() {
        return KoID("smalltiles", i18n("Small Tiles"));
    }

    KisFilterConfigurationSP defaultConfiguration(KisResourcesInterfaceSP resourcesInterface) const override;

public:
    KisConfigWidget * createConfigurationWidget(QWidget* parent, const KisPaintDeviceSP dev, bool useForMasks) const override;

};

#endif
