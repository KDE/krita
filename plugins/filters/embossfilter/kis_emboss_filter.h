/*
 * This file is part of the KDE project
 *
 * SPDX-FileCopyrightText: Michael Thaler <michael.thaler@physik.tu-muenchen.de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef _KIS_EMBOSS_FILTER_H_
#define _KIS_EMBOSS_FILTER_H_

#include "filter/kis_filter.h"
#include "kis_config_widget.h"

class KisEmbossFilter : public KisFilter
{
public:
    KisEmbossFilter();
public:

    void processImpl(KisPaintDeviceSP device,
                     const QRect& applyRect,
                     const KisFilterConfigurationSP config,
                     KoUpdater* progressUpdater
                     ) const override;
    static inline KoID id() {
        return KoID("emboss", i18n("Emboss with Variable Depth"));
    }

public:
    KisConfigWidget * createConfigurationWidget(QWidget* parent, const KisPaintDeviceSP dev, bool useForMasks) const override;
protected:
    KisFilterConfigurationSP defaultConfiguration(KisResourcesInterfaceSP resourcesInterface) const override;

private:
    inline int Lim_Max(int Now, int Up, int Max) const;
};

#endif
