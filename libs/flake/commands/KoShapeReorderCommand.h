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

#ifndef KOSHAPEREORDEROMMAND_H
#define KOSHAPEREORDEROMMAND_H

#include "KoSelection.h"

#include <koffice_export.h>

#include <QUndoCommand>
#include <QList>

class KoShape;
class KoShapeManager;

/// This command allows you to change the zIndex of a number of shapes.
class FLAKE_EXPORT KoShapeReorderCommand : public QUndoCommand {
public:
    /**
     * Constructor.
     * @param shapes the set of objects that are moved.
     * @param newIndexes the new indexes for the shapes.
     *  this list naturally must have the same amount of items as the shapes set.
     * @param parent the parent command used for macro commands
     */
    KoShapeReorderCommand(const QList<KoShape*> &shapes, QList<int> &newIndexes, QUndoCommand *parent = 0);

    enum MoveShapeType  {
        RaiseShape,
        LowerShape,
        BringToFront,
        SendToBack
    };
    static KoShapeReorderCommand *createCommand(const QList<KoShape*> &shapes, KoShapeManager *manager,
            MoveShapeType move, QUndoCommand *parent = 0);

    /// redo the command
    void redo ();
    /// revert the actions done in redo
    void undo ();

private:
    QList<KoShape*> m_shapes;
    QList<int> m_previousIndexes, m_newIndexes;
};

#endif
