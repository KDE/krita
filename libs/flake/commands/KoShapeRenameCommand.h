/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2008 Thorsten Zachmann <zachmann@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KOSHAPERENAMECOMMAND_H
#define KOSHAPERENAMECOMMAND_H

#include "kritaflake_export.h"
#include <kundo2command.h>

class QString;
class KoShape;

/// API docs go here
class KRITAFLAKE_EXPORT KoShapeRenameCommand : public KUndo2Command
{
public:
    KoShapeRenameCommand(KoShape *shape, const QString &newName, KUndo2Command *parent = 0);
    ~KoShapeRenameCommand() override;

    /// redo the command
    void redo() override;
    /// revert the actions done in redo
    void undo() override;

private:
    class Private;
    Private * const d;
};

#endif /* KOSHAPERENAMECOMMAND_H */
