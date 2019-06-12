/*
 * This file is part of Krita
 *
 * Copyright (c) 2019 Miguel Lopez <reptillia39@live.com>
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

#ifndef KIS_GUASSIANHIGHPASS_FILTER_H
#define KIS_GUASSIANHIGHPASS_FILTER_H

#include "filter/kis_filter.h"
#include "kis_cached_paint_device.h"


class KisGuassianHighPassFilter : public KisFilter
{
public:

    KisGuassianHighPassFilter();

    void processImpl(KisPaintDeviceSP device,
                     const QRect& applyRect,
                     const KisFilterConfigurationSP config,
                     KoUpdater *
                     ) const override;

    static inline KoID id() {
        return KoID("guassianhighpass", i18n("Guassian High Pass"));
    }

    KisConfigWidget * createConfigurationWidget(QWidget* parent, const KisPaintDeviceSP dev, bool useForMasks) const override;
    KisFilterConfigurationSP factoryConfiguration() const override;

    QRect changedRect(const QRect & rect, const KisFilterConfigurationSP _config, int lod) const override;
    QRect neededRect(const QRect & rect, const KisFilterConfigurationSP _config, int lod) const override;

private:
    mutable KisCachedPaintDevice m_cachedPaintDevice;
};

#endif
