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
    using KisFilter::process;

    void process(KisPaintDeviceSP device,
                const QRect& applyRect,
                const KisFilterConfiguration *config,
                KoUpdater *progressUpdater
                ) const;

    QRect neededRect(const QRect &rect, const KisFilterConfiguration *config) const;
    QRect changedRect(const QRect &rect, const KisFilterConfiguration *config) const;

    virtual KisConfigWidget *createConfigurationWidget(QWidget *parent, const KisPaintDeviceSP dev, const KisImageWSP image = 0) const;
    virtual KisFilterConfiguration* factoryConfiguration(const KisPaintDeviceSP) const;
};

#endif  //KIS_PHONG_BUMPMAP_FILTER_H
