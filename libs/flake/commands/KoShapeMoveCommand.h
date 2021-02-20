/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2006 Thomas Zander <zander@kde.org>
 * SPDX-FileCopyrightText: 2006 Jan Hambrecht <jaham@gmx.net>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KOSHAPEMOVECOMMAND_H
#define KOSHAPEMOVECOMMAND_H

#include "kritaflake_export.h"

#include <kundo2command.h>
#include <QList>
#include <QPointF>
#include <KoFlake.h>

class KoShape;

/// The undo / redo command for shape moving.
class KRITAFLAKE_EXPORT KoShapeMoveCommand : public KUndo2Command
{
public:
    /**
     * Constructor.
     * @param shapes the set of objects that are moved.
     * @param previousPositions the known set of previous positions for each of the objects.
     *  this list naturally must have the same amount of items as the shapes set.
     * @param newPositions the new positions for the shapes.
     *  this list naturally must have the same amount of items as the shapes set.
     * @param parent the parent command used for macro commands
     */
    KoShapeMoveCommand(const QList<KoShape*> &shapes, QList<QPointF> &previousPositions, QList<QPointF> &newPositions,
                       KoFlake::AnchorPosition anchor = KoFlake::Center, KUndo2Command *parent = 0);

    KoShapeMoveCommand(const QList<KoShape*> &shapes, const QPointF &offset, KUndo2Command *parent = 0);

    ~KoShapeMoveCommand() override;
    /// redo the command
    void redo() override;
    /// revert the actions done in redo
    void undo() override;

    int id() const override;
    bool mergeWith(const KUndo2Command *command) override;

private:
    class Private;
    Private * const d;
};

#endif
