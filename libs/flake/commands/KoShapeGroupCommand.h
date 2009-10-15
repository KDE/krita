/* This file is part of the KDE project
 * Copyright (C) 2006 Thomas Zander <zander@kde.org>
 * Copyright (C) 2007 Jan Hambrecht <jaham@gmx.net>
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

#ifndef KOSHAPEGROUPCOMMAND_H
#define KOSHAPEGROUPCOMMAND_H

#include "flake_export.h"

#include <QList>
#include <QUndoCommand>
#include <QtGui/QMatrix>

class KoShape;
class KoShapeGroup;
class KoShapeContainer;

/// The undo / redo command for grouping shapes
class FLAKE_EXPORT KoShapeGroupCommand : public QUndoCommand
{
public:
    /**
     * Create command to group a set of shapes into a predefined container.
     * This used the KoShapeGroupCommand( KoShapeGroup *container, QList<KoShape *> shapes, QUndoCommand *parent = 0 );
     * constructor.
     * The createCommand will make sure that the group will have the z-index and the parent of the top most shape in the group.
     * 
     * @param container the group to group the shapes under.
     * @param parent the parent command if the resulting command is a compound undo command.
     * @param shapes a list of all the shapes that should be grouped.
     */
    static KoShapeGroupCommand * createCommand(KoShapeGroup *container, QList<KoShape *> shapes, QUndoCommand *parent = 0);

    /**
     * Command to group a set of shapes into a predefined container.
     * @param container the container to group the shapes under.
     * @param shapes a list of all the shapes that should be grouped.
     * @param clipped a list of the same length as the shapes list with one bool for each shape.
     *      See KoShapeContainer::childClipped()
     * @param parent the parent command used for macro commands
     */
    KoShapeGroupCommand(KoShapeContainer *container, QList<KoShape *> shapes, QList<bool> clipped,
                        QUndoCommand *parent = 0);
    /**
     * Command to group a set of shapes into a predefined container.
     * Convenience constructor since KoShapeGroup does not allow clipping.
     * @param container the group to group the shapes under.
     * @param parent the parent command if the resulting command is a compound undo command.
     * @param shapes a list of all the shapes that should be grouped.
     */
    KoShapeGroupCommand(KoShapeGroup *container, QList<KoShape *> shapes, QUndoCommand *parent = 0);
    virtual ~KoShapeGroupCommand() { }
    /// redo the command
    virtual void redo();
    /// revert the actions done in redo
    virtual void undo();

protected:
    KoShapeGroupCommand(QUndoCommand* parent = 0); ///< protected constructor for child classes
    QList<KoShape*> m_shapes; ///<list of shapes to be grouped
    QList<bool> m_clipped; ///< list of booleas to specify the shape of the same index to eb clipped
    KoShapeContainer *m_container; ///< the container where the grouping should be for.
    QList<KoShapeContainer*> m_oldParents; ///< the old parents of the shapes
    QList<bool> m_oldClipped; ///< if the shape was clipped in the old parent
    QList<int> m_oldZIndex; ///< the old z-index of the shapes

private:
    void init();
    QRectF containerBoundingRect();
};

#endif
