/*
 *  SPDX-FileCopyrightText: 2010 Dmitry Kazakov <dimula73@gmail.com>
 *  SPDX-FileCopyrightText: 2010-2011 José Luis Vergara <pentalis@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
    KisFilterConfigurationSP defaultConfiguration(KisResourcesInterfaceSP resourcesInterface) const override;
private:
    //bool m_usenormalmap;
};

#endif  //KIS_PHONG_BUMPMAP_FILTER_H
