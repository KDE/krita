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

#ifndef KOUNGROUPSHAPESCOMMAND_H
#define KOUNGROUPSHAPESCOMMAND_H

#include "commands/KoGroupShapesCommand.h"

#include <koffice_export.h>

#include <QUndoCommand>

/// The undo / redo command for ungrouping shapes
class FLAKE_EXPORT KoUngroupShapesCommand : public KoGroupShapesCommand {
public:
    /**
     * Command to ungroup a set of shapes from one parent container.
     * @param container the group to ungroup the shapes from.
     * @param shapes a list of all the shapes that should be ungrouped.
     * @param parent the parent command used for macro commands
     */
    KoUngroupShapesCommand(KoShapeContainer *container, QList<KoShape *> shapes, QUndoCommand *parent = 0);
    /// redo the command
    void redo ();
    /// revert the actions done in redo
    void undo ();
};

#endif
