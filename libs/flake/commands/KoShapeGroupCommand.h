/* This file is part of the KDE project
 * Copyright (C) 2006,2010 Thomas Zander <zander@kde.org>
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

#include "kritaflake_export.h"

#include <QList>
#include <QScopedPointer>
#include <kundo2command.h>

class KoShape;
class KoShapeGroup;
class KoShapeContainer;
class KoShapeGroupCommandPrivate;

/// The undo / redo command for grouping shapes
class KRITAFLAKE_EXPORT KoShapeGroupCommand : public KUndo2Command
{
public:
    /**
     * Create command to group a set of shapes into a predefined container.
     * This uses the KoShapeGroupCommand(KoShapeGroup *container, const QList<KoShape *> &shapes, KUndo2Command *parent = 0);
     * constructor.
     * The createCommand will make sure that the group will have the z-index and the parent of the top most shape in the group.
     *
     * @param container the group to group the shapes under.
     * @param shapes a list of all the shapes that should be grouped.
     * @param shouldNormalize whether the shapes should be normalized
     */
    static KoShapeGroupCommand *createCommand(KoShapeContainer *container, const QList<KoShape *> &shapes, bool shouldNormalize = false);

        /**
     * Command to group a set of shapes into a predefined container.
     * @param container the container to group the shapes under.
     * @param shapes a list of all the shapes that should be grouped.
     * @param shouldNormalize shows whether the shapes should be normalized by the container
     * @param parent the parent command used for macro commands
     */
    KoShapeGroupCommand(KoShapeContainer *container, const QList<KoShape *> &shapes, bool shouldNormalize, KUndo2Command *parent = 0);

    /**
     * Command to group a set of shapes into a predefined container.
     * Convenience constructor since KoShapeGroup does not allow clipping.
     * @param container the group to group the shapes under.
     * @param parent the parent command if the resulting command is a compound undo command.
     * @param shapes a list of all the shapes that should be grouped.
     */
    KoShapeGroupCommand(KoShapeContainer *container, const QList<KoShape *> &shapes, KUndo2Command *parent = 0);
    ~KoShapeGroupCommand() override;

    /// redo the command
    void redo() override;

    /// revert the actions done in redo
    void undo() override;

protected:
    const QScopedPointer<KoShapeGroupCommandPrivate> d;
};

#endif
