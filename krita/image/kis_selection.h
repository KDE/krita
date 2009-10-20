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

#include <kicon.h>

#include "kis_types.h"
#include "kis_paint_device.h"
#include "kis_mask.h"

#include "krita_export.h"

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
 * KisSelection is a paint device that is constructed out of several components.
 * such as a pixel selection, a vector selection or something else we haven't thought
 * up yet.
 *
 * Every pixel in the paint device can indicate a degree of selectedness, varying
 * between MIN_SELECTED and MAX_SELECTED.
 *
 * The paint device itself is only a projection:
 * you can read from it, but not write to it. You need to keep track of the need
 * for updating the projection yourself: there is no automatic updating after changing
 * the contents of one or more of the selection components.
 *
 * XXX: optimize: if there is only a PixelSelection, use that as projection.
 */
class KRITAIMAGE_EXPORT KisSelection : public KisPaintDevice
{

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
    KisSelection(KisPaintDeviceSP parent, KisMaskSP mask);

    /**
     * Create a new KisSelection. This selection will not have a
     * parent paint device.
     */
    KisSelection();

    /**
     * Copy the selection. The selection components are copied, too.
     */
    KisSelection(const KisSelection& rhs);

    /**
     * Delete the selection. The shape selection component is deleted, the
     * pixel selection component is contained in a shared pointer, so that
     * may still be valid.
     *
     * XXX: Make this sane!
     */
    virtual ~KisSelection();

    /**
     * Returns selectedness of the specified pixel, or 0 if invalid
     * coordinates. The projection is not updated before determinging selectedness.
     */
    quint8 selected(qint32 x, qint32 y) const;

    /**
     * Clear the selection. This invalidates the projection. It does not clear
     * the selection components.
     * Afterwards, you can call updateProjection to recompute the selection.
     */
    void clear();

    /**
     * Clear the specified rect in the selection projection. This invalidates
     * the projection. It does not clear the selection components.
     * Afterwards, you can call updateProjection to recompute
     * the selection.
     */
    void clear(const QRect& r);

    /**
     * Tests if the the rect is totally outside the selection.
     */
    bool isTotallyUnselected(const QRect & r) const;

    /**
     * Tests if the the rect is totally outside the selection, but
     * uses selectedRect instead of selectedExactRect, and this is faster
     * (but might deliver false positives!)
     */
    bool isProbablyTotallyUnselected(const QRect & r) const;

    /**
     * Rough, but fastish way of determining the area
     * of the tiles used by the selection projection.
     */
    QRect selectedRect() const;

    /**
     * Slow, but exact way of determining the rectangle
     * that encloses the selection projection.
     */
    QRect selectedExactRect() const;

    /**
     * If the parent paint device is interested in keeping up to date
     * with the dirtyness of this selection, set to true
     */
    void setInterestedInDirtyness(bool b);

    /**
     * returns true if the parent paint device is interested in
     * keeping up with the dirtyness of the selection.
     */
    bool interestedInDirtyness() const;

    virtual void setDirty(const QRect & rc);
    virtual void setDirty(const QRegion & region);
    virtual void setDirty();

    bool hasPixelSelection() const;
    bool hasShapeSelection() const;

    /**
     * return the pixel selection component of this selection or zero
     * if hasPixelSelection() returns false.
     */
    KisPixelSelectionSP pixelSelection() const;

    /**
     * return the vector selection component of this selection or zero
     * if hasShapeSelection() returns false.
     */
    KisSelectionComponent* shapeSelection() const;

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

    void setDeselected(bool deselected);
    bool isDeselected();

private:

    // We don't want these methods to be used on selections:

    QRect extent() const {
        return KisPaintDevice::extent();
    }


    QRect exactBounds() const {
        return KisPaintDevice::exactBounds();
    }


private:

    struct Private;
    Private * const m_d;
};

#endif // KIS_SELECTION_H_
