/*
 *  SPDX-FileCopyrightText: 2020 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISFILTERFASTCOLOROVERLAY_H
#define KISFILTERFASTCOLOROVERLAY_H

#include "filter/kis_filter.h"

class KisFilterFastColorOverlay : public KisFilter
{
public:
    static QColor defaultColor();
    static int defaultOpacity();
    static QString defaultCompositeOp();

    KisFilterFastColorOverlay();

    void processImpl(KisPaintDeviceSP device,
                     const QRect& rect,
                     const KisFilterConfigurationSP config,
                     KoUpdater* progressUpdater
                     ) const override;

    static inline KoID id() {
        return KoID("fastcoloroverlay", i18n("Fast Color Overlay"));
    }

    KisConfigWidget * createConfigurationWidget(QWidget* parent, const KisPaintDeviceSP dev, bool useForMasks) const override;
    KisFilterConfigurationSP defaultConfiguration(KisResourcesInterfaceSP resourcesInterface) const override;
};

#endif // KISFILTERFASTCOLOROVERLAY_H
