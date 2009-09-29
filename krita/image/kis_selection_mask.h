/*
 *  Copyright (c) 2006 Boudewijn Rempt <boud@valdyas.org>
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
 * can contain both vector and pixel selection components.
*/
class KRITAIMAGE_EXPORT KisSelectionMask : public KisMask
{
    Q_OBJECT
public:

    /**
     * Create an empty selection mask. There is filter and no layer
     * associated with this mask.
     */
    KisSelectionMask(KisImageWSP image);

    virtual ~KisSelectionMask();
    KisSelectionMask(const KisSelectionMask& rhs);

    bool allowAsChild(KisNodeSP) const;

    QIcon icon() const {
        return KIcon("edit-paste");
    }

    KisNodeSP clone() const {
        return KisNodeSP(new KisSelectionMask(*this));
    }

    /// Set the selection of this adjustment layer to a copy of selection.
    void setSelection(KisSelectionSP selection);

    bool accept(KisNodeVisitor &v);

    /**
     * @return the deselected selection or 0 if no selection was deselected
     */
    KisSelectionSP deleselectedSelection();

    /**
     * Set deselected selection
     */
    void setDeleselectedSelection(KisSelectionSP selection);

private:

    KisImageWSP image() const;

    class Private;
    Private * const m_d;

};

#endif //_KIS_SELECTION_MASK_
