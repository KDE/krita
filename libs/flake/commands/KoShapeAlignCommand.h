/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2006 Thomas Zander <zander@kde.org>
 * SPDX-FileCopyrightText: 2006 Jan Hambrecht <jaham@gmx.net>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KOSHAPEALIGNCOMMAND_H
#define KOSHAPEALIGNCOMMAND_H

#include "kritaflake_export.h"

#include <kundo2command.h>
#include <QList>

class KoShape;
class QRectF;

/// The undo / redo command for aligning shapes
class KRITAFLAKE_EXPORT KoShapeAlignCommand : public KUndo2Command
{
public:
    /// The different alignment options for this command
    enum Align {
        HorizontalLeftAlignment,    ///< Align left
        HorizontalCenterAlignment,  ///< Align Centered horizontally
        HorizontalRightAlignment,   ///< Align Right
        VerticalBottomAlignment,    ///< Align bottom
        VerticalCenterAlignment,    ///< Align centered vertically
        VerticalTopAlignment        ///< Align top
    };
    /**
     * Command to align a set of shapes in a rect
     * @param shapes a set of all the shapes that should be aligned
     * @param align the alignment type
     * @param boundingRect the rect the shape will be aligned in
     * @param parent the parent command used for macro commands
     */
    KoShapeAlignCommand(const QList<KoShape*> &shapes, Align align, const QRectF &boundingRect, KUndo2Command *parent = 0);
    ~KoShapeAlignCommand() override;
    /// redo the command
    void redo() override;
    /// revert the actions done in redo
    void undo() override;
private:
    class Private;
    Private * const d;
};

#endif
