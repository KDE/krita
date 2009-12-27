/* This file is part of the KDE project
 * Copyright (C) 2006 Thomas Zander <zander@kde.org>
 * Copyright (C) 2006,2008 Jan Hambrecht <jaham@gmx.net>
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

#ifndef KOSHAPEBACKGROUNDCOMMAND_H
#define KOSHAPEBACKGROUNDCOMMAND_H

#include "flake_export.h"

#include <QUndoCommand>
#include <QList>
#include <QBrush>

class KoShape;
class KoShapeBackground;

/// The undo / redo command for setting the shape background
class FLAKE_EXPORT KoShapeBackgroundCommand : public QUndoCommand
{
public:
    /**
     * Command to set a new shape background.
     * @param shapes a set of all the shapes that should get the new background.
     * @param fill the new shape background
     * @param parent the parent command used for macro commands
     */
    KoShapeBackgroundCommand(const QList<KoShape*> &shapes, KoShapeBackground *fill, QUndoCommand *parent = 0);

    /**
    * Command to set a new shape background.
    * @param shape a single shape that should get the new background.
    * @param fill the new shape background
    * @param parent the parent command used for macro commands
    */
    KoShapeBackgroundCommand(KoShape *shape, KoShapeBackground *fill, QUndoCommand *parent = 0);

    /**
     * Command to set new shape backgrounds.
     * @param shapes a set of all the shapes that should get a new background.
     * @param fills the new backgrounds, one for each shape
     * @param parent the parent command used for macro commands
     */
    KoShapeBackgroundCommand(const QList<KoShape*> &shapes, const QList<KoShapeBackground*> &fills, QUndoCommand *parent = 0);

    virtual ~KoShapeBackgroundCommand();
    /// redo the command
    void redo();
    /// revert the actions done in redo
    void undo();
private:
    class Private;
    Private * const d;
};

#endif
