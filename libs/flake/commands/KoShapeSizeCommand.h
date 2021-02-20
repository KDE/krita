/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2006 Thomas Zander <zander@kde.org>
 * SPDX-FileCopyrightText: 2006 Jan Hambrecht <jaham@gmx.net>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KOSHAPESIZECOMMAND_H
#define KOSHAPESIZECOMMAND_H

#include "kritaflake_export.h"

#include <kundo2command.h>
#include <QList>

class KoShape;

/// The undo / redo command for shape sizing.
class KRITAFLAKE_EXPORT KoShapeSizeCommand : public KUndo2Command
{
public:
    /**
     * The undo / redo command for shape sizing.
     * @param shapes all the shapes that will be rezised at the same time
     * @param previousSizes the old sizes; in a list with a member for each shape
     * @param newSizes the new sizes; in a list with a member for each shape
     * @param parent the parent command used for macro commands
     */
    KoShapeSizeCommand(const QList<KoShape*> &shapes, const QList<QSizeF> &previousSizes,
            const QList<QSizeF> &newSizes, KUndo2Command *parent = 0);
    ~KoShapeSizeCommand() override;

    /// redo the command
    void redo() override;
    /// revert the actions done in redo
    void undo() override;

private:
    class Private;
    Private * const d;
};

#endif
