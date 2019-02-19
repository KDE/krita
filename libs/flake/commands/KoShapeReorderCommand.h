/* This file is part of the KDE project
 * Copyright (C) 2006 Thomas Zander <zander@kde.org>
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

#ifndef KOSHAPEREORDERCOMMAND_H
#define KOSHAPEREORDERCOMMAND_H

#include "kritaflake_export.h"

#include <boost/operators.hpp>
#include <kundo2command.h>
#include <QList>

class KoShape;
class KoShapeManager;
class KoShapeReorderCommandPrivate;

/// This command allows you to change the zIndex of a number of shapes.
class KRITAFLAKE_EXPORT KoShapeReorderCommand : public KUndo2Command
{
public:
    struct KRITAFLAKE_EXPORT IndexedShape : boost::less_than_comparable<IndexedShape> {
        IndexedShape();
        IndexedShape(KoShape *_shape);

        bool operator<(const IndexedShape &rhs) const;

        int zIndex = 0;
        KoShape *shape = 0;
    };


public:
    /**
     * Constructor.
     * @param shapes the set of objects that are moved.
     * @param newIndexes the new indexes for the shapes.
     *  this list naturally must have the same amount of items as the shapes set.
     * @param parent the parent command used for macro commands
     */
    KoShapeReorderCommand(const QList<KoShape*> &shapes, QList<int> &newIndexes, KUndo2Command *parent = 0);
    KoShapeReorderCommand(const QList<IndexedShape> &shapes, KUndo2Command *parent = 0);
    ~KoShapeReorderCommand() override;

    /// An enum for defining what kind of reordering to use.
    enum MoveShapeType  {
        RaiseShape,     ///< raise the selected shape to the level that it is above the shape that is on top of it.
        LowerShape,     ///< Lower the selected shape to the level that it is below the shape that is below it.
        BringToFront,   ///< Raise the selected shape to be on top of all shapes.
        SendToBack      ///< Lower the selected shape to be below all other shapes.
    };

    /**
     * Create a new KoShapeReorderCommand by calculating the new indexes required to move the shapes
     * according to the move parameter.
     * @param shapes all the shapes that should be moved.
     * @param manager the shapeManager that contains all the shapes that could have their indexes changed.
     * @param move the moving type.
     * @param parent the parent command for grouping purposes.
     * @return command for reordering the shapes or 0 if no reordering happened
     */
    static KoShapeReorderCommand *createCommand(const QList<KoShape*> &shapes, KoShapeManager *manager,
            MoveShapeType move, KUndo2Command *parent = 0);

    /**
     * @brief mergeInShape adjust zIndex of all the \p shapes and \p newShape to
     *        avoid collisions between \p shapes and \p newShape.
     *
     *        Note1: \p newShape may or may not be contained in \p shapes, there
     *               is no difference.
     *        Note2: the collisions inside \p shapes are ignored. They are just
     *               adjusted to avoid collisions with \p newShape only
     * @param shapes list of shapes
     * @param newShape the new shape
     * @param parent the parent command for grouping purposes.
     * @return command for reordering the shapes or 0 if no reordering happened
     */
    static KoShapeReorderCommand *mergeInShape(QList<KoShape*> shapes, KoShape *newShape,
                                               KUndo2Command *parent = 0);

    /**
     * Recalculates the attached z-indexes of \p shapes so that all indexes go
     * strictly in ascending order and no shapes have repetitive indexes. The
     * physical order of the shapes in the array is not changed, on the indexes
     * in IndexedShape are corrected.
     */
    static
    QList<KoShapeReorderCommand::IndexedShape>
    homogenizeZIndexes(QList<IndexedShape> shapes);

    /**
     * Convenience version of homogenizeZIndexes() that removes all the IndexedShape
     * objects, which z-index didn't change during homogenization. In a result
     * you get a list that can be passed to KoShapeReorderCommand directly.
     */
    static
    QList<KoShapeReorderCommand::IndexedShape>
    homogenizeZIndexesLazy(QList<IndexedShape> shapes);

    /**
     * Put all the shapes in \p shapesAbove above the shapes in \p shapesBelow, adjusting their
     * z-index values.
     */
    static QList<IndexedShape> mergeDownShapes(QList<KoShape*> shapesBelow, QList<KoShape*> shapesAbove);

    /// redo the command
    void redo() override;
    /// revert the actions done in redo
    void undo() override;

private:
    KoShapeReorderCommandPrivate * const d;
};

KRITAFLAKE_EXPORT QDebug operator<<(QDebug dbg, const KoShapeReorderCommand::IndexedShape &indexedShape);

#endif
