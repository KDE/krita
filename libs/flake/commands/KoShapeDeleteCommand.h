/* This file is part of the KDE project
 * Copyright (C) 2006 Thomas Zander <zander@kde.org>
 * Copyright (C) 2006 Jan Hambrecht <jaham@gmx.net>
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

#ifndef KOSHAPEDELETECOMMAND_H
#define KOSHAPEDELETECOMMAND_H

#include <koffice_export.h>
#include <QUndoCommand>
#include <QList>

class KoShape;
class KoShapeContainer;
class KoShapeControllerBase;

/// The undo / redo command for deleting shapes
class FLAKE_EXPORT KoShapeDeleteCommand : public QUndoCommand {
public:
    /**
     * Command to delete a single shape by means of a shape controller.
     * @param controller the controller to used for deleting.
     * @param shape a single shape that should be deleted.
     * @param parent the parent command used for macro commands
     */
    KoShapeDeleteCommand( KoShapeControllerBase *controller, KoShape *shape, QUndoCommand *parent = 0 );
    /**
     * Command to delete a set of shapes by means of a shape controller.
     * @param controller the controller to used for deleting.
     * @param shapes a set of all the shapes that should be deleted.
     * @param parent the parent command used for macro commands
     */
    KoShapeDeleteCommand( KoShapeControllerBase *controller, const QList<KoShape*> &shapes, QUndoCommand *parent = 0 );
    virtual ~KoShapeDeleteCommand();
    /// redo the command
    void redo ();
    /// revert the actions done in redo
    void undo ();
private:
    KoShapeControllerBase *m_controller; ///< the shape controller to use for removing/readding
    QList<KoShape*> m_shapes; ///< the list of shapes to delete
    QList<KoShapeContainer*> m_oldParents; ///< the old parents of the shapes
    bool m_deleteShapes;  ///< shows if shapes should be deleted when deleting the command
};

#endif
