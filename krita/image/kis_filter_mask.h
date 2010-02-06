/*
 *  Copyright (c) 2007 Boudewijn Rempt <boud@valdyas.org>
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
#ifndef _KIS_FILTER_MASK_
#define _KIS_FILTER_MASK_

#include "kis_types.h"
#include "kis_effect_mask.h"

#include "kis_node_filter_interface.h"

class KisFilterConfiguration;

/**
   An filter mask is a single channel mask that applies a particular
   filter to the layer the mask belongs to. It differs from an
   adjustment layer in that it only works on its parent layer, while
   adjustment layers work on all layers below it in its layer group.
*/

class KRITAIMAGE_EXPORT KisFilterMask : public KisEffectMask, public KisNodeFilterInterface
{
    Q_OBJECT

public:

    /**
     * Create an empty filter mask.
     */
    KisFilterMask();

    virtual ~KisFilterMask();

    QIcon icon() const;

    KisNodeSP clone() const {
        return KisNodeSP(new KisFilterMask(*this));
    }

    bool accept(KisNodeVisitor &v);

    KisFilterMask(const KisFilterMask& rhs);

    bool allowAsChild(KisNodeSP) const;

    KisFilterConfiguration * filter() const;
    void setFilter(KisFilterConfiguration * filterConfig);

    QRect decorateRect(KisPaintDeviceSP &src,
                       KisPaintDeviceSP &dst,
                       const QRect & rc) const;

    QRect changeRect(const QRect &rect) const;
    QRect needRect(const QRect &rect, PositionToFilthy pos = NORMAL) const;

private:

    class Private;
    Private * const m_d;
};

#endif //_KIS_FILTER_MASK_
