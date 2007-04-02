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

#include "kis_types.h"
#include "kis_paint_device.h"
#include "kis_mask.h"

#include <krita_export.h>


enum enumSelectionMode {
    SELECTION_ADD,
    SELECTION_SUBTRACT
};

/**
 * KisSelection contains a byte-map representation of a layer, where
 * the value of a byte signifies whether a corresponding pixel is selected, or not.
 *
 * NOTE: If you need to manually call emitSelectionChanged on the owner paint device
 *       of a selection. KisSelection does not emit any signals by itself because
 *       often you want to combine several actions in to perfom one operation and you
 *       do not want recomposition to happen all the time.
 */
class KRITAIMAGE_EXPORT KisSelection : public KisMask {

    typedef KisMask super;

public:
    /**
     * Create a new KisSelection
    * @param dev the parent paint device. The selection will never be bigger than the parent
     *              paint device.
     */
    KisSelection(KisPaintDeviceSP dev);

    /**
     * Create a new KisSelection. This selection will not have a parent paint device.
     */
    KisSelection();

    /**
     * Copy the selection
     */
    KisSelection(const KisSelection& rhs);

    virtual ~KisSelection();

    // Returns selectedness, or 0 if invalid coordinates
    quint8 selected(qint32 x, qint32 y) const;

    void setSelected(qint32 x, qint32 y, quint8 s);

    QImage maskImage() const;

    void select(QRect r);

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

    void paint(QImage img, qint32 x, qint32 y, qint32 w, qint32 h);
    void paint(QImage img, const QRect& scaledImageRect, const QSize& scaledImageSize, const QSize& imageSize);

    // if the parent layer is interested in keeping up to date with the dirtyness
    // of this layer, set to true
    void setInterestedInDirtyness(bool b) { m_dirty = b; }
    bool interestedInDirtyness() const { return m_dirty; }

    virtual void setDirty(const QRect & rc);
    virtual void setDirty();

private:
    void paintUniformSelectionRegion(QImage img, const QRect& imageRect, const QRegion& uniformRegion);


private:

    // We don't want these methods to be used on selections:
    void extent(qint32 &x, qint32 &y, qint32 &w, qint32 &h) const
        {
            KisPaintDevice::extent(x,y,w,h);
        }

    QRect extent() const { return KisPaintDevice::extent(); }

    void exactBounds(qint32 &x, qint32 &y, qint32 &w, qint32 &h) const
        {
            return KisPaintDevice::exactBounds(x,y,w,h);
        }

    QRect exactBounds() const
        {
            return KisPaintDevice::exactBounds();
        }

    QRegion region() const
        {
            return KisPaintDevice::region();
        }

private:
    KisPaintDeviceWSP m_parentPaintDevice;
    bool m_dirty;
};

#endif // KIS_SELECTION_H_
