/* This file is part of the KDE project
 *
 * Copyright (C) 2017 Grigory Tantsevov <tantsevov@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef KIS_ABSTRACT_SPLATS_PLANE_H
#define KIS_ABSTRACT_SPLATS_PLANE_H

#include "kis_splat.h"
#include "kis_wetmap.h"
#include "kis_paint_device.h"
#include <QRect>

/**
 * Base class for the splats' containers in watercolor
 * brush. It needs for separating splats in different states.
 * This plane storages list of splats and paint it on
 * paint device. So for this it can:
 *
 * 1) add() splats to list and paint device;
 *
 * 2) remove() splats from list and paint device;
 *
 * 3) paint() paint device with given painter
 */
class KisBaseSplatsPlane
{
public:
    KisBaseSplatsPlane(bool useCaching,
                       KisBaseSplatsPlane *lowLvlPlane = 0,
                       const KoColorSpace *colorSpace = 0);
    virtual ~KisBaseSplatsPlane();

    /**
     * @brief add splat to list of splats and paint device
     * @param splat - new splat
     */
    void add(KisSplat *splat);

    /**
     * @brief revome splat from plane
     * @param splat - removing splat
     */
    void remove(KisSplat *splat);

    /**
     * @brief paint plane on given painter in given rect
     * @param gc - painter
     * @param rect - zone of paint
     */
    void paint(KisPainter *gc, QRect rect);

    /**
     * @brief update plane
     * @param wetMap - base for updating
     */
    virtual QRect update(KisWetMap *wetMap);


protected:
    QList<KisSplat*> m_splats;
    KisBaseSplatsPlane *m_lowLvlPlane;

protected:
    void setDirty(KisSplat *splat);
    QList<KisSplat *> m_dirtySplats;

private:
    KisPaintDeviceSP m_cachedPD;
    bool m_isDirty;
    bool m_useCaching;
};

#endif // KIS_ABSTRACT_SPLATS_PLANE_H
