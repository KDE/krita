/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2006 Thomas Zander <zander@kde.org>
 * SPDX-FileCopyrightText: 2006 Jan Hambrecht <jaham@gmx.net>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KOSHAPEDISTRIBUTECOMMAND_H
#define KOSHAPEDISTRIBUTECOMMAND_H


#include "kritaflake_export.h"

#include <kundo2command.h>
#include <QList>

class KoShape;

/// The undo / redo command for distributing shapes
class KRITAFLAKE_EXPORT KoShapeDistributeCommand : public KUndo2Command
{
public:
    /// The different options to distribute with this command
    enum Distribute {
        HorizontalCenterDistribution,   ///< Horizontal centered
        HorizontalGapsDistribution,     ///< Horizontal Gaps
        HorizontalLeftDistribution,     ///< Horizontal Left
        HorizontalRightDistribution,    ///< Horizontal Right
        VerticalCenterDistribution,     ///< Vertical centered
        VerticalGapsDistribution,       ///< Vertical Gaps
        VerticalBottomDistribution,     ///< Vertical bottom
        VerticalTopDistribution         ///< Vertical top
    };
    /**
     * Command to align a set of shapes in a rect
     * @param shapes a set of all the shapes that should be distributed
     * @param distribute the distribution type
     * @param boundingRect the rect the shapes will be distributed in
     * @param parent the parent command used for macro commands
     */
    KoShapeDistributeCommand(const QList<KoShape*> &shapes, Distribute distribute,
            const QRectF &boundingRect, KUndo2Command *parent = 0);
    ~KoShapeDistributeCommand() override;
    /// redo the command
    void redo() override;
    /// revert the actions done in redo
    void undo() override;
private:

    class Private;
    Private * const d;
};

#endif
