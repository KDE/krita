/*
 * This file is part of the KDE project
 *
 * Copyright (c) Michael Thaler <michael.thaler@physik.tu-muenchen.de>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
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

    KisFilterConfigurationSP defaultConfiguration() const override;

    QRect neededRect(const QRect & rect, const KisFilterConfigurationSP _config, int lod) const override;
    QRect changedRect(const QRect & rect, const KisFilterConfigurationSP _config, int lod) const override;

public:
    KisConfigWidget * createConfigurationWidget(QWidget* parent, const KisPaintDeviceSP dev, bool useForMasks) const override;

private:
    void OilPaint(const KisPaintDeviceSP src, KisPaintDeviceSP dst, const QRect &applyRect,
                  int BrushSize, int Smoothness, KoUpdater* progressUpdater) const;
    void MostFrequentColor(KisPaintDeviceSP src, quint8* dst, const QRect& bounds, int X, int Y, int Radius, int Intensity) const;
};

#endif
