/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2006 Thomas Zander <zander@kde.org>
 * SPDX-FileCopyrightText: 2008 Jan Hambrecht <jaham@gmx.net>
 * SPDX-FileCopyrightText: 2010 Thorsten Zachmann <zachmann@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */
#ifndef KOFLAKE_H
#define KOFLAKE_H

#include "kritaflake_export.h"

class QGradient;
class QRectF;
class QPointF;
class QSizeF;

class KoShape;
class QTransform;

#include <Qt>

/**
 * Flake reference
 */
namespace KoFlake
{
    enum FillVariant {
        Fill,
        StrokeFill
    };

    enum FillType {
        None,
        Solid,
        Gradient,
        Pattern,
        MeshGradient
    };

    enum MarkerPosition {
        StartMarker,
        MidMarker,
        EndMarker
    };

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

    /**
     * Used to see which style type is active
     */
    enum StyleType {
        Background, ///< the background / fill style is active
        Foreground  ///< the foreground / stroke style is active
    };

    enum AnchorPosition {
        TopLeft,
        Top,
        TopRight,
        Left,
        Center,
        Right,
        BottomLeft,
        Bottom,
        BottomRight,
        NoAnchor,
        NumAnchorPositions
    };

    KRITAFLAKE_EXPORT QPointF anchorToPoint(AnchorPosition anchor, const QRectF rect, bool *valid = 0);

    enum CanvasResource {
        HotPosition = 1410100299
    };

    /// clones the given gradient
    KRITAFLAKE_EXPORT QGradient *cloneGradient(const QGradient *gradient);

    KRITAFLAKE_EXPORT QGradient *mergeGradient(const QGradient *coordsSource, const QGradient *fillSource);

    /**
     * Convert absolute to relative position
     *
     * @param absolute absolute position
     * @param size for which the relative position needs to be calculated
     *
     * @return relative position
     */
    KRITAFLAKE_EXPORT QPointF toRelative(const QPointF &absolute, const QSizeF &size);

    /**
     * Convert relative size to absolute size
     *
     * @param relative relative position
     * @param size for which the absolute position needs to be calculated
     *
     * @return absolute position
     */
    KRITAFLAKE_EXPORT QPointF toAbsolute(const QPointF &relative, const QSizeF &size);

    KRITAFLAKE_EXPORT Qt::Orientation significantScaleOrientation(qreal scaleX, qreal scaleY);

    KRITAFLAKE_EXPORT void scaleShape(KoShape *shape, qreal scaleX, qreal scaleY,
                                      const QPointF &absoluteStillPoint,
                                      const QTransform &postScalingCoveringTransform);

    KRITAFLAKE_EXPORT void scaleShapeGlobal(KoShape *shape, qreal scaleX, qreal scaleY,
                          const QPointF &absoluteStillPoint);


    KRITAFLAKE_EXPORT void resizeShape(KoShape *shape, qreal scaleX, qreal scaleY,
                              const QPointF &absoluteStillPoint,
                              bool useGlobalMode);

    KRITAFLAKE_EXPORT void resizeShapeCommon(KoShape *shape, qreal scaleX, qreal scaleY,
                                       const QPointF &absoluteStillPoint,
                                       bool useGlobalMode,
                                       bool usePostScaling, const QTransform &postScalingCoveringTransform);
}

#endif
