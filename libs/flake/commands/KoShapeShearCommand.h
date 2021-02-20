/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2006 Thomas Zander <zander@kde.org>
 * SPDX-FileCopyrightText: 2006 Jan Hambrecht <jaham@gmx.net>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KOSHAPESHEARCOMMAND_H
#define KOSHAPESHEARCOMMAND_H

#include "kritaflake_export.h"

#include <kundo2command.h>
#include <QList>

class KoShape;
class KoShapeShearCommandPrivate;

/// The undo / redo command for shape shearing.
class KRITAFLAKE_EXPORT KoShapeShearCommand : public KUndo2Command
{
public:
    /**
     * Command to rotate a selection of shapes.  Note that it just alters the rotated
     * property of those shapes, and nothing more.
     * @param shapes all the shapes that should be rotated
     * @param previousShearXs a list with the same amount of items as shapes with the
     *        old shearX values
     * @param previousShearYs a list with the same amount of items as shapes with the
     *        old shearY values
     * @param newShearXs a list with the same amount of items as shapes with the new values.
     * @param newShearYs a list with the same amount of items as shapes with the new values.
     * @param parent the parent command used for macro commands
     */
    KoShapeShearCommand(const QList<KoShape*> &shapes, const QList<qreal> &previousShearXs, const QList<qreal> &previousShearYs, const QList<qreal> &newShearXs, const QList<qreal> &newShearYs, KUndo2Command *parent = 0);

    ~KoShapeShearCommand() override;
    /// redo the command
    void redo() override;
    /// revert the actions done in redo
    void undo() override;

private:
    KoShapeShearCommandPrivate * const d;
};

#endif
