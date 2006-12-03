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
#ifndef _KIS_WETNESS_MASK_
#define _KIS_WETNESS_MASK_

#include "kis_types.h"

#include "kis_mask.h"
/**
   A wetness mask simulates the wetness of pixels in the parent layer.
   In contrast with applications like Corel Painter, all layers can be
   wet. It depends on the material of the pixel which effect the
   wetness has. A layer with a wetness mask will dry at a particular
   rate through simulated air contact and adsorbency.

   XXX: think of a good way to visualize wetness. With real paint,
   wetness means deeper colors and reflections, but we obviously
   cannot reflect the user's face and hands in our paint.
*/

class KisWetnessMask : public KisMask
{
    KisWetnessMask( KisPaintDeviceSP device );
    ~KisWetnessMask();
    KisWetnessMask( const KisWetnessMask& rhs );

    void setDryingRate( float rate );
    float dryingRate();

private:

    float m_dryingRate;

};

#endif // _KIS_WETNESS_MASK_
