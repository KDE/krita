/*
 *  Copyright (c) 2004 Boudewijn Rempt <boud@valdyas.org>
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
#ifndef KIS_SELECTION_H_
#define KIS_SELECTION_H_

#include <QRect>

#include "kicon.h"
#include "kis_types.h"
#include "kis_paint_device.h"
#include "kis_mask.h"

#include <krita_export.h>

enum selectionType {
    READ_SELECTION,
    WRITE_PROTECTION,
    READ_WRITE
};

enum selectionMode {
    PIXEL_SELECTION,
    SHAPE_PROTECTION
};

enum selectionAction {
    SELECTION_REPLACE,
    SELECTION_ADD,
    SELECTION_SUBTRACT,
    SELECTION_INTERSECT
};

const QString KIS_SELECTION_ID = "KisSelection";

class KisSelectionComponent;

/**
 * KisSelection contains a byte-map representation of a layer, where
 * the value of a byte signifies whether a corresponding pixel is selected, or not.
 *
 * NOTE: You need to manually call emitSelectionChanged on the owner
 * paint device of a selection. KisSelection does not emit any signals
 * by itself because often you want to combine several actions in to
 * perfom one operation and you do not want recomposition to happen
 * all the time.
 */
class KRITAIMAGE_EXPORT KisSelection : public KisPaintDevice {

public:
    /**
     * Create a new KisSelection.
     *
     * @param dev the parent paint device. The selection will never be
     * bigger than the parent paint device.
     */
    KisSelection(KisPaintDeviceSP dev);

    /**
     * Create a new KisSelection from the given mask. The selection
     * will share its pixel data with the mask
     */
    KisSelection( KisPaintDeviceSP parent, KisMaskSP mask );

    /**
     * Create a new KisSelection. This selection will not have a parent paint device.
     */
    KisSelection();

    /**
     * Copy the selection
     */
    KisSelection(const KisSelection& rhs);

    virtual ~KisSelection();

    QIcon icon() const
        {
            return KIcon("frame-edit"); //XXX: Get good icon!
        }


    /**
     * Returns selectedness of the specified pixel, or 0 if invalid
     * coordinates
     */
    quint8 selected(qint32 x, qint32 y) const;

    /**
     * Invert the selection.
     *
     * XXX: The extent that is inverted is the total
     * extent of the selection projection, not that of the selection
     * components, the parent paint device or the image. Shouldn't we
     * fix this?
     */
    void invert();

    /**
     * Clear the selection.
     *
     * XXX: shouldn't we also clear the selection components?
     */
    void clear();
    void clear(const QRect& r);

    /**
     * Tests if the the rect is totally outside the selection
     */
    bool isTotallyUnselected(QRect r) const;

    /**
     * Tests if the the rect is totally outside the selection, but
     * uses selectedRect instead of selectedRect, and this is faster
     * (but might deliver false positives!)
     */
    bool isProbablyTotallyUnselected(QRect r) const;

    /**
     * Rough, but fastish way of determining the area
     * of the tiles used by the selection.
     */
    QRect selectedRect() const;

    /**
     * Slow, but exact way of determining the rectangle
     * that encloses the selection
     */
    QRect selectedExactRect() const;

    void paint(QImage* img, const QRect & r);

    /**
     * If the parent paint device is interested in keeping up to date
     * with the dirtyness of this selection, set to true
     */
    void setInterestedInDirtyness(bool b) { m_interestedInDirtyness = b; }

    /**
     * returns true if the parent paint device is interested in
     * keeping up with the dirtyness of the selection.
     */
    bool interestedInDirtyness() const { return m_interestedInDirtyness; }

    virtual void setDirty(const QRect & rc);
    virtual void setDirty();

    bool hasPixelSelection() const;
    bool hasShapeSelection() const;

    /**
     * return the pixel selection component of this selection or zero
     * if hasPixelSelection() returns false.
     */
    KisPixelSelectionSP pixelSelection();

    /**
     * return the vector selection component of this selection or zero
     * if hasShapeSelection() returns false.
     */
    KisSelectionComponent* shapeSelection();


    /**
     * Return the pixel selection associated with this selection or
     * create a new one if there is currently no pixel selection
     * component in this selection.
     */
    KisPixelSelectionSP getOrCreatePixelSelection();

    void setPixelSelection(KisPixelSelectionSP pixelSelection);
    void setShapeSelection(KisSelectionComponent* shapeSelection);

    void updateProjection();
    void updateProjection(const QRect& r);

private:

    // We don't want these methods to be used on selections:

    QRect extent() const
        {
            return KisPaintDevice::extent();
        }


    QRect exactBounds() const
        {
            return KisPaintDevice::exactBounds();
        }


private:

    // XXX: Move to Private class!
    KisPaintDeviceWSP m_parentPaintDevice;
    bool m_interestedInDirtyness;
    bool m_hasPixelSelection;
    bool m_hasShapeSelection;
    KisPixelSelectionSP m_pixelSelection;
    KisSelectionComponent* m_shapeSelection;
};

#endif // KIS_SELECTION_H_
