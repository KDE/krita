/*
 * This file is part of the KDE project
 *
 * SPDX-FileCopyrightText: Michael Thaler <michael.thaler@physik.tu-muenchen.de>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef _KIS_PIXELIZE_FILTER_H_
#define _KIS_PIXELIZE_FILTER_H_

#include "filter/kis_filter.h"
#include "kis_config_widget.h"

class KisPixelizeFilter : public KisFilter
{
public:
    KisPixelizeFilter();
public:

    void processImpl(KisPaintDeviceSP device,
                     const QRect& applyRect,
                     const KisFilterConfigurationSP config,
                     KoUpdater* progressUpdater) const override;

    static inline KoID id() {
        return KoID("pixelize", i18n("Pixelize"));
    }

    QRect neededRect(const QRect & rect, const KisFilterConfigurationSP config, int lod) const override;
    QRect changedRect(const QRect & rect, const KisFilterConfigurationSP config, int lod) const override;

public:
    KisConfigWidget * createConfigurationWidget(QWidget* parent, const KisPaintDeviceSP dev, bool useForMasks) const override;
    KisFilterConfigurationSP defaultConfiguration(KisResourcesInterfaceSP resourcesInterface) const override;
};

#endif
