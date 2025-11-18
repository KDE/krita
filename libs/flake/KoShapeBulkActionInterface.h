/* This file is part of the KDE project
   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KOSHAPEBULKACTIONINTERFACE_H
#define KOSHAPEBULKACTIONINTERFACE_H

#include <QRect>

/**
 * @brief Interface for bulk actions on shapes
 *
 * This interface provides a mechanism for executing long-running actions
 * on shapes that will result in multiple shapeChanged() callbacks.
 *
 * When a shape enters an "bulk action" state, the following semantics apply:
 *
 * 1) All methods of KoShape class except paint() and outline() are considered
 *    valid throughout any moment of the shape being in this state, including
 *    outlineRect() and boundingRect().
 *
 * 2) Painting-related methods cannot be used in this state, as the shape might
 *    need to perform heavy calculations for them.
 *
 * 3) outline() may (or may not) also become invalid in this state.
 *
 * 4) Any non-KoShape specific methods may be invalid in this state.
 */
struct KoShapeBulkActionInterface
{
    virtual ~KoShapeBulkActionInterface() = default;

    /**
     * Called by the GUI code when it wants to execute some kind
     * of a long action, which will result in multiple shapeChanged()
     * callbacks to be delivered.
     *
     * When started, the shape enters into an "bulk action" state
     * as described in the class documentation.
     */
    virtual void startBulkAction() = 0;

    /**
     * Ends bulk action state and performs all calculation-heavy
     * operations over the new state. Returns a rectangle in absolute
     * coordinates that should be updated to make these changes visible
     * to the user.
     */
    virtual QRectF endBulkAction() = 0;
};

#endif // KOSHAPEBULKACTIONINTERFACE_H
