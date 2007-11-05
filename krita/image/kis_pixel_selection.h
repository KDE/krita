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
#ifndef KIS_PIXEL_SELECTION_H_
#define KIS_PIXEL_SELECTION_H_

#include <QRect>
#include <QPainterPath>

#include <kicon.h>

#include "kis_types.h"
#include "kis_paint_device.h"
#include "kis_selection_component.h"

#include <krita_export.h>

/**
 * KisPixelSelection contains a byte-map representation of a layer, where
 * the value of a byte signifies whether a corresponding pixel is selected, or not.
 *
 * NOTE: If you need to manually call emitSelectionChanged on the owner paint device
 *       of a selection. KisPixelSelection does not emit any signals by itself because
 *       often you want to combine several actions in to perfom one operation and you
 *       do not want recomposition to happen all the time.
 */
class KRITAIMAGE_EXPORT KisPixelSelection : public KisPaintDevice, public KisSelectionComponent {

public:


    /**
     * Create a new KisPixelSelection. This selection will not have a
     * parent paint device.
     */
    KisPixelSelection();

    /**
     * Create a new KisPixelSelection. The selection will never be
     * bigger than the parent paint device. This constructor will not
     * set the newly created pixel selection on the parent paintdevice.
     *
     * The parent paint device can be the paintDevice of a paint
     * layer. If the selection belongs to a paint layer, both the
     * selection and the paint device will have the paint layer set as
     * parent layer.
     *
     * @param dev the parent paint device.
     */
    KisPixelSelection(KisPaintDeviceSP dev);


    /**
     * Create a new KisPixelSelection from the given mask. The
     * selection will share its pixel data with the mask.
     */
    KisPixelSelection( KisPaintDeviceSP parent, KisMaskSP mask );


    /**
     * Copy the selection
     */
    KisPixelSelection(const KisPixelSelection& rhs);

    virtual ~KisPixelSelection();

    // Returns selectedness, or 0 if invalid coordinates
    quint8 selected(qint32 x, qint32 y) const;

    void setSelected(qint32 x, qint32 y, quint8 s);

    QImage maskImage( KisImageSP image ) const;

    void select(const QRect & r, quint8 selectedness = MAX_SELECTED);

    /**
     * Apply a selection to the selection using the specified selection mode
     * Note: SELECTION_REPLACE will be treated as SELECTION_ADD
     */
    void applySelection(KisPixelSelectionSP selection, selectionAction action);

    /** Add a selection */
    void addSelection(KisPixelSelectionSP selection);

    /** Subtracts a selection */
    void subtractSelection(KisPixelSelectionSP selection);

    /** Intersects a selection */
    void intersectSelection(KisPixelSelectionSP selection);

    void invert();

    void clear(QRect r);

    void clear();

    /// Tests if the the rect is totally outside the selection
    bool isTotallyUnselected(QRect r) const;

    /**
     * Tests if the the rect is totally outside the selection, but uses selectedRect
     * instead of selectedRect, and this is faster (but might deliver false positives!)
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

    /**
     * If the parent layer is interested in keeping up to date with
     * the dirtyness of this layer, set to true
     */
    void setInterestedInDirtyness(bool b);

    /**
     * returns true if the parent layer is interested in keeping up to
     * date with the dirtyness of this layer.
     */
    bool interestedInDirtyness() const;

    void setDirty(const QRect & rc);
    void setDirty();

    QVector<QPolygon> outline();

    virtual void renderToProjection(KisSelection* projection);
    virtual void renderToProjection(KisSelection* projection, const QRect& r);


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


    enum EdgeType
    {
        TopEdge = 1, LeftEdge = 2, BottomEdge = 3, RightEdge = 0, NoEdge = 4
    };

    bool isOutlineEdge(EdgeType edge, qint32 row, qint32 col, quint8* buffer, qint32 width, qint32 height);

    EdgeType nextEdge(EdgeType edge) { return edge == NoEdge ? edge : static_cast<EdgeType>((edge + 1) % 4); }

    void nextOutlineEdge(EdgeType *edge, qint32 *row, qint32 *col, quint8* buffer, qint32 bufWidth, qint32 bufHeight);

    void appendCoordinate(QPolygon * path, int x, int y, EdgeType edge);

private:

    struct Private;
    Private * const m_d;
};

#endif // KIS_PIXEL_SELECTION_H_
