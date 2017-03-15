/*
 *  Copyright (c) 2017 Eugene Ingerman
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
#ifndef _KIS_INPAINT_MASK_
#define _KIS_INPAINT_MASK_

#include "kis_types.h"
#include "kis_transparency_mask.h"

class QRect;

/**
 *  A inpaint mask is a single channel mask that works with inpaint operation to denote area affected by inpaint operation.
 *
 */
class KRITAIMAGE_EXPORT KisInpaintMask : public KisTransparencyMask
{
    Q_OBJECT

public:

    KisInpaintMask();
    KisInpaintMask(const KisInpaintMask& rhs);
    virtual ~KisInpaintMask();

    KisNodeSP clone() const {
        return KisNodeSP(new KisInpaintMask(*this));
    }

    QRect decorateRect(KisPaintDeviceSP &src, KisPaintDeviceSP &dst,
                       const QRect & rc,
                       PositionToFilthy maskPos) const;
};

#endif //_KIS_INPAINT_MASK_
