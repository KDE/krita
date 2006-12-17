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

#ifndef KOGROUPSHAPESCOMMAND_H
#define KOGROUPSHAPESCOMMAND_H

#include <koffice_export.h>

#include <QList>
#include <kcommand.h>

class KoShape;
class KoShapeGroup;
class KoShapeContainer;

/// The undo / redo command for grouping shapes
class FLAKE_EXPORT KoGroupShapesCommand : public KCommand {
public:
    /**
     * Command to group a set of shapes into a predefined container.
     * @param container the container to group the shapes under.
     * @param shapes a list of all the shapes that should be grouped.
     * @param clipped a list of the same length as the shapes list with one bool for each shape.
     *      See KoShapeContainer::childClipped()
     */
    KoGroupShapesCommand(KoShapeContainer *container, QList<KoShape *> shapes, QList<bool> clipped);
    /**
     * Command to group a set of shapes into a predefined container.
     * Convenience constructor since KoShapeGroup does not allow clipping.
     * @param container the group to group the shapes under.
     * @param shapes a list of all the shapes that should be grouped.
     */
    KoGroupShapesCommand(KoShapeGroup *container, QList<KoShape *> shapes);
    virtual ~KoGroupShapesCommand() { };
    /// execute the command
    virtual void execute ();
    /// revert the actions done in execute
    virtual void unexecute ();
    /// return the name of this command
    virtual QString name () const;

protected:
    KoGroupShapesCommand(); ///< protected constructor for child classes
    QList<KoShape*> m_shapes; ///<list of shapes to be grouped
    QList<bool> m_clipped; ///< list of booleas to specify the shape of the same index to eb clipped
    KoShapeContainer *m_container; ///< the container where the grouping should be for.
    QList<KoShapeContainer*> m_oldParents; ///< the old parents of the shapes
};

#endif
