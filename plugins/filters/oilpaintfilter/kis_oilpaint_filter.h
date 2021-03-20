/*
 * This file is part of the KDE project
 *
 * SPDX-FileCopyrightText: Michael Thaler <michael.thaler@physik.tu-muenchen.de>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef _KIS_OILPAINT_FILTER_H_
#define _KIS_OILPAINT_FILTER_H_

#include "filter/kis_filter.h"
#include "kis_config_widget.h"

class KisOilPaintFilter : public KisFilter
{
public:
    KisOilPaintFilter();
public:

    void processImpl(KisPaintDeviceSP device,
                     const QRect& applyRect,
                     const KisFilterConfigurationSP config,
                     KoUpdater* progressUpdater ) const override;
    static inline KoID id() {
        return KoID("oilpaint", i18n("Oilpaint"));
    }


    QRect neededRect(const QRect & rect, const KisFilterConfigurationSP _config, int lod) const override;
    QRect changedRect(const QRect & rect, const KisFilterConfigurationSP _config, int lod) const override;

    KisFilterConfigurationSP defaultConfiguration(KisResourcesInterfaceSP resourcesInterface) const override;
public:
    KisConfigWidget * createConfigurationWidget(QWidget* parent, const KisPaintDeviceSP dev, bool useForMasks) const override;

private:
    void OilPaint(const KisPaintDeviceSP src, KisPaintDeviceSP dst, const QRect &applyRect,
                  int BrushSize, int Smoothness, KoUpdater* progressUpdater) const;
    void MostFrequentColor(KisPaintDeviceSP src, quint8* dst, const QRect& bounds, int X, int Y, int Radius, int Intensity) const;
};

#endif
