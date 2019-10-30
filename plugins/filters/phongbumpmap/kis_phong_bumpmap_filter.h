/*
 *  Copyright (c) 2010 Dmitry Kazakov <dimula73@gmail.com>
 *  Copyright (c) 2010-2011 José Luis Vergara <pentalis@gmail.com>
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

#ifndef KIS_PHONG_BUMPMAP_FILTER_H
#define KIS_PHONG_BUMPMAP_FILTER_H

#include <kis_types.h>
#include <filter/kis_filter.h>

/**
 * This class is an implementation of the phong illumination model.
 * It uses a heightmap as an input mesh (normally taken from 1
 * channel of a colorspace) to achieve a bumpmapping effect with
 * multiple illumination sources.
 */
class KisFilterPhongBumpmap : public KisFilter
{
public:
    KisFilterPhongBumpmap();

public:

    void processImpl(KisPaintDeviceSP device,
                     const QRect& applyRect,
                     const KisFilterConfigurationSP config,
                     KoUpdater *progressUpdater
                     ) const override;

    QRect neededRect(const QRect &rect, const KisFilterConfigurationSP config, int lod) const override;
    QRect changedRect(const QRect &rect, const KisFilterConfigurationSP config, int lod) const override;

    KisConfigWidget *createConfigurationWidget(QWidget *parent, const KisPaintDeviceSP dev, bool useForMasks) const override;
    KisFilterConfigurationSP defaultConfiguration() const override;
private:
    //bool m_usenormalmap;
};

#endif  //KIS_PHONG_BUMPMAP_FILTER_H
