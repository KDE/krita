/*
 * Copyright (c) 2006 Boudewijn Rempt (boud@valdyas.org)
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
#ifndef KIS_SUBSTRATE_H
#define KIS_SUBSTRATE_H

#include <qrect.h>
#include <ksharedptr.h>

class KisImage;

/// All values are normalized to a range between 0 and 1.
/// XXX: Do we need more?
struct KisSubstratePixel {
    float height;      // absolute height of the current position
    float smoothness;  // determines how easily the painting tool "slips" over the surface
    float absorbency;  // determines how much wetness the substrate can absorb. XXX: How about speed of absorbing?
    float r;           //.Red component of reflectivity
    float g;           // Green component of reflectivity
    float b;           // Blue component of reflectivity
    float transmittance; // Similar to alpha. XXX: Ask Leonardo about this.
};

/**
 * This abstract class defines the properties of a substrate -- that is, the simulation
 * of the paper or canvas for natural media.
 *
 * Subclass this interface to define a specific type of substrate: repeating,
 * or full-size, with specific and cool ways of generating the surface, or
 * maybe based on scans of real substrates.
 */
class KisSubstrate : public KShared {

public:

    KisSubstrate(KisImage * /*img*/) : KShared() {};
    virtual ~KisSubstrate() {};


    /**
     * Copy the pixel values in the specified rect into an array of Substrate.
     * Make sure the array is big enough!
     */ 
    virtual void getPixels(KisSubstratePixel * /*substrate*/, const QRect & /*rc*/) = 0;

    /**
     * Return a pointer to the substrate at the specified position. Note that
     * you cannot do pointe arithmetic with this value: the position of the
     * neighbouring pixels cannot be determined from this value
     */
    virtual KisSubstratePixel * getPixel(uint x, uint y) = 0;

};

#endif
