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
#ifndef _KIS_SELECTION_MASK_
#define _KIS_SELECTION_MASK_

#include <QRect>
#include "kicon.h"

#include "kis_types.h"
#include "kis_mask.h"

/**
 * An selection mask is a single channel mask that applies a
 * particular selection to the layer the mask belongs to. A selection
 * can contain both
*/
class KRITAIMAGE_EXPORT KisSelectionMask : public KisMask
{
public:

    /**
     * Create an empty selection mask. There is filter and no layer
     * associated with this mask.
     */
    KisSelectionMask();

    virtual ~KisSelectionMask();
    KisSelectionMask( const KisSelectionMask& rhs );

    bool allowAsChild( KisNodeSP );

    QIcon icon() const
        {
            return KIcon(""); // XXX: Find nice icon for the subclasses.
        }

    KisNodeSP clone()
    {
        return KisNodeSP(new KisSelectionMask(*this));
    }


    KisSelectionSP selection() const;

    /// Set the selection of this adjustment layer to a copy of selection.
    void setSelection(KisSelectionSP selection);

    /**
     * overriden from KisBaseNode
     */
    qint32 x() const;

    /**
     * overriden from KisBaseNode
     */
    void setX(qint32 x);

    /**
     * overriden from KisBaseNode
     */
    qint32 y() const;

    /**
     * overriden from KisBaseNode
     */
    void setY(qint32 y);

    /**
     * overriden from KisBaseNode
     */
    QRect extent() const;

    /**
     * overriden from KisBaseNode
     */
    QRect exactBounds() const;

private:

    KisImageSP image() const;

    class Private;
    Private * const m_d;

};

#endif //_KIS_SELECTION_MASK_
