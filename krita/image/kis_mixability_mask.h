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
#ifndef _KIS_MIXABILITY_MASK_
#define _KIS_MIXABILITY_MASK_

#include "kis_types.h"

#include "kis_mask.h"
/**
   Mixability affects both the spreading of paint through gravity
   and wetness and the and transfer of paint from the canvas to the
   brush.

   See http://gamma.cs.unc.edu/viscous/Baxter-ViscousPaint-CASA04.pdf
   for a scholarly discussion.
*/

class KisMixabilityMask : public KisMask
{
    KisMixabilityMask( KisPaintDeviceSP device );
    ~KisMixabilityMask();
    KisMixabilityMask( const KisMixabilityMask& rhs );
};

#endif // _KIS_MIXABILITY_MASK_
