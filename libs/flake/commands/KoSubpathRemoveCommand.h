/* This file is part of the KDE project
 * Copyright (C) 2006 Jan Hambrecht <jaham@gmx.net>
 * Copyright (C) 2006,2007 Thorsten Zachmann <zachmann@kde.org>
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

#ifndef KOSUBPATHREMOVECOMMAND_H
#define KOSUBPATHREMOVECOMMAND_H

#include <QUndoCommand>
#include "flake_export.h"
#include "KoPathPointData.h"

#include "KoPathPoint.h"

/// The undo / redo command for removing a subpath
class FLAKE_TEST_EXPORT KoSubpathRemoveCommand : public QUndoCommand
{
public:
    /**
     * Create a new command to remove a subpath.
     * @param pathShape the shape to work on.
     * @param subpathIndex the index. See KoPathShape::removeSubpath()
     * @param parent the parent command if the resulting command is a compound undo command.
     */
    KoSubpathRemoveCommand(KoPathShape *pathShape, int subpathIndex, QUndoCommand *parent = 0);
    ~KoSubpathRemoveCommand();

    /// redo the command
    void redo();
    /// revert the actions done in redo
    void undo();

private:
    KoPathShape *m_pathShape;
    int m_subpathIndex;
    KoSubpath *m_subpath;
};

#endif // KOSUBPATHREMOVECOMMAND_H
