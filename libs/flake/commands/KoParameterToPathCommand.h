/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2006, 2008 Jan Hambrecht <jaham@gmx.net>
 * SPDX-FileCopyrightText: 2006, 2007 Thorsten Zachmann <zachmann@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KOPARAMETERTOPATHCOMMAND_H
#define KOPARAMETERTOPATHCOMMAND_H

#include <kundo2command.h>
#include <QList>

#include "kritaflake_export.h"

class KoParameterShape;
class KoParameterToPathCommandPrivate;

/// The undo / redo command for changing a KoParameterShape into a KoPathShape
class KRITAFLAKE_EXPORT KoParameterToPathCommand : public KUndo2Command
{
public:
    /**
     * Constructor.
     * @param shape the shape this command works on
     * @param parent the parent command if this is a compound undo command.
     */
    explicit KoParameterToPathCommand(KoParameterShape *shape, KUndo2Command *parent = 0);
    /**
     * Constructor.
     * @param shapes the list of shapes this command works on
     * @param parent the parent command if this is a compound undo command.
     */
    explicit KoParameterToPathCommand(const QList<KoParameterShape*> &shapes, KUndo2Command *parent = 0);
    ~KoParameterToPathCommand() override;

    /// redo the command
    void redo() override;
    /// revert the actions done in redo
    void undo() override;
private:
    KoParameterToPathCommandPrivate * const d;
};

#endif // KOPARAMETERTOPATHCOMMAND_H
