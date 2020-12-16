/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2007 Jan Hambrecht <jaham@gmx.net>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KOPATHREVERSECOMMAND_H
#define KOPATHREVERSECOMMAND_H

#include <kundo2command.h>
#include <QList>
#include "kritaflake_export.h"

class KoPathShape;

/// The undo / redo command for reversing path directions
class KRITAFLAKE_EXPORT KoPathReverseCommand : public KUndo2Command
{
public:
    /**
     * Command for reversing directions of a list of paths
     * @param paths the list of paths to reverse
     * @param parent the parent command used for macro commands
     */
    explicit KoPathReverseCommand(const QList<KoPathShape*> &paths, KUndo2Command *parent = 0);
    ~KoPathReverseCommand() override;
    /// redo the command
    void redo() override;
    /// revert the actions done in redo
    void undo() override;
private:
    class Private;
    Private * const d;
};

#endif // KOPATHREVERSECOMMAND_H
