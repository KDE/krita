/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2006 Thomas Zander <zander@kde.org>
 * SPDX-FileCopyrightText: 2006 Jan Hambrecht <jaham@gmx.net>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KOSHAPELOCKCOMMAND_H
#define KOSHAPELOCKCOMMAND_H

#include <kundo2command.h>
#include <QList>

class KoShape;

/// The undo / redo command to lock a set of shapes position and size
class KoShapeLockCommand : public KUndo2Command
{
public:
    /**
     * Command to lock a set of shapes position and size
     * @param shapes a set of shapes that should change lock state
     * @param oldLock list of old lock states the same length as @p shapes
     * @param newLock list of new lock states the same length as @p shapes
     * @param parent the parent command used for macro commands
     */
    KoShapeLockCommand(const QList<KoShape*> &shapes, const QList<bool> &oldLock, const QList<bool> &newLock,
                       KUndo2Command *parent = 0);
    ~KoShapeLockCommand() override;

    /// redo the command
    void redo() override;
    /// revert the actions done in redo
    void undo() override;

private:
    QList<KoShape*> m_shapes;    /// the shapes to set background for
    QList<bool> m_oldLock;       /// old lock states
    QList<bool> m_newLock;       /// new lock states
};

#endif
