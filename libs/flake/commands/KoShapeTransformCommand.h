/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2007 Jan Hambrecht <jaham@gmx.net>
 * SPDX-FileCopyrightText: 2007 Thomas Zander <zander@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KOSHAPETRANSFORMCOMMAND_H
#define KOSHAPETRANSFORMCOMMAND_H

#include "kritaflake_export.h"
#include <kundo2command.h>

class KoShape;
class QTransform;

/**
 * A command to transform a selection of shapes with the same transformation.
 */
class KRITAFLAKE_EXPORT KoShapeTransformCommand : public KUndo2Command
{
public:

    /**
     * A command to transform a selection of shapes to the new state.
     * Each shape passed has an old state and a new state of transformation passed in.
     * @param shapes all the shapes that should be transformed
     * @param oldState the old shapes transformations
     * @param newState the new shapes transformations
     * @see KoShape::transformation()
     * @see KoShape::setTransformation()
     */
    KoShapeTransformCommand(const QList<KoShape*> &shapes, const QList<QTransform> &oldState, const QList<QTransform> &newState, KUndo2Command * parent = 0);
    ~KoShapeTransformCommand() override;
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

#endif // KOSHAPETRANSFORMCOMMAND_H
