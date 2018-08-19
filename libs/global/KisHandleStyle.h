/*
 *  Copyright (c) 2017 Dmitry Kazakov <dimula73@gmail.com>
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

#ifndef KISHANDLESTYLE_H
#define KISHANDLESTYLE_H

#include <QVector>
#include <QPen>
#include <QBrush>

#include "kritaglobal_export.h"


/**
 * A special class that defines a set of predefined styles for painting handles.
 * Please use static methods for requesting standard krita styles.
 */
class KRITAGLOBAL_EXPORT KisHandleStyle
{
public:

    /**
     * Default style that does *no* change to the painter. That is, the painter
     * will paint with its current pen and brush.
     */
    static KisHandleStyle& inheritStyle();

    /**
     * Main style. Used for showing a single selection or a boundary box of
     * multiple selected objects.
     */
    static KisHandleStyle& primarySelection();

    /**
     * Secondary style. Used for highlighting objects inside a mupltiple
     * selection.
     */
    static KisHandleStyle& secondarySelection();

    /**
     * Style for painting gradient handles
     */
    static KisHandleStyle& gradientHandles();

    /**
     * Style for painting linear gradient arrows
     */
    static KisHandleStyle& gradientArrows();

    /**
     * Same as primary style, but the handles are filled with red color to show
     * that the user is hovering them.
     */
    static KisHandleStyle& highlightedPrimaryHandles();

    /**
     * Same as primary style, but the handles are filled with red color to show
     * that the user is hovering them and the outline is solid
     */
    static KisHandleStyle& highlightedPrimaryHandlesWithSolidOutline();

    /**
     * Used for nodes, which control points are highlighted (the node itself
     * is not highlighted)
     */
    static KisHandleStyle& partiallyHighlightedPrimaryHandles();

    /**
     * Same as primary style, but the handles are filled with green color to show
     * that they are selected.
     */
    static KisHandleStyle& selectedPrimaryHandles();

    struct IterationStyle {
        IterationStyle() : isValid(false) {}
        IterationStyle(const QPen &pen, const QBrush &brush)
            : isValid(true),
              stylePair(pen, brush)
        {
        }

        bool isValid;
        QPair<QPen, QBrush> stylePair;
    };

    QVector<IterationStyle> handleIterations;
    QVector<IterationStyle> lineIterations;
};

#endif // KISHANDLESTYLE_H
