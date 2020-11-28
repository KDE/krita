/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2007 Jan Hambrecht <jaham@gmx.net>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KOPATHFILLRULECOMMAND_H
#define KOPATHFILLRULECOMMAND_H

#include "kritaflake_export.h"

#include <kundo2command.h>
#include <QList>

class KoPathShape;

/// The undo / redo command for setting the fill rule of a path shape
class KRITAFLAKE_EXPORT KoPathFillRuleCommand : public KUndo2Command
{
public:
    /**
     * Command to set a new shape background.
     * @param shapes a set of all the path shapes that should get the new fill rule
     * @param fillRule the new fill rule
     * @param parent the parent command used for macro commands
     */
    KoPathFillRuleCommand(const QList<KoPathShape*> &shapes, Qt::FillRule fillRule, KUndo2Command *parent = 0);
    ~KoPathFillRuleCommand() override;
    /// redo the command
    void redo() override;
    /// revert the actions done in redo
    void undo() override;
private:
    class Private;
    Private * const d;
};

#endif // KOPATHFILLRULECOMMAND_H
