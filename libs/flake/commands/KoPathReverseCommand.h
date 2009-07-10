/* This file is part of the KDE project
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

#ifndef KOPATHREVERSECOMMAND_H
#define KOPATHREVERSECOMMAND_H

#include <QUndoCommand>
#include <QList>
#include "flake_export.h"

class KoPathShape;

/// The undo / redo command for reversing path directions
class FLAKE_EXPORT KoPathReverseCommand : public QUndoCommand
{
public:
    /**
     * Command for reversing directions of a list of paths
     * @param paths the list of paths to reverse
     * @param parent the parent command used for macro commands
     */
    explicit KoPathReverseCommand(const QList<KoPathShape*> &paths, QUndoCommand *parent = 0);
    virtual ~KoPathReverseCommand();
    /// redo the command
    void redo();
    /// revert the actions done in redo
    void undo();
private:
    class Private;
    Private * const d;
};

#endif // KOPATHREVERSECOMMAND_H
