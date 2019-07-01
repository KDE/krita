/*
 * This file is part of Krita
 *
 * Copyright (c) 2016 Spencer Brown <sbrown655@gmail.com>
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

#ifndef KIS_GRADIENT_MAP_H
#define KIS_GRADIENT_MAP_H

#include <kis_global.h>
#include <kis_types.h>
#include <kis_paint_device.h>
#include <filter/kis_filter.h>
#include <kis_filter_configuration.h>
#include <kis_config_widget.h>
#include <KoUpdater.h>

class KritaFilterGradientMap : public KisFilter
{
public:
    KritaFilterGradientMap();
public:
    enum ColorMode {
        Blend,
        Nearest,
        Dither,
        };

    static inline KoID id() {
        return KoID("gradientmap", i18n("Gradient Map"));
    }

    void processImpl(KisPaintDeviceSP device,
                     const QRect& applyRect,
                     const KisFilterConfigurationSP config,
                     KoUpdater *progressUpdater) const override;

    KisFilterConfigurationSP factoryConfiguration() const override;

    KisConfigWidget* createConfigurationWidget(QWidget* parent, const KisPaintDeviceSP dev, bool useForMasks) const override;
};

#endif
