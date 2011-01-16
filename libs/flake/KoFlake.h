/* This file is part of the KDE project
 * Copyright (C) 2006 Thomas Zander <zander@kde.org>
 * Copyright (C) 2008 Jan Hambrecht <jaham@gmx.net>
 * Copyright (C) 2010 Thorsten Zachmann <zachmann@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */
#ifndef KOFLAKE_H
#define KOFLAKE_H

#include "flake_export.h"

#include <QtGui/QBrush>

class KoShape;
class QGradient;
class KoShapeBackground;

/**
 * Flake reference
 */
namespace KoFlake
{
    /// the selection type for KoSelection::selectedObjects()
    enum SelectionType {
        FullSelection,      ///< Create a list of all user-shapes in the selection. This excludes KoShapeGroup grouping objects that may be selected.
        StrippedSelection,  ///< Create a stripped list, without children if the container is also in the list.
        TopLevelSelection   ///< Create a list, much like the StrippedSelection, but have the KoShapeGroup instead of all of its children if one is selected.
    };

    /// Enum determining which handle is meant, used in KoInteractionTool
    enum SelectionHandle {
        TopMiddleHandle,    ///< The handle that is at the top - center of a selection
        TopRightHandle,     ///< The handle that is at the top - right of  a selection
        RightMiddleHandle,  ///< The handle that is at the right - center of a selection
        BottomRightHandle,  ///< The handle that is at the bottom right of a selection
        BottomMiddleHandle, ///< The handle that is at the bottom center of a selection
        BottomLeftHandle,   ///< The handle that is at the bottom left of a selection
        LeftMiddleHandle,   ///< The handle that is at the left center of a selection
        TopLeftHandle,      ///< The handle that is at the top left of a selection
        NoHandle            ///< Value to indicate no handle
    };

    /**
     * Used to change the behavior of KoShapeManager::shapeAt()
     */
    enum ShapeSelection {
        Selected,   ///< return the first selected with the highest z-ordering (i.e. on top).
        Unselected, ///< return the first unselected on top.
        NextUnselected, ///< return the first unselected directly under a selected shape, or the top most one if nothing is selected.
        ShapeOnTop  ///< return the shape highest z-ordering, regardless of selection.
    };

    /// position. See KoShape::absolutePosition()
    enum Position {
        TopLeftCorner, ///< the top left corner
        TopRightCorner, ///< the top right corner
        BottomLeftCorner, ///< the bottom left corner
        BottomRightCorner, ///< the bottom right corner
        CenteredPosition ///< the centred corner
    };

    /**
     * Used to see which style type is active
     */
    enum StyleType {
        Background, ///< the background / fill style is active
        Foreground  ///< the foreground / border style is active
    };

    /// clones the given gradient
    FLAKE_EXPORT QGradient *cloneGradient(const QGradient *gradient);

    /**
     * Convert absolute to relative position
     *
     * @param absolute absolute position
     * @param size for which the relative position needs to be calculated
     *
     * @return relative position
     */
    FLAKE_EXPORT QPointF toRelative(const QPointF &absolute, const QSizeF &size);

    /**
     * Convert relative size to absolute size
     *
     * @param relative relative position
     * @param size for which the absolute position needs to be calculated
     *
     * @return absolute position
     */
    FLAKE_EXPORT QPointF toAbsolute(const QPointF &relative, const QSizeF &size);
}

#endif
