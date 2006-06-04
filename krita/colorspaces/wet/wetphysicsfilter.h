/*
 * This file is part of Krita
 *
 * Copyright (c) 2005 Boudewijn Rempt <boud@valdyas.org>
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

#ifndef WET_PHYSICS_FILTER_H
#define WET_PHYSICS_FILTER_H

#include <klocale.h>

#include <kis_filter.h>
#include <kis_types.h>

#include "kis_wet_colorspace.h"

class KoID;
class QRect;


/**
 * The wet physics filter must be run regularly from a timer
 * or preferably from a thread. Every time the filter is processed
 * the paint flows; every third time, the paint is adsorbed unto the
 * lower pixel and dried.
 *
 * Note: this might also be implemented as three separate filters.
 *       That might even be better.
 */
class WetPhysicsFilter: public KisFilter
{
public:
    WetPhysicsFilter();
public:
    virtual void process(KisPaintDeviceSP src, KisPaintDeviceSP dst, KisFilterConfiguration*, const QRect& r);

    static inline KoID id() { return KoID("wetphysics", i18n("Watercolor Physics Simulation Filter")); };

    virtual bool supportsPainting() { return false; }
    virtual bool supportsPreview() { return false; }
    virtual ColorSpaceIndependence colorSpaceIndependence() { return FULLY_INDEPENDENT; };
    virtual bool workWith(KoColorSpace* cs) { return (cs->id() == KoID("WET")); };

private:

    // Move paint from the paint part of the pixel to the adsorb part.
    void adsorbPixel(WetPix * paint, WetPix * adsorb);

private:

    qint32 m_adsorbCount;


};

#endif
