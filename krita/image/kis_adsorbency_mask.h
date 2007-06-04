/*
 *  Copyright (c) 2006 Boudewijn Rempt <boud@valdyas.org>
 *
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
#ifndef _KIS_ADSORBENCY_MASK_
#define _KIS_ADSORBENCY_MASK_

#include "kis_types.h"

#include "kis_mask.h"
/**
   A adsorbency mask is a single channel mask that applies a particular
   adsorbency to the layer the mask belongs to. Adsorbency influences
   the drying rate of wet material through adsorbing wetness into the
   canvas material.
*/

class KRITAIMAGE_EXPORT KisAdsorbencyMask : public KisMask
{
    KisAdsorbencyMask( KisPaintDeviceSP device );
    virtual ~KisAdsorbencyMask();
    KisAdsorbencyMask( const KisAdsorbencyMask& rhs );
    QString id() { return "KisAdsorbencyMask"; }
};

#endif //_KIS_ADSORBENCY_MASK_
