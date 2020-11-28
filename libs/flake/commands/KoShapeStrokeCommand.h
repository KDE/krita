/* This file is part of the KDE project
 *
 * SPDX-FileCopyrightText: 2006 Thomas Zander <zander@kde.org>
 * SPDX-FileCopyrightText: 2006-2008 Jan Hambrecht <jaham@gmx.net>
 * SPDX-FileCopyrightText: 2012 Inge Wallin <inge@lysator.liu.se>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KOSHAPESTROKECOMMAND_H
#define KOSHAPESTROKECOMMAND_H

#include "kritaflake_export.h"

#include <KoFlakeTypes.h>
#include <kundo2command.h>
#include <QList>

class KoShape;
class KoShapeStrokeModel;

/// The undo / redo command for setting the shape stroke
class KRITAFLAKE_EXPORT KoShapeStrokeCommand : public KUndo2Command
{
public:
    /**
     * Command to set a new shape stroke.
     * @param shapes a set of all the shapes that should get the new stroke.
     * @param stroke the new stroke, the same for all given shapes
     * @param parent the parent command used for macro commands
     */
    KoShapeStrokeCommand(const QList<KoShape*> &shapes, KoShapeStrokeModelSP stroke, KUndo2Command *parent = 0);

    /**
     * Command to set new shape strokes.
     * @param shapes a set of all the shapes that should get a new stroke.
     * @param strokes the new strokes, one for each shape
     * @param parent the parent command used for macro commands
     */
    KoShapeStrokeCommand(const QList<KoShape*> &shapes, const QList<KoShapeStrokeModelSP> &strokes, KUndo2Command *parent = 0);

    /**
     * Command to set a new shape stroke.
     * @param shape a single shape that should get the new stroke.
     * @param stroke the new stroke
     * @param parent the parent command used for macro commands
     */
    KoShapeStrokeCommand(KoShape* shape, KoShapeStrokeModelSP stroke, KUndo2Command *parent = 0);

    ~KoShapeStrokeCommand() override;
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
