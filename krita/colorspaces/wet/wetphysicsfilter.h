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

class KisID;
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

    static inline KisID id() { return KisID("wetphysics", i18n("Watercolor Physics Simulation Filter")); };

    virtual bool supportsPainting() { return false; }
    virtual bool supportsPreview() { return false; }
    virtual ColorSpaceIndependence colorSpaceIndependence() { return FULLY_INDEPENDENT; };
    virtual bool workWith(KisColorSpace* cs) { return (cs->id() == KisID("WET")); };
    
private:

    void flow(KisPaintDeviceSP src, KisPaintDeviceSP dst, const QRect & r);
    void dry(KisPaintDeviceSP src, KisPaintDeviceSP dst, const QRect & r);

    // Move stuff from the upperlayer to the lower layer. This is filter-level stuff.
    void adsorb(KisPaintDeviceSP src, KisPaintDeviceSP dst, const QRect & r);

    // NOTE: this does not set the height fields
    void combinePixels (WetPixDbl *dst, WetPixDbl *src1, WetPixDbl *src2);
    void dilutePixel (WetPixDbl *dst, WetPix *src, double dilution);
    void reducePixel (WetPixDbl *dst, WetPix *src, double dilution);

    /*
     * Allows visualization of adsorption by rotating the hue 120 degrees
     * layer-merge combining. src1 is the top layer
     *
     * This does not set the dst h or w fields.
     */
    void mergePixel (WetPixDbl *dst, WetPixDbl *src1, double dilution1, WetPixDbl *src2);


private:

    Q_INT32 m_adsorbCount;


};

#endif
