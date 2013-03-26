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
#include "krita_export.h"
#include "kis_paint_device.h"

enum SelectionMode {
    PIXEL_SELECTION,
    SHAPE_PROTECTION
};

enum SelectionAction {
    SELECTION_REPLACE,
    SELECTION_ADD,
    SELECTION_SUBTRACT,
    SELECTION_INTERSECT
};

#include "kis_pixel_selection.h"


class KisSelectionComponent;

/**
 * KisSelection is a composite object. It may contain an instance
 * of KisPixelSelection and a KisShapeSelection object. Both these
 * selections are merged into a projection of the KisSelection.
 *
 * Every pixel in the paint device can indicate a degree of selectedness, varying
 * between MIN_SELECTED and MAX_SELECTED.
 *
 * The projection() paint device itself is only a projection: you can
 * read from it, but not write to it. You need to keep track of
 * the need for updating the projection yourself: there is no
 * automatic updating after changing the contents of one or more
 * of the selection components.
 */
class KRITAIMAGE_EXPORT KisSelection : public KisShared
{

public:
    /**
     * Create a new KisSelection.
     *
     * @param defaultBounds defines the bounds of the selection when
     * Select All is initiated.
     */
    KisSelection(KisDefaultBoundsBaseSP defaultBounds = 0);

    /**
     * Copy the selection. The selection components are copied, too.
     */
    KisSelection(const KisSelection& rhs);

    /**
     * Delete the selection. The shape selection component is deleted, the
     * pixel selection component is contained in a shared pointer, so that
     * may still be valid.
     */
    virtual ~KisSelection();

    /**
     * The paint device of the pixel selection should report
     * about it's setDirty events to its parent. The creator
     * should set the parent manually if he wants to get the
     * signals
     */
    void setParentNode(KisNodeWSP node);

    bool hasPixelSelection() const;
    bool hasShapeSelection() const;

    QVector<QPolygon> outline() const;

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

    void setShapeSelection(KisSelectionComponent* shapeSelection);

    /**
     * Return the pixel selection associated with this selection or
     * create a new one if there is currently no pixel selection
     * component in this selection.
     */
    KisPixelSelectionSP getOrCreatePixelSelection();

    /**
     * Returns the projection of the selection. It may be the same
     * as pixel selection. You must read selection data from this
     * paint device only
     */
    KisPaintDeviceSP projection() const;

    /**
     * Updates the projection of the selection. You should call this
     * method after the every change of the selection components.
     * There is no automatic updates framework present
     */
    void updateProjection(const QRect& rect);
    void updateProjection();

    void setVisible(bool visible);
    bool isVisible();

    /**
     * Convenience functions. Just call the corresponding methods
     * of the underlying projection
     */
    bool isTotallyUnselected(const QRect & r) const;

    QRect selectedRect() const;

    /**
     * @brief Slow, but exact way of determining the rectangle
     * that encloses the selection.
     *
     * Default pixel of the selection device may vary and you would get wrong bounds.
     * selectedExactRect() handles all these cases.
     *
     */
    QRect selectedExactRect() const;

    void setX(qint32 x);
    void setY(qint32 y);

    qint32 x() const;
    qint32 y() const;

    void setDefaultBounds(KisDefaultBoundsBaseSP bounds);

    void clear();

    /**
     * @brief flatten creates a new pixel selection component from the shape selection
     * and throws away the shape selection. This has no effect if there is no
     * shape selection.
     */
    void flatten();

    KDE_DEPRECATED quint8 selected(qint32 x, qint32 y) const;
    KDE_DEPRECATED void setDirty(const QRect &rc = QRect());

private:
    friend class KisSelectionTest;
    friend class KisMaskTest;
    friend class KisAdjustmentLayerTest;
    KisNodeWSP parentNode() const;

private:

    struct Private;
    Private * const m_d;
};

#endif // KIS_SELECTION_H_
