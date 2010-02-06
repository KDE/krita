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
#ifndef _KIS_TRANSPARENCY_MASK_
#define _KIS_TRANSPARENCY_MASK_

#include "kis_types.h"
#include "kis_mask.h"
#include "kis_effect_mask.h"

class QRect;

/**
 *  A transparency mask is a single channel mask that applies a particular
 *  transparency to the layer the mask belongs to. It differs from an
 *  adjustment layer in that it only works on its parent layer, while
 *  adjustment layers work on all layers below it in its layer group.
 *
 *  XXX: Use KisConfig::useProjections() to enable/disable the caching of
 *       the projection.
 */
class KRITAIMAGE_EXPORT KisTransparencyMask : public KisEffectMask
{
    Q_OBJECT

public:

    KisTransparencyMask();
    KisTransparencyMask(const KisTransparencyMask& rhs);
    virtual ~KisTransparencyMask();

    KisNodeSP clone() const {
        return KisNodeSP(new KisTransparencyMask(*this));
    }

    bool allowAsChild(KisNodeSP) const;
    QRect decorateRect(KisPaintDeviceSP &src, KisPaintDeviceSP &dst,
                       const QRect & rc) const;
    QIcon icon() const;
    bool accept(KisNodeVisitor &v);

    QRect changeRect(const QRect &rect) const;
    QRect needRect(const QRect &rect, PositionToFilthy pos = NORMAL) const;
};

#endif //_KIS_TRANSPARENCY_MASK_
